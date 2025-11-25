/**
 * @file    Actuator.h
 * @brief   HAL-Schnittstelle für Aktoren (Kameraservo und RGB-LEDs).
 *
 * @details
 * Dieses Modul kapselt alle nicht-antriebsrelevanten Aktoren des Rovers:
 *  - Kameraservo (Neigung / Blickrichtung),
 *  - RGB-LED-Streifen bzw. -Leiste für Status- und Diagnosesignale.
 *
 * Rolle im System:
 *  - Stellt eine schlanke, plattformnahe Schnittstelle für die Anwendung
 *    und Diagnosetools bereit.
 *  - Versteckt Bibliotheksdetails (z. B. @c Servo , @c SoftPWM ) und
 *    konkrete Pin-Zuordnungen ( @c Pin::Servo , @c Pin::RGB_* ).
 *
 * Typische Verwendung:
 *  - @c HAL::Actuator::init() im @c setup() aufrufen,
 *  - während der Hardware-Diagnose @c rgbTest() und @c servoTest()
 *    zyklisch aufrufen,
 *  - im Fahrbetrieb @c setCameraAngle() für die Ausrichtung der Kamera nutzen.
 */

#pragma once

#include <Arduino.h>
#include <stdint.h>

namespace HAL::Actuator {

/**
 * @brief Initialisiert Kameraservo und RGB-LEDs.
 *
 * @details
 * Führt folgende Schritte aus (Implementierungsdetails in @c Actuator.cpp ):
 *  - Startet die SoftPWM-Logik für RGB-LEDs (und ggf. weitere Kanäle).
 *  - Hängt den Kameraservo an den zugehörigen Servo-Pin an und fährt ihn in
 *    eine neutrale Startposition (typisch 90°).
 *  - Setzt alle RGB-Kanäle in einen definierten Grundzustand (aus, kein Fade).
 *
 * @pre
 *  - Die Pins müssen gemäß @c Pins.h korrekt mit dem Shield/Board verdrahtet
 *    sein.
 *
 * @note
 *  Diese Funktion sollte genau einmal im @c setup() der Anwendung aufgerufen
 *  werden, bevor andere Aktor-Funktionen verwendet werden.
 */
void init();

/**
 * @brief RGB-LED-Testsequenz (Rot → Grün → Blau).
 *
 * @details
 * Führt eine einfache, nicht-blockierende Testsequenz für die RGB-Beleuchtung
 * aus, bei der in festen Zeitabständen zwischen den Grundfarben gewechselt
 * wird:
 *  - Phase 0: Rot,
 *  - Phase 1: Grün,
 *  - Phase 2: Blau.
 *
 * Typischer Einsatz:
 *  - Hardware-Diagnose,
 *  - optisches Feedback während Testläufen.
 *
 * @note
 * Die Funktion ist so ausgelegt, dass sie zyklisch aus einer @c loop()- oder
 * Diagnose-Routine aufgerufen werden kann, ohne den Programmablauf zu
 * blockieren (zeitgesteuerter Zustand, kein @c delay() notwendig).
 */
void rgbTest();

/**
 * @brief Setzt den Kameraservo auf einen gewünschten Winkel [°].
 *
 * @details
 * - Erwarteter Sollwertbereich: 0–180° (Standard-Servobereich),
 * - intern wird der Winkel auf einen sicheren, mechanisch verträglichen
 *   Bereich begrenzt (Clamping), um Anschläge und Überlast zu vermeiden.
 *
 * Typische Verwendung:
 *  - Ausrichten der Kamera für unterschiedliche Missionsprofile
 *    (z. B. „Bodenblick“, „Horizont“, „Hindernisbereich“).
 *
 * @param angleDeg Zielwinkel in Grad (Sollwert); Werte außerhalb des
 *                 erlaubten Bereichs werden intern auf Min/Max begrenzt.
 */
void setCameraAngle(int angleDeg);

/**
 * @brief Nicht-blockierender Servotest (Sweep zwischen Min/Max).
 *
 * @details
 * Führt einen kontinuierlichen, langsamen Sweep des Kameraservos zwischen
 * definierten Minimal- und Maximalwinkeln durch:
 *  - Bewegung in kleinen Schrittweiten (z. B. 2°),
 *  - periodischer Wechsel der Bewegungsrichtung (Ping-Pong),
 *  - ohne blockierende Wartezeiten, geeignet für Aufruf im regulären
 *    @c loop() -Takt.
 *
 * Typischer Einsatz:
 *  - Überprüfung der mechanischen Freiheit des Servos,
 *  - Sichtkontrolle der Auslenkung und Geräuschentwicklung,
 *  - Lern- und Demozwecke (Servo-Bewegungsprinzip).
 */
void servoTest();

} // namespace HAL::Actuator
