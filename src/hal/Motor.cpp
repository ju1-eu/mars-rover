/**
 * @file       Motor.cpp
 * @brief      Low-Level-Treiber für DC-Motoren (H-Brücke).
 *
 * @details
 * Dieses Modul übersetzt "Geschwindigkeit" (-255 bis +255) in elektrische
 * Signale für die H-Brücke.
 *
 * Architektur-Einordnung & Didaktik:
 * - **Hardware-Abstraktion:** Der Code hier weiß, welche Pins HIGH/LOW
 * sein müssen, damit der Motor dreht. Die Schichten darüber (Logic) wissen
 * nur "Fahre Vorwärts".
 * - **Safety:** Hier findet das "Clamping" statt. Egal was die Logik fordert,
 * wir senden niemals mehr als `Config::SpeedMax` an die Hardware.
 *
 * @dependency SoftPWM (für Pins ohne Hardware-PWM Support)
 */

#include "hal/Motor.h"
#include "Config.h"
#include "Pins.h"
#include <Arduino.h>
#include <SoftPWM.h>

// ==========================================================================
// INTERNE HELPER (Anonyme Namespace)
// ==========================================================================
namespace {

/**
 * @brief Steuert einen einzelnen Kanal der H-Brücke.
 *
 * @details
 * H-Brücken-Logik (L298N / TB6612):
 * - Vorwärts: IN1 = PWM, IN2 = 0
 * - Rückwärts: IN1 = 0, IN2 = PWM
 * - Coast (Rollen): IN1 = 0, IN2 = 0
 * - Brake (Bremsen): IN1 = 1, IN2 = 1 (Hier nicht implementiert, da SoftPWM)
 *
 * @param pinFwd Pin für Vorwärts-Signal
 * @param pinRev Pin für Rückwärts-Signal
 * @param speed  Geschwindigkeit (-255 bis +255)
 */
void driveSingleMotor(uint8_t pinFwd, uint8_t pinRev, int speed) {
    // 1. Safety: Clamping (Begrenzung auf Hardware-Limits)
    if (speed > Config::SpeedMax) {
        speed = Config::SpeedMax;
    } else if (speed < -Config::SpeedMax) {
        speed = -Config::SpeedMax;
    }

    // 2. Ansteuerung
    if (speed > 0) {
        // Vorwärts
        SoftPWMSet(pinFwd, speed);
        SoftPWMSet(pinRev, 0);
    } else if (speed < 0) {
        // Rückwärts (Speed ist negativ, wir brauchen positiven PWM-Wert)
        SoftPWMSet(pinFwd, 0);
        SoftPWMSet(pinRev, -speed);
    } else {
        // Stillstand (Coasting)
        SoftPWMSet(pinFwd, 0);
        SoftPWMSet(pinRev, 0);
    }
}

} // end anonymous namespace

// ==========================================================================
// PUBLIC API IMPLEMENTATION
// ==========================================================================
namespace HAL::Motor {

/**
 * @brief Initialisiert die Motortreiber.
 */
void init() {
    // Timer für Software-PWM starten
    SoftPWMBegin();

    // Fade-Time auf 0 setzen für direkte Reaktion (wichtig für PID!)
    SoftPWMSetFadeTime(Pin::MotorL_Forward, 0, 0);
    SoftPWMSetFadeTime(Pin::MotorL_Reverse, 0, 0);
    SoftPWMSetFadeTime(Pin::MotorR_Forward, 0, 0);
    SoftPWMSetFadeTime(Pin::MotorR_Reverse, 0, 0);

    stop();
}

/**
 * @brief Setzt die Geschwindigkeit beider Motoren.
 *
 * @param leftSpeed  -255 (Rück) bis +255 (Vor)
 * @param rightSpeed -255 (Rück) bis +255 (Vor)
 */
void setSpeed(int leftSpeed, int rightSpeed) {
    driveSingleMotor(Pin::MotorL_Forward, Pin::MotorL_Reverse, leftSpeed);
    driveSingleMotor(Pin::MotorR_Forward, Pin::MotorR_Reverse, rightSpeed);
}

/**
 * @brief Schaltet Motoren stromlos (Coasting).
 */
void stop() { setSpeed(0, 0); }

/**
 * @brief Nicht-blockierender Hardware-Test.
 *
 * @details
 * Durchläuft zyklisch die Grundfunktionen:
 * Vorwärts -> Rückwärts -> Links -> Rechts -> Stop.
 * Nutzt `millis()` statt `delay()`, um den Prozessor nicht zu blockieren.
 */
void motorTestLogic() {
    static unsigned long lastChange = 0;
    static uint8_t step = 0; // 0..4

    unsigned long now = millis();

    // Zustandswechsel alle `Config::TestPhaseDuration` ms
    if (now - lastChange < Config::TestPhaseDuration) {
        return;
    }
    lastChange = now;

    // Wir nutzen den definierten Test-Speed aus Config.h
    int spd = Config::SpeedTest;

    switch (step) {
    case 0:
        Serial.println(F("[Test] Motor: Vorwärts"));
        setSpeed(spd, spd);
        break;
    case 1:
        Serial.println(F("[Test] Motor: Rückwärts"));
        setSpeed(-spd, -spd);
        break;
    case 2:
        Serial.println(F("[Test] Motor: Drehung Links"));
        setSpeed(-spd, spd);
        break;
    case 3:
        Serial.println(F("[Test] Motor: Drehung Rechts"));
        setSpeed(spd, -spd);
        break;
    default:
        Serial.println(F("[Test] Motor: Stopp"));
        stop();
        // Reset nach einer Pause, oder hier verweilen?
        // Wir setzen step auf -1, damit es im nächsten Takt auf 0 springt
        step = 255; // Overflow bei ++step -> 0
        break;
    }

    if (step != 255) {
        step++;
    }
}

} // namespace HAL::Motor
