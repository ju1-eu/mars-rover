/**
 * @file       DriveAssistant.cpp
 * @brief      Spurhalte- und Kippschutz-Assistent auf Basis der IMU-Daten.
 *
 * @details
 * Dieses Modul implementiert einen einfachen Fahrassistenten für den Rover,
 * der zwei Hauptaufgaben übernimmt:
 *
 *  1. Kippschutz:
 *     - Überwacht den Pitch-Winkel (Nase hoch/runter) der IMU.
 *     - Überschreitet der Betrag von Pitch einen konfigurierten Grenzwert
 *       (@c MAX_PITCH_DEG ), wird ein Sicherheits-Flag gesetzt und der
 *       Aufrufer kann einen Not-Stopp auslösen.
 *
 *  2. Spurhaltung / Drift-Kompensation:
 *     - Nutzt die Gierrate (YawRate), um ungewollte Drehbewegungen zu
 *       erkennen (Drift).
 *     - Ein einfacher P-Regler berechnet aus der Gierrate eine Korrektur,
 *       die links/rechts differenziell auf die Motoren verteilt wird.
 *       Dadurch wird versucht, den Rover auf einer Geradeauslinie zu halten.
 *
 * Integration:
 *  - Das Modul greift direkt auf die IMU-Werte aus @c HAL::Sensor zu
 *    (@c getPitch() , @c getYawRate() ).
 *  - Die berechneten Motorgeschwindigkeiten werden an @c HAL::Motor::setSpeed()
 *    übergeben.
 *  - Die Funktion @c update() ist nicht-blockierend und für den zyklischen
 *    Aufruf in der Hauptschleife ( @c loop() ) vorgesehen.
 */

#include "logic/DriveAssistant.h"
#include "hal/Motor.h"
#include "hal/Sensor.h"
#include <Arduino.h>

namespace Logic::DriveAssistant {

/**
 * @brief Proportional-Verstärkung für die Yaw-Regelung.
 *
 * @details
 * Bestimmt, wie stark der Assistent auf eine gemessene Gierrate reagiert.
 * - Kleine Werte => sanftes Gegenlenken.
 * - Große Werte  => aggressiver Eingriff, potentiell oszillierendes Verhalten.
 */
constexpr float KP_YAW = 2.0f; // Sanftes Gegensteuern

/**
 * @brief Totzone für die Gierrate in °/s.
 *
 * @details
 * Gierraten mit einem Betrag kleiner als @c DEADZONE_DPS werden als 0
 * interpretiert, um Messrauschen der IMU zu unterdrücken und unnötiges
 * „Hin- und Herlenken“ zu vermeiden.
 */
constexpr float DEADZONE_DPS = 3.0f; // Ignoriert kleines Rauschen besser

/**
 * @brief Maximal zulässiger Pitch-Winkel in °.
 *
 * @details
 * Sobald der Betrag von Pitch diesen Wert überschreitet, betrachtet
 * der Assistent die Situation als unsicher (z. B. Kippgefahr) und
 * meldet dies über den Rückgabewert von @c update() .
 *
 * @note
 *  - Typischer Wert im Fahrbetrieb: ~35–45°.
 *  - Für Tests mit aufgebocktem Rover (Räder frei) kann der Wert
 *    höher gewählt werden (z. B. 90°).
 */
constexpr float MAX_PITCH_DEG = 45.0f; // 90° für Test aufgebockt - Räder frei!

/**
 * @brief Initialisiert den DriveAssistant.
 *
 * @details
 * Aktuell ist keine explizite Initialisierung notwendig; die Funktion ist
 * als Erweiterungshaken vorgesehen (z. B. Reset interner Zustände).
 *
 * @note
 * Sollte einmalig im @c setup() der Anwendung aufgerufen werden.
 */
void init() {}

/**
 * @brief Aktualisiert Spurhalte- und Kippschutzlogik und steuert die Motoren.
 *
 * @details
 * Ablauf pro Aufruf:
 *  1. Sensorabfrage:
 *     - @c pitch  = HAL::Sensor::getPitch()
 *     - @c yawRate = HAL::Sensor::getYawRate()
 *  2. Kippschutz:
 *     - Wenn |pitch| > @c MAX_PITCH_DEG → Sicherheitsverletzung;
 *       Funktion liefert @c false zurück (Aufrufer kann Not-Aus auslösen).
 *  3. Driftbestimmung:
 *     - @c drift = -yawRate (Vorzeichenkonvention gemäß Tests).
 *     - Werte innerhalb der Totzone (@c DEADZONE_DPS) werden auf 0 gesetzt.
 *  4. P-Regler:
 *     - @c correction = drift * KP_YAW
 *     - Das Vorzeichen der Korrektur wird so auf beide Motoren verteilt,
 *       dass dem Drift entgegengewirkt wird:
 *         - linker Motor  = baseSpeed - correction
 *         - rechter Motor = baseSpeed + correction
 *  5. Sättigung:
 *     - Motorwerte werden auf [-255, 255] begrenzt.
 *  6. Ausgabe:
 *     - In ~4 Hz werden Debug-Informationen (Status, Gierrate, Motorwerte)
 *       auf die serielle Schnittstelle ausgegeben.
 *
 * @param baseSpeed Basisgeschwindigkeit (PWM) für beide Motoren im Bereich
 *                  0–255. Von dieser Basis aus wird die Lenk-Korrektur
 *                  nach links/rechts addiert bzw. subtrahiert.
 *
 * @retval true   Wenn die Neigung innerhalb des sicheren Bereichs liegt
 *                und die Motoren entsprechend angesteuert wurden.
 * @retval false  Wenn der Pitch-Grenzwert überschritten wurde (Kippgefahr).
 *                In diesem Fall werden keine Motorbefehle mehr verändert;
 *                der Aufrufer ist für den Not-Stopp zuständig.
 */
bool update(uint8_t baseSpeed) {
    float pitch = HAL::Sensor::getPitch();
    float yawRate = HAL::Sensor::getYawRate();

    // 1. Kipp-Schutz
    if (abs(pitch) > MAX_PITCH_DEG) {
        return false;
    }

    // 2. Drift berechnen (Vorzeichenkonvention aus Tests)
    float drift = -yawRate;
    if (abs(drift) < DEADZONE_DPS)
        drift = 0.0f;

    // 3. Regelung (P-Regler auf Gierrate)
    float correction = drift * KP_YAW;

    // Wir tauschen die Vorzeichen bei correction!
    int speedL = static_cast<int>(baseSpeed) - static_cast<int>(correction);
    int speedR = static_cast<int>(baseSpeed) + static_cast<int>(correction);

    // Sättigung der Werte auf den zulässigen Bereich
    speedL = constrain(speedL, -255, 255);
    speedR = constrain(speedR, -255, 255);

    HAL::Motor::setSpeed(speedL, speedR);

    // 4. AUSGABE (Visualisierung)
    static unsigned long lastDebug = 0;
    if (millis() - lastDebug > 250) { // 4x pro Sekunde
        lastDebug = millis();

        Serial.print(F("DA: Gier="));
        Serial.print(yawRate, 1);
        Serial.print(F("°/s "));

        if (drift == 0.0f) {
            Serial.print(F("[GERADEAUS]      "));
        } else if (correction > 0) {
            // Correction > 0 bedeutet: linker Motor schneller -> Rechtskurve.
            // Das tun wir, wenn Drift negativ war (Rover zog nach links).
            Serial.print(F("[LENKE RECHTS ->]"));
        } else {
            Serial.print(F("[<- LENKE LINKS] "));
        }

        Serial.print(F(" | Motor L:"));
        Serial.print(speedL);
        Serial.print(F(" R:"));
        Serial.println(speedR);
    }

    return true;
}

} // namespace Logic::DriveAssistant
