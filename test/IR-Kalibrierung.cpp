/**
 * @file    main.cpp
 * @brief   Spezial-Tool zur IR-Kalibrierung (Hinderniserkennung).
 */

#include "Pins.h"
#include <Arduino.h>

void setup() {
    Serial.begin(115200);

    // Pins definieren (gemäß Pins.h: D8 = Links, D7 = Rechts)
    pinMode(Pin::IR_Left, INPUT);
    pinMode(Pin::IR_Right, INPUT);

    Serial.println(F("--- IR KALIBRIERUNGS-MODUS ---"));
    Serial.println(
        F("1. Objekt in gewuenschter Entfernung (12cm) platzieren."));
    Serial.println(F("2. Potentiometer am blauen Modul drehen."));
    Serial.println(F("3. Ziel: LED am Modul geht an UND Anzeige hier springt "
                     "auf 'BLOCKIERT'."));
    Serial.println(F(
        "------------------------------------------------------------------"));
    delay(2000);
}

void loop() {
    // Sensoren lesen (LOW = Hindernis, HIGH = Frei)
    bool leftBlocked = (digitalRead(Pin::IR_Left) == LOW);
    bool rightBlocked = (digitalRead(Pin::IR_Right) == LOW);

    // --- Grafische Ausgabe für schnelle Lesbarkeit ---

    Serial.print(F("LINKS (D8): "));
    if (leftBlocked) {
        Serial.print(F("[██████] BLOCKIERT   ")); // Visuelles Feedback
    } else {
        Serial.print(F("[      ] Frei        "));
    }

    Serial.print(F(" |   RECHTS (D7): "));
    if (rightBlocked) {
        Serial.println(F("[██████] BLOCKIERT"));
    } else {
        Serial.println(F("[      ] Frei"));
    }

    // Kurze Pause, damit der Serial Monitor nicht überflutet wird,
    // aber schnell genug für Echtzeit-Feedback beim Drehen.
    delay(100);
}
