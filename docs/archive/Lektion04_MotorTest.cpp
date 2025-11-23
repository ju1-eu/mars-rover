/**
 * @file       HardwareTest.cpp
 * @brief      End-of-Line (EOL) Test für die Antriebseinheit.
 * @details    Eine deterministische Sequenz, um Verkabelung und Drehrichtung
 * der Motoren zu validieren. Dient der Abnahme der Hardware vor der
 * eigentlichen Programmierung der Logik.
 * * @test       Prüfschritte:
 * 1. Vorwärts: Beide Räder drehen "nach vorne"? (Falls nein: Motor verpolt)
 * 2. Rückwärts: Beide Räder drehen "nach hinten"? (Falls nein: H-Brücke Logik
 * prüfen)
 * 3. Drehung: Räder drehen gegenläufig?
 * * @warning    SICHERHEITSHINWEIS: Roboter vor dem Test "aufbocken" (Räder in
 * der Luft), damit er nicht vom Tisch fährt, falls die Sensoren noch nicht
 * aktiv sind!
 * @author     Jan Unger
 * @version    1.0.0
 * @date       2025-11-22
 */

#include "Config.h"
#include "hal/Motor.h"
#include <Arduino.h>

// --- Definitionen ---

/**
 * @enum  TestState
 * @brief Schritte der Validierungs-Sequenz.
 */
enum class TestState {
    Start,       ///< Sicherheits-Wartezeit nach Reset
    Vorwaerts,   ///< Prüfen der Polarität (Plus/Plus)
    Pause1,      ///< Optische Kontrolle & Auslaufen
    Rueckwaerts, ///< Prüfen der Invertierung (Minus/Minus)
    Pause2,      ///< Optische Kontrolle & Auslaufen
    DrehenLinks, ///< Prüfen der Differenzial-Logik (Minus/Plus)
    Fertig       ///< Sicherer Endzustand
};

// Globale Status-Variablen
TestState currentState = TestState::Start;
unsigned long lastStateChange = 0;

void setup() {
    // Baudrate passend zum Bootloader/Monitor
    Serial.begin(115200);
    Serial.println("[DIAGNOSTIC] --- START: Motor Hardware Verification ---");
    Serial.println("[INSTRUCTION] Ensure robot wheels are elevated!");

    // Hardware initialisieren (Pins, Timer)
    HAL::Motor::init();

    // Defined State: Sicherstellen, dass PWM initial 0 ist.
    HAL::Motor::stop();
}

void loop() {
    unsigned long now = millis();

    // ---------------------------------------------------------
    // TIMER LOGIK (Time-Slicing)
    // ---------------------------------------------------------
    // Hier wird eine feste Zeitbasis für jeden Testschritt definiert.
    // Jeder Schritt (Action) bleibt genau 'Config::TestPhaseDuration' lang
    // aktiv.
    if (now - lastStateChange > Config::TestPhaseDuration) {
        lastStateChange = now;

        switch (currentState) {
        case TestState::Start:
            // Trigger: Test beginnt
            // Action: Keine (Warten war implizit)
            // Transition: Nächster Schritt
            currentState = TestState::Vorwaerts;
            break;

        case TestState::Vorwaerts:
            Serial.print("[TEST] Forward Check | Speed: ");
            Serial.println(Config::SpeedTest);

            // ACTION: Motoren bestromen.
            // VERIFIKATION: Drehen beide Räder in Fahrtrichtung?
            // Falls ein Rad rückwärts dreht -> Kabel am Motor tauschen!
            HAL::Motor::setSpeed(Config::SpeedTest, Config::SpeedTest);

            currentState = TestState::Pause1;
            break;

        case TestState::Pause1:
            Serial.println("[TEST] Pause / Coasting");
            HAL::Motor::stop();
            currentState = TestState::Rueckwaerts;
            break;

        case TestState::Rueckwaerts:
            Serial.print("[TEST] Reverse Check | Speed: -");
            Serial.println(Config::SpeedTest);

            // ACTION: Negative Werte übergeben.
            // VERIFIKATION: Software-PWM muss Vorzeichen korrekt handhaben.
            HAL::Motor::setSpeed(-Config::SpeedTest, -Config::SpeedTest);

            currentState = TestState::Pause2;
            break;

        case TestState::Pause2:
            Serial.println("[TEST] Pause / Coasting");
            HAL::Motor::stop();
            currentState = TestState::DrehenLinks;
            break;

        case TestState::DrehenLinks:
            Serial.println("[TEST] Differential Logic Check (Turn Left)");

            // ACTION: Panzer-Wende (Pivot Turn).
            // VERIFIKATION: Linkes Rad rückwärts, Rechtes Rad vorwärts.
            HAL::Motor::setSpeed(-Config::SpeedTest, Config::SpeedTest);

            currentState = TestState::Fertig;
            break;

        case TestState::Fertig:
            // End State: Wird periodisch aufgerufen, setzt aber immer wieder
            // STOP. Dies ist sicherer als eine leere Endlosschleife
            // `while(1);`, da der Microcontroller weiter auf serielle Befehle
            // reagieren könnte.
            if (currentState == TestState::Fertig) {
                static bool msgShown = false;
                if (!msgShown) {
                    Serial.println(
                        "[RESULT] Test Sequence Complete. Reset to restart.");
                    msgShown = true;
                }
                HAL::Motor::stop(); // Redundante Sicherheit
            }
            break;
        }
    }
}
