/**
 * @file    main.cpp
 * @brief   Spezial-Tool zur Ultraschall-Kalibrierung (Single-Pin Mode).
 */

#include "Pins.h"
#include <Arduino.h>

// Standard-Faktor für Schallgeschwindigkeit (kann angepasst werden)
float factor = 0.01715f;

void setup() {
    Serial.begin(115200);
    Serial.println();
    Serial.println(F("--- ULTRASCHALL KALIBRIERUNG (Single Pin D10) ---"));
    Serial.println(F("Bitte Lineal anlegen (z.B. bei 10cm)"));
}

void loop() {
    // SCHRITT 1: SENDEN (Pin auf OUTPUT schalten)
    pinMode(Pin::Ultrasonic_Trig, OUTPUT);

    digitalWrite(Pin::Ultrasonic_Trig, LOW);
    delayMicroseconds(2);
    digitalWrite(Pin::Ultrasonic_Trig, HIGH);
    delayMicroseconds(10);
    digitalWrite(Pin::Ultrasonic_Trig, LOW);

    // SCHRITT 2: EMPFANGEN (Sofort auf INPUT umschalten)
    pinMode(Pin::Ultrasonic_Echo, INPUT);

    // SCHRITT 3: MESSEN
    // Timeout auf 30ms erhöht (~5m), um sicherzugehen
    unsigned long duration = pulseIn(Pin::Ultrasonic_Echo, HIGH, 30000);

    // SCHRITT 4: AUSGABE
    if (duration == 0) {
        Serial.println(F("Kein Echo"));
    } else {
        float distance = duration * factor;

        Serial.print(F("Rohwert: "));
        Serial.print(duration);
        Serial.print(F(" us  =>  Gemessen: "));
        Serial.print(distance, 2);
        Serial.println(F(" cm"));
    }

    delay(200); // 5 Messungen pro Sekunde
}
