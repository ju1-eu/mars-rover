/**
 * @file       Motion.cpp
 * @brief      Implementierung der Antriebslogik (Kinematik-Layer).
 *
 * @details
 * Dieses Modul stellt eine High-Level-Schnittstelle für Fahrbefehle eines
 * Differenzialantriebs (Differential Drive) bereit. Es übersetzt abstrakte
 * Kommandos wie „vorwärts fahren“, „rückwärts fahren“ oder „auf der Stelle
 * drehen“ in konkrete Drehzahlanforderungen an die beiden Antriebsräder.
 *
 * Rolle im System:
 *  - Applikationsebene (z. B. Autonomer Modus, Fahrassistent) ruft nur
 *    Funktionen aus @c Logic::Motion auf.
 *  - Die eigentliche Motoransteuerung (PWM, Richtung, H-Brücken) ist im
 *    HAL-Treiber @c HAL::Motor gekapselt.
 *
 * Konventionen:
 *  - Signiertes Geschwindigkeitsmodell im HAL:
 *      - positiver Wert  => Vorwärtsdrehung,
 *      - negativer Wert  => Rückwärtsdrehung,
 *      - 0               => Stillstand.
 *  - Parameterreihenfolge bei @c HAL::Motor::setSpeed(left, right):
 *      - erster Parameter: linker Motor,
 *      - zweiter Parameter: rechter Motor.
 *
 * @author     Jan Unger
 * @version    1.0.0
 * @date       2025-11-22
 */

#include "logic/Motion.h" // High-Level Bewegungslogik (Differential Drive)
#include "hal/Motor.h"    // Direkte Abhängigkeit zum Hardware-Treiber

namespace Logic::Motion {

/**
 * @brief Initialisiert die unterlagerte Motor-Hardware.
 *
 * @details
 * Reicht den Initialisierungsaufruf direkt an den Motor-HAL weiter.
 * Diese Funktion sollte idealerweise einmalig im @c setup() der Anwendung
 * aufgerufen werden, bevor Bewegungsbefehle verwendet werden.
 */
void init() { HAL::Motor::init(); }

/**
 * @brief Fährt geradeaus vorwärts.
 *
 * @details
 * Beide Motoren erhalten dieselbe, positive Geschwindigkeit. Dadurch
 * bewegt sich der Rover (bei identischem Reifendurchmesser/Grip) ohne
 * Lenkeinschlag nach vorne.
 *
 * @param speed PWM-Geschwindigkeit (0–255).
 *              Ein höherer Wert entspricht einer größeren Motorspannung
 *              bzw. einem höheren Drehmoment (abhängig vom Treiber).
 */
void moveForward(uint8_t speed) {
    // Mapping: Beide Motoren gleiche Polarität (+)
    HAL::Motor::setSpeed(speed, speed);
}

/**
 * @brief Fährt geradeaus rückwärts.
 *
 * @details
 * Beide Motoren werden mit negativer Geschwindigkeit angesteuert, um
 * den Rover gleichmäßig rückwärts zu bewegen.
 *
 * @param speed PWM-Geschwindigkeit (0–255).
 *
 * @note
 * Der @c uint8_t -Parameter wird hier durch Vorzeichenwechsel als
 * negativer Wert an den HAL übergeben. Die Implementierung von
 * @c HAL::Motor::setSpeed() muss daher einen signierten Typ
 * (z. B. @c int16_t oder @c int ) erwarten, um die Drehrichtung
 * unterscheiden zu können.
 */
void moveBackward(uint8_t speed) {
    // Mapping: Beide Motoren invertierte Polarität (-)
    HAL::Motor::setSpeed(-speed, -speed);
}

/**
 * @brief Führt eine Drehung auf der Stelle (Pivot Turn) nach links aus.
 *
 * @details
 * Die beiden Antriebsräder werden gegenläufig angesteuert:
 *  - linker Motor: rückwärts,
 *  - rechter Motor: vorwärts.
 *
 * Dadurch entsteht eine Rotation um die vertikale Fahrzeugachse mit
 * theoretischem Wenderadius 0 (Pivot Turn). Diese Manöver setzen eine
 * ausreichend hohe Traktion und einen ebenen Untergrund voraus.
 *
 * @param speed PWM-Geschwindigkeit (0–255) für die Drehbewegung.
 */
void turnLeft(uint8_t speed) {
    // Kinematik: Links rückwärts (-), Rechts vorwärts (+)
    // Resultat: Rotation gegen den Uhrzeigersinn (CCW)
    HAL::Motor::setSpeed(-speed, speed);
}

/**
 * @brief Führt eine Drehung auf der Stelle (Pivot Turn) nach rechts aus.
 *
 * @details
 * Die beiden Antriebsräder werden gegenläufig angesteuert:
 *  - linker Motor: vorwärts,
 *  - rechter Motor: rückwärts.
 *
 * Dies führt zu einer Rotation um die vertikale Fahrzeugachse im
 * Uhrzeigersinn.
 *
 * @param speed PWM-Geschwindigkeit (0–255) für die Drehbewegung.
 */
void turnRight(uint8_t speed) {
    // Kinematik: Links vorwärts (+), Rechts rückwärts (-)
    // Resultat: Rotation im Uhrzeigersinn (CW)
    HAL::Motor::setSpeed(speed, -speed);
}

/**
 * @brief Stoppt alle Antriebe sofort (Hard Stop).
 *
 * @details
 * Reicht den Stopp-Befehl direkt an den Motor-HAL weiter. Das konkrete
 * Verhalten (Auslaufen vs. aktives Bremsen) hängt von der Implementierung
 * von @c HAL::Motor::stop() ab.
 *
 * @warning
 * Für sicherheitskritische Anwendungen (z. B. Betrieb in Menschennähe)
 * sollte geprüft werden, ob der HAL tatsächlich ein aktives Bremsen
 * implementiert oder die Motoren nur in den Leerlauf schaltet.
 */
void stopMove() { HAL::Motor::stop(); }

} // namespace Logic::Motion
