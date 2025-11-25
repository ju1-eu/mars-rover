/**
 * @file    main.cpp
 * @brief   Spezial-Tool zur Ultraschall-Kalibrierung (Single-Pin Mode).
 *
 * @details
 * Dieses Hilfsprogramm dient zur Kalibrierung eines
 * Ultraschall-Entfernungssensors im Single-Pin-Betrieb:
 *  - TRIG-Impuls und ECHO-Auswertung erfolgen über denselben physischen Pin
 *    (in @c Pins.h typischerweise D10 definiert).
 *  - Der Mikrocontroller schaltet den Pin zur Laufzeit zwischen OUTPUT
 *    (Senden des Triggerpulses) und INPUT (Empfangen des Echos) um.
 *
 * Ziel:
 *  - Referenzmessung mit Lineal (z. B. 10 cm Abstand) durchführen,
 *  - Rohdauer @c duration (µs) ablesen,
 *  - Kalibrierfaktor @c factor ggf. anpassen, bis berechnete Distanz
 *    der realen Entfernung entspricht.
 *
 * Formel (vereinfachtes Prinzip):
 *  - @c distance_cm ≈ @c duration_µs * @c factor
 *  - @c factor basiert auf Schallgeschwindigkeit und der Tatsache,
 *    dass das Signal Hin- und Rückweg zurücklegt.
 */

#include "Pins.h"
#include <Arduino.h>

/**
 * @brief Umrechnungsfaktor von Pulsdauer (µs) in Entfernung (cm).
 *
 * @details
 *  - Näherung basierend auf Schallgeschwindigkeit bei Raumtemperatur.
 *  - Wird direkt als Multiplikator auf die von @c pulseIn() gelieferte
 *    Pulsdauer angewendet:
 *      @code
 *      distance_cm = duration_µs * factor;
 *      @endcode
 *
 * @note
 *  Bei signifikanten Abweichungen zwischen Anzeige und Referenz-Lineal
 *  kann dieser Faktor experimentell leicht angepasst werden (Fein-Tuning).
 */
float factor = 0.01715f;

/**
 * @brief Initialisiert serielle Schnittstelle und zeigt Kalibrierhinweise an.
 *
 * @details
 *  - Startet @c Serial mit 115200 Baud.
 *  - Gibt eine kurze Anleitung zur Positionierung des Lineals bzw. der
 *    Referenzdistanz aus.
 *
 * @note
 *  Die eigentliche Pin-Konfiguration (Eingang/Ausgang) erfolgt zyklisch
 *  in @c loop() , da für den Single-Pin-Betrieb zwischen TRIG und ECHO
 *  umgeschaltet werden muss.
 */
void setup() {
    Serial.begin(115200);
    Serial.println();
    Serial.println(F("--- ULTRASCHALL KALIBRIERUNG (Single Pin D10) ---"));
    Serial.println(F("Bitte Lineal anlegen (z.B. bei 10cm)"));
}

/**
 * @brief Führt zyklisch eine Ultraschallmessung im Single-Pin-Modus durch.
 *
 * @details
 * Ablauf pro Zyklus:
 *  1. Senden:
 *     - Pin als OUTPUT konfigurieren,
 *     - kurzen LOW-Impuls, dann 10 µs HIGH-Trigger, anschließend wieder LOW.
 *  2. Empfangen:
 *     - Pin sofort auf INPUT umschalten,
 *     - mit @c pulseIn() die HIGH-Pulsdauer des Echos messen.
 *  3. Auswertung:
 *     - Falls @c duration == 0 → kein Echo erkannt,
 *     - sonst Entfernung in cm über @c factor berechnen und ausgeben.
 *
 * @note
 *  - Timeout ist auf 30 ms gesetzt (≈ 5 m Reichweite),
 *  - Ausgabefrequenz: ca. 5 Messungen pro Sekunde ( @c delay(200) ).
 */
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

    // 5 Messungen pro Sekunde (Lesbarkeit im Serial Monitor)
    delay(200);
}
