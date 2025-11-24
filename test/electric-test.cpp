/**
 * Anwendungsschicht: Hardware-Diagnose (alle Komponenten)
 * Anpassung: Integriert IMU-Update für Filter und nutzt Getter-Methoden.
 */

#include "Config.h"
#include "hal/Actuator.h"
#include "hal/Sensor.h"
#include "logic/Motion.h"
#include <Arduino.h>

// Einfaches State-Enum für den Gesamttest-Ablauf
enum class HardwareTestPhase {
    INIT,
    MOTOR_TEST,
    SERVO_TEST,
    RGB_TEST,
    ULTRASONIC_TEST,
    IR_TEST,
    IMU_TEST,
    FINISHED
};

HardwareTestPhase currentPhase = HardwareTestPhase::INIT;
unsigned long phaseStartTime = 0;

constexpr unsigned long PHASE_DURATION_MS = 5000UL;
constexpr unsigned long BATTERY_LOG_INTERVAL_MS = 10000UL;

// ----------------------------------------------------------------------------
// Motor-Test Logik (Unverändert)
// ----------------------------------------------------------------------------
static void motorTestLogicViaMotion(unsigned long now) {
    static unsigned long lastStepChange = 0;
    static uint8_t step = 0;
    static int8_t lastStepPrinted = -1;

    constexpr unsigned long STEP_INTERVAL_MS = 1000UL;
    constexpr uint8_t MAX_STEP = 4;

    if (lastStepChange == 0) {
        lastStepChange = now;
        step = 0;
    } else if (now - lastStepChange >= STEP_INTERVAL_MS && step < MAX_STEP) {
        lastStepChange = now;
        ++step;
    }

    const uint8_t TEST_SPEED = static_cast<uint8_t>(Config::SpeedMax * 0.20f);

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

void setup() {
    Serial.begin(115200);
    Serial.println();
    Serial.println(F("--- GALAXY RVR HARDWARE DIAGNOSE START ---"));

    HAL::Sensor::init();
    Logic::Motion::init();
    HAL::Actuator::init();

    // IMU-Kalibrierung (WICHTIG: Rover muss still stehen!)
    Serial.println(
        F("IMU: Kalibrierung (bitte warten, Rover nicht bewegen)..."));
    HAL::Sensor::imuCalibrate(50);
    Serial.println(F("IMU: Kalibrierung fertig."));

    Serial.println(F("SET+START"));

    phaseStartTime = millis();
}

void loop() {
    unsigned long now = millis();

    // ========================================================================
    // WICHTIG: IMU-Update muss IMMER laufen (für den Filter-Algorithmus)
    // ========================================================================
    HAL::Sensor::imuUpdate();

    // --- Periodisches Battery-Logging ---
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
            Logic::Motion::stopMove();
            Serial.println(F("Starte Servo-Test..."));
            phaseStartTime = now;
            currentPhase = HardwareTestPhase::SERVO_TEST;
        }
        break;

    case HardwareTestPhase::SERVO_TEST:
        HAL::Actuator::servoTest();
        if (now - phaseStartTime > PHASE_DURATION_MS) {
            Serial.println(F("Starte RGB-Test..."));
            phaseStartTime = now;
            currentPhase = HardwareTestPhase::RGB_TEST;
        }
        break;

    case HardwareTestPhase::RGB_TEST:
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

        // Anzeige nur alle 200ms aktualisieren (lesbarer)
        if (now - lastPrint > 200) {
            lastPrint = now;

            float p = HAL::Sensor::getPitch();   // Neigung (Nase hoch/runter)
            float r = HAL::Sensor::getRoll();    // Wanken (Seitlich kippen)
            float y = HAL::Sensor::getYawRate(); // Drehung (Gieren)

            // "Totzone": Werte unter 2.0 werden als 0 betrachtet (Ruhige
            // Anzeige)
            bool isFlat = abs(p) < 2.0f;
            bool isLevel = abs(r) < 2.0f;
            bool isStill = abs(y) < 2.0f;

            // --- 1. PITCH (Nase) ---
            Serial.print(F("Lage: "));
            if (isFlat)
                Serial.print(F("FLACH   "));
            else if (p > 0)
                Serial.print(F("BERGAUF ")); // Nase hoch
            else
                Serial.print(F("BERGAB  ")); // Nase runter

            Serial.print(F("("));
            Serial.print(p, 1);
            Serial.print(F("°)"));

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
            Serial.print(F("°)"));

            // --- 3. YAW (Lenkung) ---
            Serial.print(F(" | Gier: "));
            if (isStill)
                Serial.print(F("STOP   "));
            else if (y > 0)
                Serial.print(F("LINKS  ")); // Positiv = Linksdrehung
            else
                Serial.print(F("RECHTS "));

            Serial.print(F("("));
            Serial.print(y, 1);
            Serial.println(F("°/s)"));

            // --- SICHERHEITS-ALARM (Grenzwert-Überwachung) ---
            // Visualisierung für den späteren Not-Aus
            if (abs(p) > 45.0f || abs(r) > 45.0f) {
                Serial.println(
                    F(">>> ALARM: KIPP-GRENZE UEBERSCHRITTEN (NOT-AUS) <<<"));
            }
        }

        // Testdauer prüfen (5 Sekunden)
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
        break;
    }
    }
}
