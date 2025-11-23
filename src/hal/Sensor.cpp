#include "Sensor.h"
#include "Pins.h"
#include <Arduino.h>
#include <Servo.h>
#include <SoftPWM.h> // für PWM-Dimmung der RGB-LED

// Servo-Instanz für Kamera
Servo testServo;

// interne Konstanten nur in dieser Datei sichtbar
namespace {
constexpr int SERVO_MIN_SAFE = 20;  // untere sichere Grenze
constexpr int SERVO_MAX_SAFE = 140; // obere sichere Grenze

// 30 % von 255 ≈ 76,5 -> auf 77 aufgerundet
constexpr uint8_t RGB_BRIGHTNESS = 77;

// Batterie-Grenzen für 2s-Li-Ion-Pack (Galaxy RVR Lesson 13)
constexpr float BATTERY_MIN_V = 6.6f; // ~leer
constexpr float BATTERY_MAX_V = 8.4f; // voll
} // namespace

namespace HAL::Sensor {

void init() {
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

    // IR-Module (digitale Eingänge)
    pinMode(Pin::IR_Left, INPUT);
    pinMode(Pin::IR_Right, INPUT);

    // Ultraschall: Pin 10 wird dynamisch in ultrasonicTest() umgeschaltet
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

void ultrasonicTest() {
    // Messung nur alle 2000 ms, sonst sofort zurück
    static unsigned long lastMeasure = 0;
    static bool hadValidRead = false;
    static float lastDistance = -1.0f;

    unsigned long now = millis();
    const unsigned long MEASURE_INTERVAL_MS = 2000UL;

    if (now - lastMeasure < MEASURE_INTERVAL_MS) {
        return;
    }
    lastMeasure = now;

    // --- TRIG SEQUENCE ---
    pinMode(Pin::Ultrasonic_Trig, OUTPUT);
    digitalWrite(Pin::Ultrasonic_Trig, LOW);
    delayMicroseconds(2);
    digitalWrite(Pin::Ultrasonic_Trig, HIGH);
    delayMicroseconds(10);
    digitalWrite(Pin::Ultrasonic_Trig, LOW);

    pinMode(Pin::Ultrasonic_Echo, INPUT);

    // --- MESSUNG ---
    constexpr float MAX_DISTANCE_CM = 300.0f;       // cm
    constexpr float MIN_DISTANCE_CM = 2.0f;         // Datenblatt-Untergrenze
    constexpr long ULTRASONIC_READ_TIMEOUT = 18000; // us

    unsigned long duration =
        pulseIn(Pin::Ultrasonic_Echo, HIGH, ULTRASONIC_READ_TIMEOUT);

    // Dauer 0 -> Timeout / kein Echo
    if (duration == 0) {
        if (hadValidRead) {
            Serial.println(F("Ultrasonic: Timeout / kein Echo"));
        }
        return;
    }

    // 0.017 ≈ (Schallgeschwindigkeit 0.034 cm/µs) / 2
    float distance = duration * 0.017f;

    // Plausibilitätsprüfung
    if (distance < MIN_DISTANCE_CM || distance > MAX_DISTANCE_CM) {
        if (hadValidRead) {
            Serial.println(F("Ultrasonic: ausserhalb der Reichweite"));
        }
        return;
    }

    hadValidRead = true;

    // Nur ausgeben, wenn sich der Wert merklich geändert hat (≥ 1 cm)
    if (lastDistance < 0.0f || distance < lastDistance - 1.0f ||
        distance > lastDistance + 1.0f) {

        Serial.print(F("Ultrasonic: "));
        Serial.print(distance);
        Serial.println(F(" cm"));

        lastDistance = distance;
    }
    // kein delay(); loop steuert die Aufrufrate
}

bool irLeftBlocked() {
    // Modul-Logik: OUT LOW = Hindernis erkannt
    return digitalRead(Pin::IR_Left) == LOW;
}

bool irRightBlocked() { return digitalRead(Pin::IR_Right) == LOW; }

void irTest() {
    static unsigned long lastPrint = 0;
    static bool lastLeft = false;
    static bool lastRight = false;

    unsigned long now = millis();
    if (now - lastPrint < 50UL)
        return; // 50 ms Polling (nur für Logik)
    lastPrint = now;

    bool left = irLeftBlocked();
    bool right = irRightBlocked();

    if (left != lastLeft || right != lastRight) {
        Serial.print(F("IR Test - Left: "));
        Serial.print(left ? F("BLOCKED") : F("CLEAR"));
        Serial.print(F(" | Right: "));
        Serial.println(right ? F("BLOCKED") : F("CLEAR"));
        lastLeft = left;
        lastRight = right;
    }
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

/**
 * @brief Misst die Batteriespannung über den ADC (Spannungsteiler auf
 * Pin::Battery).
 * @return Batteriespannung in Volt (auf zwei Nachkommastellen gerundet).
 */
float getBatteryVoltage() {
    // Annahme: Spannungsteiler 1:1 (wie im SunFounder-Beispiel),
    // d. h. U_Bat = 2 * U_ADC.
    int adcValue = analogRead(Pin::Battery); // Pin::Battery z. B. A3

    float adcVoltage = static_cast<float>(adcValue) * 5.0f / 1023.0f;
    float batteryVoltage = adcVoltage * 2.0f;

    // auf 2 Nachkommastellen runden
    batteryVoltage =
        static_cast<float>(static_cast<int>(batteryVoltage * 100.0f + 0.5f)) /
        100.0f;

    return batteryVoltage;
}

/**
 * @brief Schätzt den Ladezustand der 2s-Li-Ion-Batterie in Prozent.
 * @return 0–100 [%]
 */
uint8_t getBatteryPercentage() {
    float u = getBatteryVoltage();

    if (u <= BATTERY_MIN_V) {
        return 0;
    }
    if (u >= BATTERY_MAX_V) {
        return 100;
    }

    float pct = (u - BATTERY_MIN_V) / (BATTERY_MAX_V - BATTERY_MIN_V) * 100.0f;
    if (pct < 0.0f)
        pct = 0.0f;
    if (pct > 100.0f)
        pct = 100.0f;

    return static_cast<uint8_t>(pct + 0.5f); // gerundet
}

bool isBatteryCharging() {
    static float lastU = -1.0f;
    static unsigned long lastT = 0;

    unsigned long now = millis();
    // z.B. alle 10 s auswerten
    const unsigned long DT = 10000UL;

    if (lastT == 0 || now - lastT < DT) {
        // noch keine Aussage möglich
        return false;
    }

    float u = getBatteryVoltage();
    float dU = u - lastU;

    lastU = u;
    lastT = now;

    // Heuristik: Anstieg > 0,02 V über ~10 s -> vermutlich Ladung
    return (dU > 0.02f);
}
} // namespace HAL::Sensor
