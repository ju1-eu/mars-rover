/**
 * @file       Motor.h
 * @brief      Schnittstellendefinition (API) für den Motor-Treiber.
 *
 * @details
 * Dieses Modul stellt die Low-Level-HAL-Schnittstelle zur Ansteuerung
 * der DC-Motoren über eine H-Brücke bereit (Differenzialantrieb).
 *
 * Aufgaben:
 *  - Konfiguration der Motorpins und Start der (Software-)PWM.
 *  - Setzen von Geschwindigkeit und Drehrichtung für linken/rechten Motor.
 *  - Sofortiger Stopp (Coasting) für Sicherheits- und Diagnosefälle.
 *  - Bereitstellung einer nicht-blockierenden Selbsttest-Sequenz.
 *
 * Die konkrete Implementierung befindet sich in @c Motor.cpp und nutzt:
 *  - Pinzuordnungen aus @c Pins.h,
 *  - Systemgrenzen aus @c Config.h,
 *  - die SoftPWM-Bibliothek zur PWM-Generierung.
 */

#pragma once

#include <Arduino.h>
#include <stdint.h>

namespace HAL::Motor {

/**
 * @brief Konfiguriert die GPIO-Pins und startet die PWM-Timer.
 *
 * @details
 * - Initialisiert die SoftPWM-Infrastruktur (Timer/Interrupts).
 * - Setzt alle Motorpins in einen definierten, sicheren Grundzustand.
 * - Führt intern einen @c stop() aus, sodass die Motoren zu Beginn
 *   spannungsfrei sind (Coasting).
 *
 * @pre
 *  - Muss einmalig im @c setup() aufgerufen werden, bevor andere
 *    Motorfunktionen verwendet werden.
 *  - Die H-Brücke muss gemäß @c Pins.h korrekt verdrahtet sein.
 *
 * @post
 *  - Motoren stehen (PWM = 0, keine Ansteuerung).
 */
void init();

/**
 * @brief Setzt Motorgeschwindigkeit und -richtung (Open Loop).
 *
 * @details
 * Interpretiert die Sollwerte als signierte PWM-Kommandos:
 *  - @p speed < 0 : Rückwärtsdrehung,
 *  - @p speed = 0 : Stopp (Coasting),
 *  - @p speed > 0 : Vorwärtsdrehung.
 *
 * Die Funktion:
 *  - übernimmt kein Feedback (kein Closed-Loop-Regler),
 *  - begrenzt die Werte intern auf den Bereich
 *    [-Config::SpeedMax, +Config::SpeedMax] (Clamping),
 *  - mappt die abstrakten „links/rechts“-Kommandos auf die physikalischen
 *    H-Brücken-Pins, die in @c Pins.h definiert sind.
 *
 * @param leftSpeed
 *  Sollgeschwindigkeit für den linken Motor.
 *  Vorzeichenkonvention:
 *   - < 0 rückwärts,
 *   - = 0 Stopp (Coasting),
 *   - > 0 vorwärts.
 *
 * @param rightSpeed
 *  Sollgeschwindigkeit für den rechten Motor.
 *  Gleiche Vorzeichenkonvention wie @p leftSpeed.
 *
 * @note
 *  Die effektive Drehzahl hängt zusätzlich von Versorgungsspannung,
 *  Last, Motorcharakteristik und H-Brücke ab.
 */
void setSpeed(int leftSpeed, int rightSpeed);

/**
 * @brief Schaltet die Motoren sofort ab.
 *
 * @details
 * Setzt beide Motorkanäle auf PWM = 0 und delegiert an @c setSpeed(0, 0) .
 * In der aktuellen Implementierung bedeutet dies:
 *  - beide H-Brücken-Ausgänge werden LOW gesetzt,
 *  - die Motoren rollen frei aus (kein aktives elektrisches Bremsen).
 *
 * @safety
 * Sollte in Not-Aus-Routinen, bei Kommunikationsverlust oder bei
 * Verletzung von Sicherheitsgrenzen (z. B. Kippwinkel, Hinderniserkennung)
 * aufgerufen werden.
 */
void stop();

/**
 * @brief Nicht-blockierender Motor-Selbsttest.
 *
 * @details
 * Wird zyklisch aus @c loop() (z. B. in einer Diagnose-Phase MOTOR_TEST)
 * aufgerufen und durchläuft in 1-s-Schritten folgende Sequenz:
 *
 *  - Schritt 0: Vorwärtsfahrt (beide Motoren vorwärts),
 *  - Schritt 1: Rückwärtsfahrt (beide Motoren rückwärts),
 *  - Schritt 2: Linksdrehung auf der Stelle,
 *  - Schritt 3: Rechtsdrehung auf der Stelle,
 *  - Schritt 4: Stopp (Coasting, verbleibt in diesem Zustand).
 *
 * Eigenschaften:
 *  - Zeitsteuerung über @c millis() (keine blockierenden @c delay() -Aufrufe).
 *  - Log-Ausgabe nur bei Schrittwechsel, um den seriellen Monitor nicht zu
 *    überfluten.
 *  - Für die Testgeschwindigkeit wird intern ein konservativer Anteil
 *    von @c Config::SpeedMax verwendet (z. B. ~20 %).
 *
 * @note
 *  Eignet sich zur Überprüfung von:
 *   - Verkabelung und Drehrichtung der Motoren,
 *   - H-Brücken-Funktion,
 *   - SoftPWM-Konfiguration.
 */
void motorTestLogic();

} // namespace HAL::Motor
