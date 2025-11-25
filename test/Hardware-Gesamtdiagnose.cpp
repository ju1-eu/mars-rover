/**
 * @file    main.cpp
 * @brief   Anwendungsschicht: Hardware-Gesamtdiagnose des Galaxy RVR.
 * @author  Jan Unger
 * @version 1.0.0
 * @date    2025-11-25
 *
 * @details
 * Dieses Modul führt einen sequentiellen Hardware-Selbsttest ("Power-On
 * Self-Test" ähnlich) durch. Es dient zur Validierung der Bestückung und
 * Verkabelung nach dem Zusammenbau.
 *
 * **Test-Sequenz:**
 * 1. **Motor:** Prüfung der H-Brücken & Getriebe (Vor/Rück/Drehen).
 * 2. **Servo:** Sweep-Test der Kamera/Sensor-Halterung.
 * 3. **RGB:** Farbzyklus (R-G-B) der Status-LEDs.
 * 4. **Sonar:** Distanzmessung (Plausibilitätscheck).
 * 5. **IR:** Prüfung der Linienfolger-Sensoren.
 * 6. **IMU:** Interaktiver Neigungstest durch den Anwender.
 *
 * @platform ESP32S3 / GalaxyRVR
 * @dependency HAL::Sensor, HAL::Actuator, Logic::Motion
 */

#include "Config.h"
#include "hal/Actuator.h"
#include "hal/Sensor.h"
#include "logic/Motion.h"
#include <Arduino.h>

/**
 * @enum    HardwareTestPhase
 * @brief   Zustandsautomat für den sequentiellen Testablauf.
 *
 * Steuert die Reihenfolge der Tests. Die Umschaltung erfolgt primär
 * zeitgesteuert
 * (`PHASE_DURATION_MS`).
 */
enum class HardwareTestPhase {
    INIT,            ///< Initialisierung & Kalibrierung (Ruhephase).
    MOTOR_TEST,      ///< H-Brücken Test: Vorwärts, Rückwärts, Punktwende.
    SERVO_TEST,      ///< PWM-Test: Pan/Tilt Servos.
    RGB_TEST,        ///< GPIO-Test: WS2812B Ansteuerung.
    ULTRASONIC_TEST, ///< I2C/IO-Test: Laufzeitmessung HC-SR04/P.
    IR_TEST,         ///< ADC/IO-Test: IR-Reflexionssensoren.
    IMU_TEST,        ///< I2C-Test: Gyro & Accel Datenfluss.
    FINISHED         ///< Ende der Diagnose, System halt.
};

/// Aktuelle Phase der State Machine.
HardwareTestPhase currentPhase = HardwareTestPhase::INIT;

/// Zeitstempel für den Eintritt in die aktuelle Phase.
unsigned long phaseStartTime = 0;

/**
 * @brief   Dauer einer einzelnen Testphase.
 * @unit    Millisekunden (ms)
 */
constexpr unsigned long PHASE_DURATION_MS = 5000UL;

/**
 * @brief   Intervall für das Telemetrie-Logging (Batterie).
 * @unit    Millisekunden (ms)
 */
constexpr unsigned long BATTERY_LOG_INTERVAL_MS = 10000UL;

// ----------------------------------------------------------------------------
// Motor-Test Logik
// ----------------------------------------------------------------------------

/**
 * @brief   Führt den Motor-Test über den Motion-Layer aus.
 *
 * @details
 * Durchläuft zyklisch die Grundbewegungen.
 * Wir nutzen hier bewusst `Logic::Motion` statt `HAL::Motor`, um auch die
 * korrekte Funktion der Abstraktionsschicht (Mapping von % auf PWM) zu prüfen.
 *
 * **Zyklus:**
 * - 0: Vorwärts
 * - 1: Rückwärts
 * - 2: Links (Punktwende)
 * - 3: Rechts (Punktwende)
 * - 4: Stop
 *
 * @param   now Aktuelle Systemzeit in ms (zur Vermeidung erneuter `millis()`
 * Calls).
 */
static void motorTestLogicViaMotion(unsigned long now) {
    static unsigned long lastStepChange = 0;
    static uint8_t step = 0;
    static int8_t lastStepPrinted = -1;

    constexpr unsigned long STEP_INTERVAL_MS = 1000UL;
    constexpr uint8_t MAX_STEP = 4;

    // Zeitsteuerung der Sub-Steps
    if (lastStepChange == 0) {
        lastStepChange = now;
        step = 0;
    } else if (now - lastStepChange >= STEP_INTERVAL_MS && step < MAX_STEP) {
        lastStepChange = now;
        ++step;
    }

    // Geschwindigkeit: 20% von Vmax (sicherer Testmodus auf Tisch)
    const uint8_t TEST_SPEED = static_cast<uint8_t>(Config::SpeedMax * 0.20f);

    // Logging nur bei Zustandswechsel (vermeidet Serial-Spam)
    if (step != lastStepPrinted) {
        switch (step) {
        case 0:
            Serial.println(F("Motor: Vorwaerts"));
            break;
        case 1:
            Serial.println(F("Motor: Rueckwaerts"));
            break;
        case 2:
            Serial.println(F("Motor: Links"));
            break;
        case 3:
            Serial.println(F("Motor: Rechts"));
            break;
        default:
            Serial.println(F("Motor: Stop"));
            break;
        }
        lastStepPrinted = static_cast<int8_t>(step);
    }

    // Aktorik
    switch (step) {
    case 0:
        Logic::Motion::moveForward(TEST_SPEED);
        break;
    case 1:
        Logic::Motion::moveBackward(TEST_SPEED);
        break;
    case 2:
        Logic::Motion::turnLeft(TEST_SPEED);
        break;
    case 3:
        Logic::Motion::turnRight(TEST_SPEED);
        break;
    default:
        Logic::Motion::stopMove();
        break;
    }
}

/**
 * @brief   System-Setup und Kalibrierung.
 *
 * @details
 * Initialisiert alle Subsysteme. Führt eine blockierende IMU-Kalibrierung
 * durch.
 *
 * @pre     **WICHTIG:** Der Rover muss beim Einschalten auf einer ebenen Fläche
 * stehen und darf während der Kalibrierung (ca. 3s) **nicht bewegt** werden.
 * Erschütterungen führen zu falschem Gyro-Offset (Drift).
 */
void setup() {
    Serial.begin(115200);
    // Warten auf USB-Serial Verbindung (optional bei ESP32 native USB)
    delay(1000);

    Serial.println();
    Serial.println(F("--- GALAXY RVR HARDWARE DIAGNOSE START ---"));

    HAL::Sensor::init();
    Logic::Motion::init();
    HAL::Actuator::init();

    // @hardware IMU-Kalibrierung (MPU6050 / QMC6310)
    Serial.println(
        F("IMU: Kalibrierung (bitte warten, Rover nicht bewegen)..."));
    HAL::Sensor::imuCalibrate(50); // 50 Samples für Mittelwertbildung
    Serial.println(F("IMU: Kalibrierung fertig."));

    Serial.println(F("SET+START"));
    phaseStartTime = millis();
}

/**
 * @brief   Main-Loop / Test-Scheduler.
 *
 * @details
 * Verwaltet den Zustandsautomaten (`HardwareTestPhase`).
 *
 * @safety
 * Unabhängig vom Testzustand wird `HAL::Sensor::imuUpdate()` **in jedem
 * Zyklus** aufgerufen. Dies ist zwingend erforderlich, damit der AHRS-Filter
 * (Mahony/Madgwick) konvergiert und keine Lags entstehen.
 */
void loop() {
    unsigned long now = millis();

    // ========================================================================
    // @safety CRITICAL: IMU-Update muss IMMER laufen (Filter-Konvergenz)
    // ========================================================================
    HAL::Sensor::imuUpdate();

    // --- Hintergrund: Batterie-Logging ---
    static unsigned long lastBatteryLog = 0;
    if (now - lastBatteryLog > BATTERY_LOG_INTERVAL_MS) {
        lastBatteryLog = now;
        Serial.print(F("[Battery] U="));
        Serial.print(HAL::Sensor::getBatteryVoltage());
        Serial.print(F("V, SOC="));
        Serial.print(HAL::Sensor::getBatteryPercentage());
        Serial.println(F("%"));
    }

    // --- Zustandsautomat ---
    switch (currentPhase) {
    case HardwareTestPhase::INIT:
        Serial.println(F("Starte Motor-Test..."));
        phaseStartTime = now;
        currentPhase = HardwareTestPhase::MOTOR_TEST;
        break;

    case HardwareTestPhase::MOTOR_TEST:
        motorTestLogicViaMotion(now);
        if (now - phaseStartTime > PHASE_DURATION_MS) {
            Logic::Motion::stopMove(); // Sicherstellen, dass Motoren aus sind
            Serial.println(F("Starte Servo-Test..."));
            phaseStartTime = now;
            currentPhase = HardwareTestPhase::SERVO_TEST;
        }
        break;

    case HardwareTestPhase::SERVO_TEST:
        // @hardware Testet Pan/Tilt Servos
        HAL::Actuator::servoTest();
        if (now - phaseStartTime > PHASE_DURATION_MS) {
            Serial.println(F("Starte RGB-Test..."));
            phaseStartTime = now;
            currentPhase = HardwareTestPhase::RGB_TEST;
        }
        break;

    case HardwareTestPhase::RGB_TEST:
        // @hardware Testet WS2812 LEDs
        HAL::Actuator::rgbTest();
        if (now - phaseStartTime > PHASE_DURATION_MS) {
            Serial.println(F("Starte Ultraschall-Test..."));
            phaseStartTime = now;
            currentPhase = HardwareTestPhase::ULTRASONIC_TEST;
        }
        break;

    case HardwareTestPhase::ULTRASONIC_TEST:
        HAL::Sensor::ultrasonicTest();
        if (now - phaseStartTime > PHASE_DURATION_MS) {
            Serial.println(F("Starte IR-Test..."));
            phaseStartTime = now;
            currentPhase = HardwareTestPhase::IR_TEST;
        }
        break;

    case HardwareTestPhase::IR_TEST:
        HAL::Sensor::irTest();
        if (now - phaseStartTime > PHASE_DURATION_MS) {
            Serial.println(F("Starte IMU-Diagnose (Kippen Sie den Rover)..."));
            phaseStartTime = now;
            currentPhase = HardwareTestPhase::IMU_TEST;
        }
        break;

    case HardwareTestPhase::IMU_TEST: {
        static unsigned long lastPrint = 0;

        // Anzeige nur alle 200ms aktualisieren (Lesbarkeit)
        if (now - lastPrint > 200) {
            lastPrint = now;

            float p = HAL::Sensor::getPitch();   // Neigung (Nase hoch/runter)
            float r = HAL::Sensor::getRoll();    // Wanken (Seitlich kippen)
            float y = HAL::Sensor::getYawRate(); // Drehung (Gieren)

            // "Deadzone": Werte unter 2.0° als 0 betrachten
            // (Rauschunterdrückung)
            // \f$ |val| < 2.0 \f$
            bool isFlat = abs(p) < 2.0f;
            bool isLevel = abs(r) < 2.0f;
            bool isStill = abs(y) < 2.0f;

            // --- 1. PITCH (Nase) ---
            Serial.print(F("Lage: "));
            if (isFlat)
                Serial.print(F("FLACH   "));
            else if (p > 0)
                Serial.print(F("BERGAUF "));
            else
                Serial.print(F("BERGAB  "));

            Serial.print(F("("));
            Serial.print(p, 1);
            Serial.print(F("deg)"));

            // --- 2. ROLL (Seite) ---
            Serial.print(F(" | Kippt: "));
            if (isLevel)
                Serial.print(F("GERADE "));
            else if (r > 0)
                Serial.print(F("LINKS  "));
            else
                Serial.print(F("RECHTS "));

            Serial.print(F("("));
            Serial.print(r, 1);
            Serial.print(F("deg)"));

            // --- 3. YAW (Lenkung) ---
            Serial.print(F(" | Gier: "));
            if (isStill)
                Serial.print(F("STOP   "));
            else if (y > 0)
                Serial.print(F("LINKS  "));
            else
                Serial.print(F("RECHTS "));

            Serial.print(F("("));
            Serial.print(y, 1);
            Serial.println(F("deg/s)"));

            // --- SAFETY MONITOR ---
            // Visualisierung für den späteren Not-Aus
            // Grenzwert: \f$ > 45^\circ \f$
            if (abs(p) > 45.0f || abs(r) > 45.0f) {
                Serial.println(
                    F(">>> ALARM: KIPP-GRENZE UEBERSCHRITTEN (NOT-AUS) <<<"));
            }
        }

        if (now - phaseStartTime > PHASE_DURATION_MS) {
            Serial.println(F("Test abgeschlossen."));
            phaseStartTime = now;
            currentPhase = HardwareTestPhase::FINISHED;
        }
        break;
    }

    case HardwareTestPhase::FINISHED: {
        static bool finishedLogged = false;
        if (!finishedLogged) {
            Logic::Motion::stopMove();
            Serial.println(F("--- HARDWARE DIAGNOSE ENDE ---"));
            finishedLogged = true;
        }
        // System bleibt hier im Idle
        break;
    }
    }
}
