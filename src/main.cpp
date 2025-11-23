/**
 * Anwendungsschicht: Hardware-Diagnose (alle Komponenten)
 * Ziel: Ruft alle HAL-Testfunktionen (Motor, Servo, RGB, Ultraschall, IR)
 *       sequentiell und nicht-blockierend auf und loggt periodisch den
 *       Batteriestatus.
 */

#include "Config.h" // ggf. von Sensor.h/Motor.h genutzte Konfiguration
#include "hal/Motor.h"
#include "hal/Sensor.h"
#include <Arduino.h>

// Einfaches State-Enum für den Gesamttest-Ablauf
enum class HardwareTestPhase {
    INIT,
    MOTOR_TEST,
    SERVO_TEST,
    RGB_TEST,
    ULTRASONIC_TEST,
    IR_TEST,
    FINISHED
};

HardwareTestPhase currentPhase = HardwareTestPhase::INIT;
unsigned long phaseStartTime = 0;

// Dauer jeder Testphase in Millisekunden
constexpr unsigned long PHASE_DURATION_MS = 5000UL;
// Intervall für Batterie-Log in Millisekunden
constexpr unsigned long BATTERY_LOG_INTERVAL_MS = 10000UL; // alle 10 s

/**
 * @brief Initialisierung von Serial und allen HAL-Komponenten.
 */
void setup() {
    Serial.begin(115200);
    Serial.println();
    Serial.println(F("--- GALAXY RVR HARDWARE DIAGNOSE START ---"));

    // Init aller HAL-Komponenten
    HAL::Motor::init();
    HAL::Sensor::init();

    // Kameratest-Befehl (hier als einfacher Textbefehl)
    Serial.println(F("SET+START"));

    // ersten Batteriestatus loggen
    float u0 = HAL::Sensor::getBatteryVoltage();
    uint8_t s0 = HAL::Sensor::getBatteryPercentage();
    Serial.print(F("[Battery] Start: U = "));
    Serial.print(u0);
    Serial.print(F(" V, SOC = "));
    Serial.print(s0);
    Serial.println(F(" %"));

    // Startzeit setzen, Phase bleibt zunächst auf INIT
    phaseStartTime = millis();
}

/**
 * @brief Zustandsautomat: führt die einzelnen Hardware-Tests nacheinander aus.
 *
 * Jede Phase ruft ihre HAL-Testfunktion nicht-blockierend auf.
 * Nach Ablauf von PHASE_DURATION_MS wird in die nächste Phase gewechselt.
 * Zusätzlich wird in festen Intervallen der Batteriestatus geloggt.
 */
void loop() {
    unsigned long now = millis();

    // --- Periodisches Battery-Logging (nicht-blockierend) ---
    static unsigned long lastBatteryLog = 0;
    if (now - lastBatteryLog > BATTERY_LOG_INTERVAL_MS) {
        lastBatteryLog = now;
        float u = HAL::Sensor::getBatteryVoltage();
        uint8_t soc = HAL::Sensor::getBatteryPercentage();
        bool charging = HAL::Sensor::isBatteryCharging();

        Serial.print(F("[Battery] U = "));
        Serial.print(u);
        Serial.print(F(" V, SOC = "));
        Serial.print(soc);
        Serial.print(F(" %, Status = "));
        Serial.println(charging ? F("CHARGING") : F("DISCHARGING/IDLE"));
    }

    // --- Zustandsautomat für die Hardware-Diagnose ---
    switch (currentPhase) {
    case HardwareTestPhase::INIT:
        // Einmalige Übergangsphase
        Serial.println(F("Init abgeschlossen, starte Motor-Test ..."));
        phaseStartTime = now;
        currentPhase = HardwareTestPhase::MOTOR_TEST;
        break;

    case HardwareTestPhase::MOTOR_TEST:
        // Non-blocking Motor-Test
        HAL::Motor::motorTestLogic();

        // Übergang nach fester Testzeit
        if (now - phaseStartTime > PHASE_DURATION_MS) {
            HAL::Motor::stop();
            Serial.println(F("Motor-Test fertig, starte Servo-Test ..."));
            phaseStartTime = now;
            currentPhase = HardwareTestPhase::SERVO_TEST;
        }
        break;

    case HardwareTestPhase::SERVO_TEST:
        // Non-blocking Servo-Test (interne Zeitsteuerung in
        // HAL::Sensor::servoTest)
        HAL::Sensor::servoTest();

        if (now - phaseStartTime > PHASE_DURATION_MS) {
            Serial.println(F("Servo-Test fertig, starte RGB-Test ..."));
            phaseStartTime = now;
            currentPhase = HardwareTestPhase::RGB_TEST;
        }
        break;

    case HardwareTestPhase::RGB_TEST:
        // Non-blocking RGB-Test (interne Zeitsteuerung in HAL::Sensor::rgbTest)
        HAL::Sensor::rgbTest();

        if (now - phaseStartTime > PHASE_DURATION_MS) {
            Serial.println(F("RGB-Test fertig, starte Ultraschall-Test ..."));
            phaseStartTime = now;
            currentPhase = HardwareTestPhase::ULTRASONIC_TEST;
        }
        break;

    case HardwareTestPhase::ULTRASONIC_TEST:
        // Non-blocking Ultraschall-Test (interne Zeitsteuerung in
        // HAL::Sensor::ultrasonicTest)
        HAL::Sensor::ultrasonicTest();

        if (now - phaseStartTime > PHASE_DURATION_MS) {
            Serial.println(F("Ultraschall-Test fertig, starte IR-Test ..."));
            phaseStartTime = now;
            currentPhase = HardwareTestPhase::IR_TEST;
        }
        break;

    case HardwareTestPhase::IR_TEST:
        // Non-blocking IR-Test (interne Zeitsteuerung in HAL::Sensor::irTest)
        HAL::Sensor::irTest();

        if (now - phaseStartTime > PHASE_DURATION_MS) {
            Serial.println(F("IR-Test fertig, Diagnose abgeschlossen."));
            phaseStartTime = now;
            currentPhase = HardwareTestPhase::FINISHED;
        }
        break;

    case HardwareTestPhase::FINISHED: {
        // Einmalige Abschlussaktion
        static bool finishedLogged = false;
        if (!finishedLogged) {
            HAL::Motor::stop();
            Serial.println(F("--- HARDWARE DIAGNOSE FERTIG ---"));

            // Abschließenden Batteriestatus loggen
            float uEnd = HAL::Sensor::getBatteryVoltage();
            uint8_t sEnd = HAL::Sensor::getBatteryPercentage();
            Serial.print(F("[Battery] Ende: U = "));
            Serial.print(uEnd);
            Serial.print(F(" V, SOC = "));
            Serial.print(sEnd);
            Serial.println(F(" %"));

            finishedLogged = true;
        }
        // Danach: kein weiterer aktiver Test, Loop bleibt idle
        break;
    }

    default:
        // Fallback – sollte im Normalfall nicht erreicht werden
        break;
    }
}
