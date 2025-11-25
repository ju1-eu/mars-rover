/**
 * @file    main.cpp
 * @brief   Einstiegspunkt für Phase 1: Präzisions-Fahrt und Hindernis-Wende.
 * @author  Jan Unger
 * @version 1.2.0 (Final Phase 1: PID + Inertia-Kompensation)
 * @date    2025-11-25
 *
 * @details
 * Dieses Hauptprogramm demonstriert die Fähigkeiten der Schichtarchitektur.
 * Es realisiert eine autonome "Cruise"-Funktion mit Hindernisvermeidung.
 *
 * **Architektur-Einordnung (Didaktik):**
 * - **App Layer (Hier):** Die "Exekutive". Sie trifft High-Level Entscheidungen
 * (z.B. "Hindernis da -> Wende einleiten").
 * - **Logic Layer (DriveAssistant):** Das "Kleinhirn". Es kümmert sich um die
 * motorische Umsetzung und Stabilisierung (PID-Regelung).
 * - **HAL (Sensor/Motor):** Die "Nervenbahnen". Zugriff auf Hardware.
 *
 * **Ablaufdiagramm:**
 * 1. Sensoren lesen (IMU, Ultraschall).
 * 2. Ist Hindernis < 15cm?
 * - JA: Führe 180° Wende aus (Open Loop Sequenz).
 * - NEIN: Fahre geradeaus (Closed Loop PID-Regelung).
 *
 * @platform ESP32S3 / GalaxyRVR
 */

#include "Config.h"
#include "hal/Actuator.h"
#include "hal/Motor.h"
#include "hal/Sensor.h"
#include "logic/DriveAssistant.h"
#include <Arduino.h>

// ==========================================================================
// KONFIGURATION
// ==========================================================================

/**
 * @brief   Sollgeschwindigkeit für die Geradeausfahrt (Cruise).
 * @unit    PWM-Duty-Cycle (0–255)
 * * @note    **Warum nicht 255?**
 * Der PID-Regler braucht "Headroom" (Regelreserve). Wenn ein Motor schon
 * auf 100% (255) läuft, kann er nicht mehr beschleunigt werden, um eine
 * Drehung auszugleichen. Er könnte nur bremsen. Ein Wert um 100 lässt
 * Spielraum in beide Richtungen.
 */
constexpr uint8_t CRUISE_SPEED = 100;

/**
 * @brief   Drehgeschwindigkeit für Pivot-Turn (Wenden auf der Stelle).
 * @safety  Nicht zu schnell wählen, um Überschwingen zu minimieren.
 */
constexpr uint8_t TURN_SPEED = 80;

/**
 * @brief   Auslöse-Distanz für Hindernisse.
 * @unit    Zentimeter (cm)
 */
constexpr float STOP_DISTANCE_CM = 15.0f;

// ==========================================================================
// HILFSFUNKTIONEN
// ==========================================================================

/**
 * @brief   Führt eine blockierende 180°-Wende aus (Dead Reckoning).
 *
 * @details
 * Diese Funktion nutzt das Gyroskop, um eine definierte Drehung auszuführen.
 *
 * **Mathematisches Prinzip (Integration):**
 * Der Winkel $\theta$ wird durch Aufsummieren der Drehgeschwindigkeit $\omega$
 * über die Zeit $t$ berechnet:
 * $$ \theta = \int \omega \, dt \approx \sum (\text{yawRate} \cdot \Delta t) $$
 *
 * **Physikalische Korrektur (Inertia):**
 * Motoren stoppen nicht instantan. Das Chassis hat Masse und Schwung.
 * Daher schalten wir die Motoren bereits bei **177.9°** ab. Der
 * "Nachlauf" (Coasting) dreht den Rover die restlichen ~2.1 Grad.
 *
 * @hardware Gyroskop (Z-Achse)
 */
void performTurn180() {
    Serial.println(F(">>> MANÖVER: 180° Wende start <<<"));

    // 1. Stoppen für sauberen Ausgangszustand
    // Wichtig, damit das Integral nicht durch Restbewegung verfälscht wird.
    HAL::Motor::stop();
    delay(500);

    float currentAngle = 0.0f;
    unsigned long lastTime = millis();

    // 2. Drehung einleiten (Gegenläufige Räder -> Drehen auf der Stelle)
    HAL::Motor::setSpeed(-TURN_SPEED, TURN_SPEED);

    // 3. Regelschleife für den Turn
    // Abbruchbedingung inkludiert Trägheits-Kompensation (177.9 statt 180.0)
    while (abs(currentAngle) < 177.9f) {

        // A. Sensoren aktualisieren (Polling)
        HAL::Sensor::imuUpdate();

        // B. Zeitdifferenz messen (dt) für Integration
        unsigned long now = millis();
        float dt = (now - lastTime) / 1000.0f; // Konvertierung ms -> Sekunden
        lastTime = now;

        // C. Numerische Integration: Winkel += Rate * Zeit
        float rate = HAL::Sensor::getYawRate();
        currentAngle += rate * dt;

        // D. Sicherheit: Kippschutz (Safety Guard)
        // Sollte der Rover während der Drehung auf eine Kante fahren und
        // kippen:
        if (abs(HAL::Sensor::getPitch()) > 60.0f) {
            HAL::Motor::stop();
            Serial.println(F("!!! ABBRUCH: Kippgefahr während Wende !!!"));
            return;
        }
    }

    // 4. Abschluss
    HAL::Motor::stop(); // Strom weg -> Rover rollt aus (Nachlauf)
    Serial.println(F(">>> MANÖVER: Wende beendet <<<"));

    // Kurze Orientierungspause für das System
    delay(1000);

    // WICHTIG: PID-Regler resetten!
    // Der Regler in DriveAssistant "denkt" noch, er müsste geradeaus fahren.
    // Die 180°-Drehung würde er als massiven Fehler interpretieren.
    // Ein Reset löscht das Gedächtnis des Reglers.
    Logic::DriveAssistant::init();
}

// ==========================================================================
// ARDUINO LIFECYCLE
// ==========================================================================

void setup() {
    Serial.begin(115200);
    delay(1000); // Warten auf USB-Stack (bei ESP32-S3 wichtig)

    Serial.println(F("=== GALAXY RVR: PHASE 1 START ==="));

    // 1. Hardware initialisieren (HAL Layer)
    HAL::Sensor::init();
    HAL::Motor::init();
    HAL::Actuator::init();

    // 2. Logik initialisieren (Logic Layer)
    Logic::DriveAssistant::init();

    // 3. Kalibrierung (WICHTIG!)
    // Das Gyroskop misst "0" nur relativ. Der Offset muss im Stillstand
    // ermittelt werden.
    Serial.println(F("IMU: Kalibriere Gyro... BITTE NICHT BEWEGEN!"));
    HAL::Actuator::setCameraAngle(90); // Kamera neutral ausrichten

    // 100 Samples reichen für eine grobe Kalibrierung beim Start
    HAL::Sensor::imuCalibrate(100);
    Serial.println(F("IMU: Kalibrierung abgeschlossen."));

    Serial.println(F("--> GO in 3 Sekunden..."));
    delay(3000);
}

void loop() {
    // 1. Globale Sensor-Updates
    // Muss zwingend einmal pro Loop-Zyklus passieren, damit Logic-Klassen
    // auf aktuelle Daten zugreifen können.
    HAL::Sensor::imuUpdate();
    HAL::Sensor::ultrasonicUpdate();

    // 2. Umgebungs-Check (Perzeption)
    float distance = HAL::Sensor::getUltrasonicDistance();

    // Gültige Distanzmessung (>0) und Hindernis innerhalb der Warnzone?
    if (distance > 0.1f && distance < STOP_DISTANCE_CM) {

        // --- ZUSTAND: HINDERNISVERMEIDUNG ---
        Serial.print(F("Hindernis erkannt: "));
        Serial.print(distance);
        Serial.println(F(" cm"));

        performTurn180();

    } else {

        // --- ZUSTAND: CRUISE (PID GEREGELT) ---
        // update() liefert false, wenn der Rover kippt (Not-Halt-Bedingung)
        bool safe = Logic::DriveAssistant::update(CRUISE_SPEED);

        if (!safe) {
            HAL::Motor::stop();
            Serial.println(F("!!! NOT-AUS: Kritische Neigung !!!"));

            // Fehlerzustand: Endlosschleife mit visuellem Signal
            // (Kamera-Wackeln)
            while (true) {
                HAL::Actuator::setCameraAngle(45); // Signal: "Ich habe Angst"
                delay(200);
                HAL::Actuator::setCameraAngle(135);
                delay(200);
            }
        }
    }

    // Optional: Zyklischer Batterie-Check (Non-blocking, alle 5s)
    static unsigned long lastBatCheck = 0;
    if (millis() - lastBatCheck > 5000) {
        lastBatCheck = millis();
        float voltage = HAL::Sensor::getBatteryVoltage();

        // Warnung bei niedriger Batterie (< 7.0V bei 2S LiPo/LiIon)
        if (voltage < 7.0f) {
            Serial.print(F("WARNUNG: Batterie niedrig: "));
        } else {
            Serial.print(F("Info: Batterie: "));
        }
        Serial.print(voltage);
        Serial.println(F(" V"));
    }

    // Kurzes Delay entlastet die CPU und gibt Sensoren Zeit zum Atmen
    // (IMU Update-Rate beachten!)
    delay(10);
}
