/**
 * @file       Actuator.cpp
 * @brief      Implementierung der Aktorik-HAL (Licht & Zusatz-Bewegung).
 *
 * @details
 * Dieses Modul kümmert sich um alles, was sich bewegt oder leuchtet,
 * aber NICHT zum Antriebsstrang gehört.
 *
 * Architektur-Einordnung & Didaktik:
 * - **Abstraktion:** Die App-Ebene ruft `setCameraAngle(90)` auf, ohne zu
 * wissen, dass dahinter eine PWM-Generierung via `Servo.h` an Pin 6 steckt.
 * - **Sicherheit:** Software-Limits ("Clamping") schützen die Hardware
 * vor mechanischer Zerstörung (Servo auf Anschlag).
 * - **Concurrency:** Test-Funktionen sind "nicht-blockierend" implementiert,
 * damit der Hauptprozessor nicht in einem `delay()` gefangen ist.
 *
 * @dependency SoftPWM (für RGB LEDs an beliebigen Pins)
 * @dependency Servo   (Standard Arduino Lib)
 */

#include "hal/Actuator.h"
#include "Pins.h"
#include <Arduino.h>
#include <Servo.h>
#include <SoftPWM.h>

// ==========================================================================
// INTERNE HELPER (Anonyme Namespace -> "Private" im File-Scope)
// ==========================================================================
namespace {

/**
 * @brief Servo-Objekt für die Kameraneigung.
 * @note  Global in dieser Datei, aber unsichtbar für andere Module.
 */
Servo cameraServo;

// --- Mechanische Sicherheitsgrenzen ---
// Der Servo könnte technisch 0-180°, aber bei <20° oder >140°
// stößt die Kamera-Halterung gegen das Chassis.
constexpr int SERVO_MIN_SAFE = 20;
constexpr int SERVO_MAX_SAFE = 140;

// --- Optische Einstellungen ---
// 30% Helligkeit reicht für Tests völlig aus und spart Strom.
constexpr uint8_t RGB_BRIGHTNESS = 77;

} // namespace

namespace HAL::Actuator {

/**
 * @brief Initialisierung der Peripherie.
 */
void init() {
    // 1. SoftPWM starten (Timer-basiertes PWM auf beliebigen Pins)
    SoftPWMBegin();

    // 2. Servo verbinden und in sichere Startposition fahren
    cameraServo.attach(Pin::Servo);
    cameraServo.write(90); // 90° = Mitte/Geradeaus

    // 3. RGB-LEDs konfigurieren
    // FadeTime = 0 bedeutet sofortiges Umschalten (hartes Blinken)
    SoftPWMSetFadeTime(Pin::RGB_R, 0, 0);
    SoftPWMSetFadeTime(Pin::RGB_G, 0, 0);
    SoftPWMSetFadeTime(Pin::RGB_B, 0, 0);

    // Initial alles aus
    SoftPWMSet(Pin::RGB_R, 0);
    SoftPWMSet(Pin::RGB_G, 0);
    SoftPWMSet(Pin::RGB_B, 0);
}

/**
 * @brief Setzt den Kamerawinkel mit mechanischem Schutz.
 *
 * @details
 * Implementiert "Clamping": Werte außerhalb des sicheren Bereichs
 * werden auf die Grenzen gezwungen.
 */
void setCameraAngle(int angleDeg) {
    // Schutzlogik (Clamping)
    if (angleDeg < SERVO_MIN_SAFE) {
        angleDeg = SERVO_MIN_SAFE;
    } else if (angleDeg > SERVO_MAX_SAFE) {
        angleDeg = SERVO_MAX_SAFE;
    }

    cameraServo.write(angleDeg);
}

/**
 * @brief Nicht-blockierender RGB-Test (State Machine).
 *
 * @details
 * Statt `delay(1000)` nutzen wir `millis()`.
 * Das ermöglicht dem Rover, während des Blinkens weiter Sensoren
 * zu lesen oder Motoren zu steuern.
 */
void rgbTest() {
    static unsigned long lastChange = 0; // "Gedächtnis" für letzte Zeit
    static uint8_t state = 0;            // 0=Rot, 1=Grün, 2=Blau

    unsigned long now = millis();

    // Ist 1 Sekunde vergangen? Wenn nein, sofort zurück (nicht blockieren!)
    if (now - lastChange < 1000UL) {
        return;
    }
    lastChange = now;

    // Zustandsautomat für die Farben
    switch (state) {
    case 0: // ROT
        Serial.println(F("[Test] RGB: Rot"));
        SoftPWMSet(Pin::RGB_R, RGB_BRIGHTNESS);
        SoftPWMSet(Pin::RGB_G, 0);
        SoftPWMSet(Pin::RGB_B, 0);
        break;
    case 1: // GRÜN
        Serial.println(F("[Test] RGB: Grün"));
        SoftPWMSet(Pin::RGB_R, 0);
        SoftPWMSet(Pin::RGB_G, RGB_BRIGHTNESS);
        SoftPWMSet(Pin::RGB_B, 0);
        break;
    case 2: // BLAU
        Serial.println(F("[Test] RGB: Blau"));
        SoftPWMSet(Pin::RGB_R, 0);
        SoftPWMSet(Pin::RGB_G, 0);
        SoftPWMSet(Pin::RGB_B, RGB_BRIGHTNESS);
        break;
    }

    // Nächster Zustand (Rotlieren 0 -> 1 -> 2 -> 0 ...)
    state = (state + 1) % 3;
}

/**
 * @brief Nicht-blockierender Servo-Sweep.
 *
 * @details
 * Bewegt den Servo langsam hin und her ("Winken").
 * Auch hier: Kein `delay()`, sondern kleine Schritte alle 40ms.
 */
void servoTest() {
    static unsigned long lastMove = 0;
    static int angle = 90;
    static int direction = 1; // +1 = hoch, -1 = runter

    // Logging-Filter: Nur Änderungen ausgeben, um Serial nicht zu fluten
    static int lastLoggedAngle = -1;

    unsigned long now = millis();

    // Update-Rate: 40ms (~25 Hz) für flüssige Bewegung
    if (now - lastMove < 40UL) {
        return;
    }
    lastMove = now;

    // Neuen Winkel berechnen
    angle += (direction * 2); // 2 Grad pro Schritt

    // Richtungsumkehr an den Grenzen (Ping-Pong)
    if (angle >= SERVO_MAX_SAFE) {
        angle = SERVO_MAX_SAFE;
        direction = -1; // Rückwärtsgang
    } else if (angle <= SERVO_MIN_SAFE) {
        angle = SERVO_MIN_SAFE;
        direction = 1; // Vorwärtsgang
    }

    // Hardware ansteuern
    cameraServo.write(angle);

    // Diagnose-Ausgabe (nur alle 10 Grad)
    if (abs(angle - lastLoggedAngle) >= 10) {
        Serial.print(F("[Test] Servo: "));
        Serial.print(angle);
        Serial.println(F(" Grad"));
        lastLoggedAngle = angle;
    }
}

} // namespace HAL::Actuator
