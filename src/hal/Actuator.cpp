/**
 * @file       Actuator.cpp
 * @brief      Implementierung der Aktorik-HAL (Servo + RGB-LEDs) für den Rover.
 *
 * @details
 * Dieses Modul kapselt alle Aktor-Funktionen, die NICHT direkt Antrieb/Motor
 * betreffen:
 *  - Kameraservo (Neigung),
 *  - RGB-LED-Leiste (Status-/Diagnoseanzeige).
 *
 * Rolle im System:
 *  - Stellt eine einheitliche Schnittstelle für die Anwendung bereit
 *    (z. B. Hardware-Diagnose, autonome Fahrmodi).
 *  - Versteckt Bibliotheksdetails ( @c Servo , @c SoftPWM ) und
 *    Pin-Zuordnungen ( @c Pin::Servo , @c Pin::RGB_* ).
 */

#include "Actuator.h"
#include "Pins.h"
#include <Arduino.h>
#include <Servo.h>
#include <SoftPWM.h>

// Servo-Instanz für Kamera
namespace {

/**
 * @brief Servo-Objekt für die Kameraneigung.
 *
 * @details
 * Wird ausschließlich in dieser Übersetzungseinheit verwendet und über
 * @c setCameraAngle() bzw. @c servoTest() angesteuert.
 */
Servo testServo;

// interne Konstanten nur in dieser Datei sichtbar

/**
 * @brief Untere sichere Grenze für den Kameraserwo-Winkel [°].
 *
 * @details
 * Verhindert mechanische Überlastung durch zu starke Abwärtsbewegung.
 */
constexpr int SERVO_MIN_SAFE = 20;

/**
 * @brief Obere sichere Grenze für den Kameraserwo-Winkel [°].
 *
 * @details
 * Verhindert mechanische Überlastung durch zu starke Aufwärtsbewegung.
 */
constexpr int SERVO_MAX_SAFE = 140;

/**
 * @brief Standard-Helligkeit für RGB-Test (ca. 30 % von 255).
 *
 * @details
 * Wird in @c rgbTest() für alle drei Grundfarben verwendet, um ein
 * gut sichtbares, aber nicht blendendes Testsignal zu erzeugen.
 */
constexpr uint8_t RGB_BRIGHTNESS = 77;

} // namespace

namespace HAL::Actuator {

/**
 * @brief Initialisiert alle Aktoren (Servo + RGB-LEDs).
 *
 * @details
 * - Startet SoftPWM (für RGB-Leiste und ggf. Motor-Pins),
 * - hängt den Kameraserwo an den Servo-Pin an und fährt ihn in Neutralstellung
 *   (90°),
 * - konfiguriert alle RGB-Pins als SoftPWM-Ausgänge ohne Fade-Effekte
 *   und setzt sie initial auf 0 (aus).
 *
 * @note
 * Diese Funktion sollte einmalig im @c setup() der Anwendung vor allen
 * anderen Aktor-Funktionen aufgerufen werden.
 */
void init() {
    SoftPWMBegin();

    // Servo Initialisierung (Neutralstellung)
    testServo.attach(Pin::Servo);
    testServo.write(90);

    // RGB-Pins: SoftPWM ohne Fade, initial aus
    SoftPWMSetFadeTime(Pin::RGB_R, 0, 0);
    SoftPWMSetFadeTime(Pin::RGB_G, 0, 0);
    SoftPWMSetFadeTime(Pin::RGB_B, 0, 0);
    SoftPWMSet(Pin::RGB_R, 0);
    SoftPWMSet(Pin::RGB_G, 0);
    SoftPWMSet(Pin::RGB_B, 0);
}

/**
 * @brief Nicht-blockierender RGB-Testlauf (Rot → Grün → Blau).
 *
 * @details
 * - Wechselt im Abstand von 1 s zeilenweise durch die drei Grundfarben:
 *   0: Rot, 1: Grün, 2: Blau.
 * - Nutzt @c RGB_BRIGHTNESS als PWM-Wert (~30 % Helligkeit).
 * - Gibt für jede Phase eine kurze Statusmeldung via Serial aus.
 *
 * Die Funktion ist nicht-blockierend:
 *  - Bei Aufruf häufiger als alle 1000 ms wird sofort zurückgegeben,
 *  - Es werden keine Delays verwendet.
 *
 * @note
 * Eignet sich für Hardware-Diagnosen und visuelles Feedback bei Tests.
 */
void rgbTest() {
    // Farbwechsel alle 1000 ms, kein Dauer-Spam in Serial
    static unsigned long lastChange = 0;
    static uint8_t colorPhase = 0; // 0 = Rot, 1 = Grün, 2 = Blau

    unsigned long now = millis();
    if (now - lastChange < 1000UL) {
        return;
    }
    lastChange = now;

    switch (colorPhase) {
    case 0:
        Serial.println(F("RGB Test: ROT (30% Helligkeit)"));
        SoftPWMSet(Pin::RGB_R, RGB_BRIGHTNESS);
        SoftPWMSet(Pin::RGB_G, 0);
        SoftPWMSet(Pin::RGB_B, 0);
        break;
    case 1:
        Serial.println(F("RGB Test: GRUEN (30% Helligkeit)"));
        SoftPWMSet(Pin::RGB_R, 0);
        SoftPWMSet(Pin::RGB_G, RGB_BRIGHTNESS);
        SoftPWMSet(Pin::RGB_B, 0);
        break;
    default:
        Serial.println(F("RGB Test: BLAU (30% Helligkeit)"));
        SoftPWMSet(Pin::RGB_R, 0);
        SoftPWMSet(Pin::RGB_G, 0);
        SoftPWMSet(Pin::RGB_B, RGB_BRIGHTNESS);
        break;
    }

    colorPhase = (colorPhase + 1) % 3;
}

/**
 * @brief Setzt den Kameraserwo auf einen gewünschten Winkel.
 *
 * @details
 * - Erwarteter Wertebereich: 0–180° (Servo-Bibliothek),
 * - tatsächlich verwendeter Bereich:
 *   - @c SERVO_MIN_SAFE  ≤ angleDeg ≤  @c SERVO_MAX_SAFE.
 *
 * Liegt @p angleDeg außerhalb des sicheren Bereichs, wird der Wert
 * auf die nächste Grenze geklemmt (Clamping), bevor er an den Servo
 * übergeben wird.
 *
 * @param angleDeg Zielwinkel in Grad (Sollwert), wird intern begrenzt.
 */
void setCameraAngle(int angleDeg) {
    // Clamping auf sicheren Bereich
    if (angleDeg < SERVO_MIN_SAFE) {
        angleDeg = SERVO_MIN_SAFE;
    }
    if (angleDeg > SERVO_MAX_SAFE) {
        angleDeg = SERVO_MAX_SAFE;
    }
    testServo.write(angleDeg);
}

/**
 * @brief Nicht-blockierender Test-Sweep des Kameraserwomechanismus.
 *
 * @details
 * - Fährt den Kameraserwo in 2°-Schritten zwischen @c SERVO_MIN_SAFE
 *   und @c SERVO_MAX_SAFE hin und her (Ping-Pong-Bewegung).
 * - Nutzt ein Schrittintervall von 40 ms für eine sanfte Bewegung.
 * - Gibt den aktuellen Winkel nur aus, wenn sich dieser mindestens um 10°
 *   gegenüber der letzten Logmeldung geändert hat, um Serial-Spam zu vermeiden.
 *
 * Ablauf:
 *  - interne Zustände (@c angle , @c dir , @c lastLoggedAngle ) werden statisch
 *    gespeichert, um bei jedem Aufruf dort weiterzumachen, wo der vorherige
 *    Aufruf aufgehört hat,
 *  - keine blockierenden @c delay() -Aufrufe, geeignet für Einsatz in
 *    Diagnoseloops.
 */
void servoTest() {
    // Nicht-blockierender Sweep zwischen SERVO_MIN_SAFE und SERVO_MAX_SAFE
    static unsigned long lastMove = 0;
    static int angle = 90;
    static int dir = 1;                 // +1 Richtung MAX, -1 Richtung MIN
    static int lastLoggedAngle = -1000; // für reduzierte Serial-Ausgabe

    unsigned long now = millis();
    const unsigned long STEP_INTERVAL_MS = 40UL; // sanftes Bewegen

    if (now - lastMove < STEP_INTERVAL_MS) {
        return;
    }
    lastMove = now;

    angle += dir * 2; // 2°-Schritte
    if (angle >= SERVO_MAX_SAFE) {
        angle = SERVO_MAX_SAFE;
        dir = -1;
    } else if (angle <= SERVO_MIN_SAFE) {
        angle = SERVO_MIN_SAFE;
        dir = 1;
    }

    testServo.write(angle);

    // Optionale Diagnoseausgabe: nur loggen, wenn sich der Winkel ≥ 10°
    // geändert hat.
    if (lastLoggedAngle < 0 || abs(angle - lastLoggedAngle) >= 10) {
        Serial.print(F("Servo Test: Winkel = "));
        Serial.println(angle);
        lastLoggedAngle = angle;
    }
}

} // namespace HAL::Actuator
