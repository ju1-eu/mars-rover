/**
 * @file    main.cpp
 * @brief   Autonomer Modus: Geradeausfahrt mit reaktiver Hindernisvermeidung.
 * @author  Jan Unger
 * @version 1.1.0
 * @date    2025-11-25
 *
 * @details
 * Dieses Modul implementiert die High-Level-Logik für den autonomen Betrieb.
 * Es fungiert als Mittler zwischen der HAL (Hardware Abstraction Layer) und
 * der Entscheidungslogik.
 *
 * **Verhaltensmuster:**
 * 1. **Cruise:** PID-stabilisierte Geradeausfahrt.
 * 2. **Detect:** Ultraschall-Erkennung (< 15cm).
 * 3. **React:** Blockierende 180°-Wende (Dead Reckoning über Gyro).
 *
 * @platform ESP32S3 / GalaxyRVR
 * @dependency HAL::Sensor, HAL::Motor, Logic::DriveAssistant
 */

#include "Config.h"
#include "hal/Actuator.h"
#include "hal/Motor.h"
#include "hal/Sensor.h"
#include "logic/DriveAssistant.h"
#include <Arduino.h>

// --- EINSTELLUNGEN ---

/**
 * @brief   Sollgeschwindigkeit für die Geradeausfahrt.
 * @unit    PWM-Duty-Cycle (0–255)
 *
 * @note    Werte unter 40 reichen oft nicht, um die Haftreibung der Getriebe
 * zu überwinden (Deadband).
 */
constexpr uint8_t CRUISE_SPEED = 100;

/**
 * @brief   Drehgeschwindigkeit für Manöver auf der Stelle.
 * @unit    PWM-Duty-Cycle (0–255)
 *
 * @warning Zu hohe Werte (>120) führen auf glattem Boden zu massivem Schlupf,
 * wodurch die IMU-Winkelberechnung ungenau wird.
 */
constexpr uint8_t TURN_SPEED = 80;

/**
 * @brief   Sicherheitsabstand für die Notbremsung/Wende.
 * @unit    Zentimeter (cm)
 */
constexpr float STOP_DISTANCE_CM = 15.0f;

/**
 * @brief   Führt eine überwachte 180°-Wende an Ort und Stelle aus (Pivot Turn).
 *
 * @details
 * Diese Funktion blockiert den Haupt-Loop (`blocking call`), bis das Manöver
 * abgeschlossen ist. Sie nutzt eine zeit-integrierte Gierraten-Messung
 * (Dead Reckoning), um den Drehwinkel zu bestimmen.
 *
 * **Ablauf:**
 * 1. Stoppen der Motoren & Wartezeit (Stabilisierung IMU).
 * 2. Gegenläufiges Ansteuern der Ketten (Links - / Rechts +).
 * 3. Integration: \f$ \theta = \sum (\omega_{gyro} \cdot \Delta t) \f$
 * 4. Abbruch bei \f$ \theta \geq 180^\circ \f$ oder Kippgefahr.
 *
 * @hardware
 * - **Aktoren:** Motoren (PWM-Steuerung)
 * - **Sensoren:** IMU (Gyroskop Z-Achse für Gierrate, Accelerometer für Pitch)
 *
 * @safety
 * Die Schleife enthält einen **Not-Aus-Wächter**: Sollte der Rover während
 * der Drehung kippen (`Pitch > 60°`), werden die Motoren sofort abgeschaltet
 * und die Funktion verlassen.
 */
void performTurn180() {
    Serial.println(F("--- WENDE MANOEVER START (180 Grad) ---"));
    HAL::Motor::stop();
    delay(500);

    float currentAngle = 0.0f;
    unsigned long lastTime = millis();

    // Drehung einleiten (Linksherum: Links Rückwärts, Rechts Vorwärts)
    HAL::Motor::setSpeed(-TURN_SPEED, TURN_SPEED);

    // Schleife: Solange drehen, bis 180 Grad erreicht sind
    while (abs(currentAngle) < 180.0f) {
        // 1. Sensoren wach halten (Polling)
        HAL::Sensor::imuUpdate();

        // 2. Zeit-Delta berechnen
        unsigned long now = millis();
        float dt = (now - lastTime) / 1000.0f; // ms -> s
        lastTime = now;

        // 3. Winkel integrieren (Rate * Zeit = Winkeländerung)
        float rate = HAL::Sensor::getYawRate(); // Einheit: °/s
        currentAngle += rate * dt;

        // Debug-Ausgabe alle 100ms (Vermeidung von Serial-Spam)
        static unsigned long lastPrint = 0;
        if (now - lastPrint > 100) {
            lastPrint = now;
            Serial.print(F("Wende: "));
            Serial.print(abs(currentAngle), 0);
            Serial.println(F(" / 180 Grad"));
        }

        // @safety Not-Aus bei Kippen
        if (abs(HAL::Sensor::getPitch()) > 60.0f) {
            HAL::Motor::stop();
            Serial.println(F("ABBRUCH: Kippgefahr erkannt!"));
            return;
        }
    }

    HAL::Motor::stop();
    Serial.println(F("--- WENDE ABGESCHLOSSEN ---"));
    delay(1000); // Kurz orientieren vor Weiterfahrt

    // Assistenten resetten (Integrator-Windup löschen)
    Logic::DriveAssistant::init();
}

/**
 * @brief   Initialisierung der Rover-Peripherie.
 *
 * @details
 * Initialisiert I2C-Bus, Motortreiber und Sensoren. Führt zwingend eine
 * Gyro-Kalibrierung durch.
 *
 * @warning Während der Ausführung (ca. 3s) darf der Rover **nicht bewegt**
 * werden, da sonst der Gyro-Offset falsch berechnet wird (Drift!).
 */
void setup() {
    Serial.begin(115200);
    Serial.println(F("--- GALAXY RVR: AUTONOMER MODUS + WENDE ---"));

    HAL::Sensor::init();
    HAL::Motor::init();
    HAL::Actuator::init();
    Logic::DriveAssistant::init();

    Serial.println(F("IMU: Kalibrierung... (Stillhalten!)"));
    HAL::Actuator::setCameraAngle(90);
    HAL::Sensor::imuCalibrate(50); // 50 Samples für Mittelwert
    Serial.println(F("IMU: Bereit."));

    Serial.println(F("START IN 3 SEKUNDEN..."));
    delay(3000);
}

/**
 * @brief   Zentrale Steuerschleife (Main Loop).
 *
 * @details
 * Frequenz: Bestimmt durch Zykluszeit der Sensor-Updates und Loop-Overhead.
 *
 * **Zustandsautomat:**
 * - **Prüfung:** Ultraschall-Distanz messen.
 * - **Fall A (Hindernis):** Aufruf von `performTurn180()`.
 * - **Fall B (Frei):** Aufruf von `Logic::DriveAssistant::update()`.
 * - **Global Check:** Batterieüberwachung (5s Intervall) und Neigungsschutz.
 *
 * @safety
 * Ein "Dead-Loop" (`while(true)`) wird betreten, wenn der DriveAssistant
 * eine unsichere Fahrzeuglage meldet. Ein Hardware-Reset ist zum Neustart
 * nötig.
 */
void loop() {
    // Sensoren immer aktualisieren für frische Daten
    HAL::Sensor::imuUpdate();
    HAL::Sensor::ultrasonicUpdate();

    float distance = HAL::Sensor::getUltrasonicDistance();

    // --- HINDERNIS-LOGIK ---
    if (distance > 0.1f && distance < STOP_DISTANCE_CM) {
        // @todo Auslagern in eigene State-Machine Funktion?
        Serial.print(F("HINDERNIS BEI "));
        Serial.print(distance, 0);
        Serial.println(F(" cm -> START WENDE!"));

        performTurn180();
    } else {
        // --- FREIE FAHRT ---
        bool safe = Logic::DriveAssistant::update(CRUISE_SPEED);

        if (!safe) {
            HAL::Motor::stop();
            Serial.println(F("NOT-AUS: Rover gekippt!"));
            // Sicherer Zustand: Endlosschleife
            while (true) {
                delay(100);
            }
        }
    }

    // Batterie-Check alle 5s (Non-blocking)
    static unsigned long lastBat = 0;
    if (millis() - lastBat > 5000) {
        lastBat = millis();
        Serial.print(F("[Bat] "));
        Serial.print(HAL::Sensor::getBatteryVoltage());
        Serial.println(F(" V"));
    }
}
