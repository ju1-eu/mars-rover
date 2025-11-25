/**
 * @file    main.cpp
 * @brief   Einstiegspunkt für Phase 1: Präzisions-Fahrt und Hindernis-Wende.
 * @author  Jan Unger
 * @version 1.1.1 (PID-Integration)
 * @date    2025-11-25
 *
 * @details
 * Dieses Hauptprogramm demonstriert die Fähigkeiten der Schichtarchitektur
 * aus Phase 1. Es verbindet:
 * 1. HAL (Hardware Abstraction Layer): Zugriff auf Motoren und Sensoren.
 * 2. Logic (DriveAssistant): PID-geregelte Geradeausfahrt.
 * 3. App (Main Loop): Einfache Entscheidungslogik (Fahren vs. Wenden).
 *
 * Ablauf:
 * - Der Rover fährt PID-stabilisiert geradeaus ("Cruise").
 * - Bei Hindernis (<15cm) führt er eine gyroskop-gesteuerte 180°-Wende aus.
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
 * @brief   Sollgeschwindigkeit für die Geradeausfahrt.
 * @unit    PWM-Duty-Cycle (0–255)
 * @note    Wert sollte > 60 sein, damit PID-Regelung genug "Headroom" hat,
 * um einen Motor zu verlangsamen, ohne ihn ganz zu stoppen.
 */
constexpr uint8_t CRUISE_SPEED = 100;

/**
 * @brief   Drehgeschwindigkeit für Pivot-Turn (Wenden auf der Stelle).
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
 * Integriert die Gierrate (Gyro-Z) über die Zeit, um den Drehwinkel zu messen.
 * Dies ist genauer als reine Zeitsteuerung (delay), da es Batteriespannung
 * und Bodenbeschaffenheit kompensiert.
 */
void performTurn180() {
    Serial.println(F(">>> MANÖVER: 180° Wende start <<<"));

    // 1. Stoppen für sauberen Ausgangszustand
    HAL::Motor::stop();
    delay(500); // Kurz warten, damit Chassi-Schwingungen abklingen

    float currentAngle = 0.0f;
    unsigned long lastTime = millis();

    // 2. Drehung einleiten (Gegenläufig)
    HAL::Motor::setSpeed(-TURN_SPEED, TURN_SPEED);

    // 3. Regelschleife für den Turn
    while (abs(currentAngle) < 180.0f) {
        // A. Sensoren aktualisieren
        HAL::Sensor::imuUpdate();

        // B. Zeitdifferenz messen (dt)
        unsigned long now = millis();
        float dt = (now - lastTime) / 1000.0f; // in Sekunden
        lastTime = now;

        // C. Integration: Winkel += Rate * Zeit
        // Rate ist in Grad/Sekunde.
        float rate = HAL::Sensor::getYawRate();
        currentAngle += rate * dt;

        // D. Sicherheit: Kippschutz
        if (abs(HAL::Sensor::getPitch()) > 60.0f) {
            HAL::Motor::stop();
            Serial.println(F("!!! ABBRUCH: Kippgefahr während Wende !!!"));
            return;
        }

        // E. Optional: Debug (nur alle 100ms)
        // static unsigned long lastPrint ... (der Übersicht halber weggelassen)
    }

    // 4. Abschluss
    HAL::Motor::stop();
    Serial.println(F(">>> MANÖVER: Wende beendet <<<"));
    delay(1000); // Kurz orientieren

    // WICHTIG: PID-Regler resetten!
    // Durch die Drehung hat sich der "Fehler" im Kopf des Reglers massiv
    // geändert oder das Integral könnte weggelaufen sein (falls aktiv).
    // Ein Reset sorgt dafür, dass er wieder bei "0" anfängt zu denken.
    Logic::DriveAssistant::init();
}

// ==========================================================================
// ARDUINO LIFECYCLE
// ==========================================================================

void setup() {
    Serial.begin(115200);
    // Warte kurz auf Serielle Verbindung (nur bei nativem USB nötig, schadet
    // aber nicht)
    delay(1000);

    Serial.println(F("=== GALAXY RVR: PHASE 1 START ==="));

    // 1. Hardware initialisieren
    HAL::Sensor::init();
    HAL::Motor::init();
    HAL::Actuator::init();

    // 2. Logik initialisieren
    Logic::DriveAssistant::init();

    // 3. Kalibrierung
    Serial.println(F("IMU: Kalibriere Gyro... BITTE NICHT BEWEGEN!"));
    HAL::Actuator::setCameraAngle(90); // Kamera neutral

    // Wir nehmen 100 Samples für höhere Genauigkeit beim Start
    HAL::Sensor::imuCalibrate(100);
    Serial.println(F("IMU: Kalibrierung abgeschlossen."));

    Serial.println(F("--> GO in 3 Sekunden..."));
    delay(3000);
}

void loop() {
    // 1. Globale Sensor-Updates (Wichtig: Einmal pro Loop!)
    HAL::Sensor::imuUpdate();
    HAL::Sensor::ultrasonicUpdate();

    // 2. Umgebungs-Check
    float distance = HAL::Sensor::getUltrasonicDistance();

    // Gültige Distanzmessung (>0) und Hindernis nah genug?
    if (distance > 0.1f && distance < STOP_DISTANCE_CM) {

        // --- ZUSTAND: REAGIEREN ---
        Serial.print(F("Hindernis erkannt: "));
        Serial.print(distance);
        Serial.println(F(" cm"));

        performTurn180();

    } else {

        // --- ZUSTAND: CRUISE (PID GEREGELT) ---
        // update() liefert false, wenn der Rover kippt (Safety)
        bool safe = Logic::DriveAssistant::update(CRUISE_SPEED);

        if (!safe) {
            HAL::Motor::stop();
            Serial.println(F("!!! NOT-AUS: Kritische Neigung !!!"));

            // Fange den Rover in einer Endlosschleife, bis Reset
            while (true) {
                HAL::Actuator::setCameraAngle(45); // Signal: Ich habe Angst
                delay(200);
                HAL::Actuator::setCameraAngle(135);
                delay(200);
            }
        }
    }

    // Optional: Zyklischer Batterie-Check (alle 5s)
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

    // Kleines Delay für Stabilität des Loops (optional, da Sensor-Updates Zeit
    // brauchen)
    delay(10);
}
