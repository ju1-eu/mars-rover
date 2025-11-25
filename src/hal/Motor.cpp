/**
 * @file       Motor.cpp
 * @brief      Low-Level-Treiber für DC-Motoren mittels Software-PWM.
 *
 * @details
 * Dieses Modul steuert eine zweikanalige H-Brücke (z. B. L298N oder
 * TB6612FNG) für einen Differenzialantrieb an. Die Ansteuerung der
 * Motorpins erfolgt über die SoftPWM-Bibliothek, um auch auf Pins ohne
 * Hardware-PWM ein PWM-Signal bereitstellen zu können.
 *
 * Aufgaben:
 *  - Kapselung der H-Brücken-Logik (Vorwärts/Rückwärts/Coast),
 *  - Begrenzung der PWM-Werte auf System-Grenzen (Safety),
 *  - Bereitstellung einer Testsequenz zur Funktionskontrolle der Motoren.
 *
 * @hardware
 *  - H-Brücke: Pins gemäß @c Pins.h
 *  - Board:   Arduino Uno / SunFounder R3-kompatibel
 *
 * @dependency
 *  - SoftPWM Library (zeitkritisch, CPU-intensiv; kann andere Interrupts
 *    stören, z. B. bei Ultraschallmessungen).
 */

#include "hal/Motor.h"
#include "Config.h"
#include "Pins.h"
#include <Arduino.h>
#include <SoftPWM.h>

// ----------------------------------------------------------------------------
// ANONYMOUS NAMESPACE
// Dient der Kapselung: Symbole sind nur in dieser Übersetzungseinheit sichtbar
// und kollidieren nicht mit gleichnamigen Funktionen in anderen Dateien.
// ----------------------------------------------------------------------------
namespace {

/**
 * @brief Untere Empfehlung für Dauerfahrten als Anteil von @c Config::SpeedMax.
 *
 * @details
 * Dient als Orientierungsgröße für Applikationslogik, die im „Cruise“-Bereich
 * fahren möchte (Einsatz z. B. für spätere Komfortfunktionen).
 */
constexpr float CRUISE_MIN_FACTOR = 0.30f;

/**
 * @brief Obere Empfehlung für Dauerfahrten als Anteil von @c Config::SpeedMax.
 */
constexpr float CRUISE_MAX_FACTOR = 0.60f;

/**
 * @brief Cruise-Faktor, der in der internen Testlogik verwendet wird.
 *
 * @details
 * Für den Motor-Test wird bewusst ein konservativer Wert (20 % von
 * @c SpeedMax ) verwendet, um die Belastung von Mechanik und Umgebung
 * gering zu halten.
 */
constexpr float CRUISE_TEST_FACTOR = 0.20f;

/**
 * @brief Steuert einen einzelnen Motor-Kanal einer H-Brücke an.
 *
 * @details
 * Diese Funktion kapselt die Low-Level-Logik für einen Motor:
 *  - Grenzwertbegrenzung (Clamping) von @p speed auf
 *    [-Config::SpeedMax, Config::SpeedMax].
 *  - Umsetzung der Drehrichtung:
 *      - @p speed > 0 : Vorwärts,
 *      - @p speed < 0 : Rückwärts,
 *      - @p speed == 0 : Coasting (beide Ausgänge LOW).
 *  - Verhindert aktiv den Zustand „beide Pins HIGH“, um Kurzschluss /
 *    aktive Bremsung zu vermeiden (hier: explizit Coasting).
 *
 * @param pinFwd GPIO-Pin für das Vorwärts-Signal (IN1/IN3 der H-Brücke).
 * @param pinRev GPIO-Pin für das Rückwärts-Signal (IN2/IN4 der H-Brücke).
 * @param speed  Sollgeschwindigkeit im Bereich
 *               [-Config::SpeedMax, +Config::SpeedMax].
 *               Das Vorzeichen kodiert die Drehrichtung.
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
 * @brief Initialisiert die SoftPWM-Logik und setzt die Motoren in einen
 *        sicheren Grundzustand.
 *
 * @details
 * - Startet den SoftPWM-Timer (Interrupt-gesteuerte Software-PWM).
 * - Deaktiviert Fading-Effekte für alle Motorpins, um eine direkte und
 *   latenzarme Ansteuerung zu gewährleisten.
 * - Ruft @c stop() auf, um sicherzustellen, dass beide Motoren zu Beginn
 *   spannungsfrei sind.
 *
 * @warning
 * Die SoftPWM-Bibliothek blockiert Interrupts kurzzeitig. Bei zeitkritischen
 * Sensoren (z. B. Ultraschall, Encoder) sollte geprüft werden, ob Hardware-PWM
 * eine robustere Alternative darstellt.
 *
 * @pre
 *  - Die Pinbelegung der H-Brücke ist gemäß @c Pins.h korrekt erfolgt.
 * @post
 *  - Beide Motoren stehen (Coasting, kein Antriebssignal).
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
 *
 * @details
 * Abstraktionsschicht zwischen Kinematik (z. B. @c Logic::Motion ) und
 * der H-Brücke. Die Funktion:
 *  - interpretiert @p leftSpeed und @p rightSpeed als signierte PWM-Werte,
 *  - delegiert die Ansteuerung an @c driveSingleMotor() für jeden Antrieb.
 *
 * Vorzeichenkonvention:
 *  - @p speed > 0 : Vorwärtsdrehung des jeweiligen Motors,
 *  - @p speed < 0 : Rückwärtsdrehung,
 *  - @p speed = 0 : Coasting.
 *
 * @param leftSpeed   Geschwindigkeit des linken Motors
 *                    (negativ = rückwärts, positiv = vorwärts).
 * @param rightSpeed  Geschwindigkeit des rechten Motors
 *                    (negativ = rückwärts, positiv = vorwärts).
 *
 * @note
 * Werte außerhalb des durch @c Config::SpeedMax definierten Bereichs
 * werden intern begrenzt (Clamping).
 */
void setSpeed(int leftSpeed, int rightSpeed) {
    // Mapping der abstrakten "Links/Rechts"-Befehle auf physische Pins
    driveSingleMotor(Pin::MotorL_Forward, Pin::MotorL_Reverse, leftSpeed);
    driveSingleMotor(Pin::MotorR_Forward, Pin::MotorR_Reverse, rightSpeed);
}

/**
 * @brief Stoppt beide Motoren sofort (Coasting).
 *
 * @details
 * Setzt beide Motoren auf @c speed = 0 und delegiert an @c setSpeed() .
 * In der aktuellen Implementierung bedeutet dies:
 *  - Beide H-Brücken-Ausgänge werden auf LOW gesetzt,
 *  - Der Motor rollt aus (keine aktive Bremsung).
 *
 * @note
 * Soll künftig aktives Bremsen implementiert werden, müsste die H-Brücke
 * dafür angepasst (z. B. beide Eingänge HIGH) und das Verhalten hier
 * entsprechend geändert werden.
 */
void stop() { setSpeed(0, 0); }

/**
 * @brief Nicht-blockierende Testsequenz für beide Motoren.
 *
 * @details
 * Ablauf in 1-s-Schritten (basierend auf @c millis() , ohne @c delay() ):
 *
 *   - Schritt 0: Vorwärtsfahrt (beide Motoren vorwärts)
 *   - Schritt 1: Rückwärtsfahrt (beide Motoren rückwärts)
 *   - Schritt 2: Linksdrehung auf der Stelle
 *   - Schritt 3: Rechtsdrehung auf der Stelle
 *   - Schritt 4: Stopp (Coasting, bleibt in diesem Zustand)
 *
 * Eigenschaften:
 *  - Die Funktion ist für zyklischen Aufruf aus einer Diagnose- oder
 *    Testschleife konzipiert (z. B. Hardware-Diagnosephase).
 *  - Die Ausgabegeschwindigkeit wird über statische Variablen gesteuert,
 *    sodass:
 *      - nur bei Schrittwechsel eine Logzeile an @c Serial gesendet wird,
 *      - die Motorbefehle nur beim Übergang zwischen Phasen geändert werden.
 *
 * @note
 * Die Cruise-Testgeschwindigkeit ist fest auf ~20 % von @c SpeedMax
 * eingestellt ( @c CRUISE_TEST_FACTOR ), um den Test reproduzierbar und
 * vergleichsweise sicher zu halten.
 */
void motorTestLogic() {
    static unsigned long lastStepChange = 0;
    static uint8_t step = 0;       // aktueller Testschritt 0..4
    static uint8_t lastStep = 255; // zuletzt geloggter Schritt (255 = ungültig)

    unsigned long now = millis();

    // Erste Initialisierung: sofort Schritt 0 ausführen
    if (lastStepChange == 0) {
        lastStepChange = now;
    } else if (now - lastStepChange >= 1000UL) {
        // Nächste 1-Sekunden-Phase
        lastStepChange = now;
        if (step < 4) {
            ++step; // nach Schritt 4 nicht weiter erhöhen
        }
    }

    // Nur bei geändertem Schritt loggen und Motoren neu setzen
    if (step == lastStep) {
        return; // kein Zustandswechsel -> nichts zu tun
    }
    lastStep = step;

    // Cruise-Testgeschwindigkeit: ~20 % von SpeedMax (bewusst konservativ)
    const int TEST_SPEED =
        static_cast<int>(Config::SpeedMax * CRUISE_TEST_FACTOR);

    switch (step) {
    case 0:
        Serial.println(F("Motor-Test (Logic): Vorwaerts (~20% PWM Cruise)"));
        setSpeed(TEST_SPEED, TEST_SPEED); // beide Motoren vorwärts
        break;

    case 1:
        Serial.println(F("Motor-Test (Logic): Rueckwaerts (~20% PWM Cruise)"));
        setSpeed(-TEST_SPEED, -TEST_SPEED); // beide Motoren rückwärts
        break;

    case 2:
        Serial.println(
            F("Motor-Test (Logic): Linksdrehung (auf der Stelle, ~20% PWM)"));
        setSpeed(-TEST_SPEED, TEST_SPEED); // links rückwärts, rechts vorwärts
        break;

    case 3:
        Serial.println(
            F("Motor-Test (Logic): Rechtsdrehung (auf der Stelle, ~20% PWM)"));
        setSpeed(TEST_SPEED, -TEST_SPEED); // links vorwärts, rechts rückwärts
        break;

    default:
        Serial.println(F("Motor-Test (Logic): Stop"));
        stop(); // beide Motoren ausrollen lassen
        break;
    }
}

} // namespace HAL::Motor
