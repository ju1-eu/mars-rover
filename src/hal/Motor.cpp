/**
 * @file       Motor.cpp
 * @brief      Low-Level Treiber für DC-Motoren mittels Software-PWM.
 * @details    Steuert eine H-Brücke (z.B. L298N oder TB6612FNG) an.
 *             Nutzt die 'SoftPWM' Bibliothek, da der Mikrocontroller ggf.
 *             nicht genügend Hardware-PWM-Kanäle an den gewählten Pins hat.
 * @hardware   H-Brücke an Pins definiert in Pins.h
 * @dependency SoftPWM Library (CPU-intensiv!)
 */

#include "hal/Motor.h"
#include "Config.h"
#include "Pins.h"
#include <Arduino.h>
#include <SoftPWM.h>

// ----------------------------------------------------------------------------
// ANONYMOUS NAMESPACE
// Dient der Kapselung: Funktionen hierdrin sind "private" für diese Datei.
// Verhindert Namenskonflikte mit anderen driveSingleMotor-Funktionen im Linker.
// ----------------------------------------------------------------------------
namespace {

// Empfohlener Cruise-Bereich: 30–60 % von SpeedMax
constexpr float CRUISE_MIN_FACTOR = 0.30f;
constexpr float CRUISE_MAX_FACTOR = 0.60f;
// Für den Test nutzen wir eine feste Cruise-Geschwindigkeit in der Mitte:
constexpr float CRUISE_TEST_FACTOR = 0.20f;

/**
 * @brief      Steuert einen einzelnen Kanal der H-Brücke an.
 * @details    Setzt die Logik für die Drehrichtung um und verhindert
 *             ungültige Zustände (beide Pins HIGH = Kurzschluss/Bremse).
 *
 * @param pinFwd  GPIO-Pin für Vorwärts-Signal
 * @param pinRev  GPIO-Pin für Rückwärts-Signal
 * @param speed   Geschwindigkeit (-Config::SpeedMax bis +Config::SpeedMax).
 *                Vorzeichen bestimmt die Richtung.
 */
void driveSingleMotor(uint8_t pinFwd, uint8_t pinRev, int speed) {
    // --- SAFETY: Clamping (Grenzwertbegrenzung) ---
    if (speed > Config::SpeedMax) {
        speed = Config::SpeedMax;
    }
    if (speed < -Config::SpeedMax) {
        speed = -Config::SpeedMax;
    }

    // --- H-Bridge Logik ---
    if (speed > 0) {
        // Vorwärts: Fwd gepulst, Rev auf Ground (Low)
        SoftPWMSet(pinFwd, speed);
        SoftPWMSet(pinRev, 0);
    } else if (speed < 0) {
        // Rückwärts: Fwd auf Ground, Rev gepulst
        SoftPWMSet(pinFwd, 0);
        SoftPWMSet(pinRev, -speed); // speed ist negativ -> -speed ist positiv
    } else {
        // Stopp / Neutral: Beide Leitungen auf Low -> Motor rollt aus
        // (Coasting)
        SoftPWMSet(pinFwd, 0);
        SoftPWMSet(pinRev, 0);
    }
}

} // end anonymous namespace

// ----------------------------------------------------------------------------
// PUBLIC API IMPLEMENTATION
// ----------------------------------------------------------------------------
namespace HAL::Motor {

/**
 * @brief Initialisiert die PWM-Timer und Pin-Zustände.
 * @warning SoftPWM blockiert Interrupts kurzzeitig. Bei Timing-Problemen
 *          mit Sensoren (z.B. Ultraschall) prüfen, ob Hardware-PWM nötig ist.
 */
void init() {
    // Startet den Timer-Interrupt für die Software-PWM
    SoftPWMBegin();

    // --- LATENCY OPTIMIZATION ---
    SoftPWMSetFadeTime(Pin::MotorL_Forward, 0, 0);
    SoftPWMSetFadeTime(Pin::MotorL_Reverse, 0, 0);
    SoftPWMSetFadeTime(Pin::MotorR_Forward, 0, 0);
    SoftPWMSetFadeTime(Pin::MotorR_Reverse, 0, 0);

    // Sicherer Startzustand
    stop();
}

/**
 * @brief Setzt die Geschwindigkeit für den Differenzialantrieb.
 * @param leftSpeed   Geschwindigkeit links (negativ = rückwärts)
 * @param rightSpeed  Geschwindigkeit rechts (negativ = rückwärts)
 */
void setSpeed(int leftSpeed, int rightSpeed) {
    // Mapping der abstrakten "Links/Rechts"-Befehle auf physische Pins
    driveSingleMotor(Pin::MotorL_Forward, Pin::MotorL_Reverse, leftSpeed);
    driveSingleMotor(Pin::MotorR_Forward, Pin::MotorR_Reverse, rightSpeed);
}

/**
 * @brief Wrapper für sofortigen Stopp.
 */
void stop() { setSpeed(0, 0); }

/**
 * @brief Nicht-blockierende Testsequenz für beide Motoren.
 *
 * Ablauf in 1-s-Schritten:
 *   0: Vorwärts
 *   1: Rückwärts
 *   2: Linksdrehung (auf der Stelle)
 *   3: Rechtsdrehung (auf der Stelle)
 *   4: Stopp (bleibt dann so)
 *
 * Wird zyklisch aus main.cpp (Phase MOTOR_TEST) aufgerufen.
 * Die Zeitsteuerung nutzt millis(), kein delay().
 *
 * Testgeschwindigkeit:
 *   Cruise-Speed im Bereich ca. 30–60 % von SpeedMax.
 *   Hier fest auf ~20 % gesetzt (CRUISE_TEST_FACTOR), um Akku und Getriebe
 *   zu schonen, aber eine klar sichtbare Bewegung zu behalten.
 */
void motorTestLogic() {
    static unsigned long lastStepChange = 0;
    static uint8_t step = 0; // 0..4

    unsigned long now = millis();

    // Erste Initialisierung: sofort Schritt 0 ausführen
    if (lastStepChange == 0) {
        lastStepChange = now;
    } else if (now - lastStepChange < 1000UL) {
        // Noch innerhalb der aktuellen 1-Sekunden-Phase: nichts ändern
        return;
    } else {
        // Nächste 1-Sekunden-Phase
        lastStepChange = now;
        if (step < 4) {
            ++step; // nach Schritt 4 nicht weiter erhöhen
        }
    }

    // Cruise-Testgeschwindigkeit: ~20 % von SpeedMax (innerhalb 30–60 %-Band)
    const int TEST_SPEED =
        static_cast<int>(Config::SpeedMax * CRUISE_TEST_FACTOR);

    switch (step) {
    case 0:
        Serial.println(F("Motor-Test: Vorwaerts (~20% PWM Cruise)"));
        setSpeed(TEST_SPEED, TEST_SPEED); // beide Motoren vorwärts
        break;

    case 1:
        Serial.println(F("Motor-Test: Rueckwaerts (~20% PWM Cruise)"));
        setSpeed(-TEST_SPEED, -TEST_SPEED); // beide Motoren rückwärts
        break;

    case 2:
        Serial.println(
            F("Motor-Test: Linksdrehung (auf der Stelle, ~20% PWM)"));
        setSpeed(-TEST_SPEED, TEST_SPEED); // links rückwärts, rechts vorwärts
        break;

    case 3:
        Serial.println(
            F("Motor-Test: Rechtsdrehung (auf der Stelle, ~20% PWM)"));
        setSpeed(TEST_SPEED, -TEST_SPEED); // links vorwärts, rechts rückwärts
        break;

    default:
        Serial.println(F("Motor-Test: Stop"));
        stop(); // beide Motoren ausrollen lassen
        break;
    }
}

} // namespace HAL::Motor
