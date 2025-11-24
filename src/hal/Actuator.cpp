#include "Actuator.h"
#include "Pins.h"
#include <Arduino.h>
#include <Servo.h>
#include <SoftPWM.h>

// Servo-Instanz für Kamera
namespace {
Servo testServo;

// interne Konstanten nur in dieser Datei sichtbar
constexpr int SERVO_MIN_SAFE = 20;  // untere sichere Grenze
constexpr int SERVO_MAX_SAFE = 140; // obere sichere Grenze

// 30 % von 255 ≈ 76,5 -> auf 77 aufgerundet
constexpr uint8_t RGB_BRIGHTNESS = 77;
} // namespace

namespace HAL::Actuator {

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

    // optionale Diagnoseausgabe: nur loggen, wenn sich der Winkel ≥ 10°
    // geändert hat
    if (lastLoggedAngle < 0 || abs(angle - lastLoggedAngle) >= 10) {
        Serial.print(F("Servo Test: Winkel = "));
        Serial.println(angle);
        lastLoggedAngle = angle;
    }
}

} // namespace HAL::Actuator
