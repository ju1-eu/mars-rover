/**
 * @file    main.cpp
 * @brief   Spezial-Tool zur Kalibrierung der IR-Hindernissensoren.
 *
 * @details
 * Dieses Programm dient zur Einstellung der Schaltschwelle der beiden
 * IR-Reflexlichtschranken am Rover. Es liest zyklisch die digitalen
 * Ausgänge der IR-Module ein und stellt deren Zustand in klarer Textform
 * auf dem seriellen Monitor dar:
 *
 *   - LOGIK: LOW  = Hindernis erkannt  ("BLOCKIERT")
 *            HIGH = kein Hindernis     ("Frei")
 *
 * Vorgehensweise bei der Kalibrierung:
 *   1. Objekt in gewünschter Entfernung (z. B. 12 cm) vor den Sensor halten.
 *   2. Potentiometer am jeweiligen IR-Modul so einstellen, dass
 *      - die Status-LED am Modul gerade einschaltet und
 *      - die Anzeige im Terminal von "Frei" auf "BLOCKIERT" wechselt.
 *
 * Die Darstellung im Terminal nutzt einfache ASCII-Balken, um den Zustand
 * der Sensoren auf einen Blick erkennbar zu machen.
 */

#include "Pins.h"
#include <Arduino.h>

/**
 * @brief Initialisiert serielle Schnittstelle und IR-Eingänge.
 *
 * @details
 *  - Öffnet die serielle Schnittstelle mit 115200 Baud.
 *  - Konfiguriert die IR-Sensorpins als Eingänge (gemäß @c Pins.h ):
 *      - @c Pin::IR_Left  -> linker IR-Sensor (z. B. D8)
 *      - @c Pin::IR_Right -> rechter IR-Sensor (z. B. D7)
 *  - Gibt eine kurze Schritt-für-Schritt-Anleitung zur Kalibrierung aus.
 *
 * @note Zwischen Anleitung und Start der Messschleife wird eine kurze
 *       Wartezeit von 2 s eingelegt, damit der Benutzer den seriellen
 *       Monitor öffnen kann.
 */
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

/**
 * @brief Hauptschleife zur visuellen Auswertung der IR-Sensorzustände.
 *
 * @details
 *  - Liest beide IR-Eingänge ein (digitalRead).
 *  - Interpretiert LOW als "BLOCKIERT" und HIGH als "Frei".
 *  - Gibt den Zustand beider Sensoren in einer Zeile aus, inklusive
 *    Balkendarstellung `[██████]` für blockiert.
 *
 * Die Schleife enthält ein kurzes Delay von 100 ms, um den seriellen
 * Monitor nicht zu überfluten und dennoch reaktives Feedback beim Drehen
 * der Potentiometer zu bieten.
 */
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
