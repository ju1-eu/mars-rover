/**
 * @file    Sensor.h
 * @brief   Schnittstelle der Sensor-HAL (Ultraschall, IR, Batterie, IMU).
 *
 * @details
 * Bündelt alle Sensorzugriffe des Rovers in einem einheitlichen Interface:
 *  - Ultraschall-Entfernungsmessung (HC-SR04, Single-Pin),
 *  - IR-Hindernissensoren (digital),
 *  - Batteriespannungs- und Ladezustandsermittlung (ADC),
 *  - IMU (MPU6050/GY-521) inklusive Kalibrierung, Sensorfusion und
 *    Getter für Pitch/Roll/YawRate.
 *
 * Die konkreten Implementierungsdetails befinden sich in @c Sensor.cpp .
 * Die Funktionen sind so ausgelegt, dass sie zyklisch aus @c loop()
 * aufgerufen werden können (nicht-blockierend / timinggesteuert).
 *
 * @author  Jan Unger
 */

#pragma once

#include <Arduino.h>

/**
 * @namespace HAL::Sensor
 * @brief Hardware-Abstraktionsschicht für alle Sensorsignale.
 *
 * @details
 * Stellt eine stabile, modulare Schnittstelle für die Applikationslogik
 * bereit und kapselt:
 *  - Pinbelegung,
 *  - Rohdatenzugriffe,
 *  - Plausibilitätsfilter,
 *  - Sensorfusion/Filterung (IMU).
 */
namespace HAL::Sensor {

// ==========================================================================
// BASIS-INITIALISIERUNG
// ==========================================================================

/**
 * @brief Initialisiert alle unterstützten Sensoren (IR, Ultraschall, IMU).
 *
 * @details
 * Führt u. a. folgende Schritte aus:
 *  - setzt IR-Pins als Eingänge,
 *  - bereitet Ultraschall-Pins für die dynamische Nutzung vor,
 *  - initialisiert die IMU (MPU6050) über I²C und prüft WHO_AM_I.
 *
 * @note
 * Sollte genau einmal im @c setup() der Anwendung aufgerufen werden,
 * bevor @c ultrasonicUpdate() , @c imuUpdate() etc. zyklisch genutzt werden.
 */
void init();

// ==========================================================================
// ULTRASCHALL (HC-SR04 oder kompatibel)
// ==========================================================================

/**
 * @brief Führt eine Ultraschallmessung im 100-ms-Raster durch.
 *
 * @details
 * - Sendet einen 10-µs-Triggerimpuls,
 * - misst mit @c pulseIn() die Echo-Pulsdauer,
 * - rechnet in Entfernung [cm] um,
 * - wendet Plausibilitätsgrenzen (z. B. 2–400 cm) an,
 * - speichert das Ergebnis intern (für @c getUltrasonicDistance() ).
 *
 * Die Funktion ist nicht-blockierend im Sinne des Hauptprogrammablaufs
 * (kein @c delay() in ms-Größenordnung) und für zyklische Aufrufe aus
 * @c loop() vorgesehen.
 */
void ultrasonicUpdate();

/**
 * @brief Gibt die letzte gültige Ultraschall-Entfernung zurück.
 *
 * @return Entfernung in Zentimetern.
 *         Bei fehlendem Echo wird ein großer Ersatzwert (z. B. 999 cm)
 *         zurückgegeben, der „kein Hindernis in Reichweite“ signalisiert.
 */
float getUltrasonicDistance();

// ==========================================================================
// IR-SENSOREN (HINDERNISERKENNUNG)
// ==========================================================================

/**
 * @brief Prüft, ob der linke IR-Sensor ein Hindernis erkennt.
 *
 * @retval true   Hindernis erkannt (digitaler Eingang LOW).
 * @retval false  Kein Hindernis (digitaler Eingang HIGH).
 */
bool irLeftBlocked();

/**
 * @brief Prüft, ob der rechte IR-Sensor ein Hindernis erkennt.
 *
 * @retval true   Hindernis erkannt (digitaler Eingang LOW).
 * @retval false  Kein Hindernis (digitaler Eingang HIGH).
 */
bool irRightBlocked();

/**
 * @brief Diagnosefunktion für die IR-Hindernissensoren.
 *
 * @details
 * - Pollt beide IR-Eingänge,
 * - loggt Zustandsänderungen (BLOCKED/CLEAR) in moderater Frequenz
 *   auf die serielle Schnittstelle.
 *
 * @note
 * Eignet sich für Verkabelungs- und Funktionstests der IR-Sensorik.
 */
void irTest();

// ==========================================================================
// BATTERIE-MESSUNG
// ==========================================================================

/**
 * @brief Ermittelt die aktuelle Batteriespannung.
 *
 * @details
 * - Liest den ADC-Wert an @c Pin::Battery (Spannungsteiler),
 * - rechnet in die reale Packspannung (z. B. 2s-Li-Ion) um,
 * - rundet auf eine Nachkommastelle.
 *
 * @return Batteriespannung in Volt.
 */
float getBatteryVoltage();

/**
 * @brief Schätzt den Ladezustand (State of Charge, SOC) der Batterie.
 *
 * @details
 * Setzt eine einfache lineare Kennlinie zwischen:
 *  - U_min → 0 %,
 *  - U_max → 100 %.
 *
 * Werte außerhalb dieses Spannungsbereichs werden saturiert.
 *
 * @return Ladezustand in Prozent (0–100).
 */
uint8_t getBatteryPercentage();

/**
 * @brief Gibt an, ob das System aktuell geladen wird.
 *
 * @details
 * In der aktuellen Implementierung ist dies ein Platzhalter und gibt
 * stets @c false zurück, da kein dedizierter Lade-Status-Pin ausgewertet
 * wird.
 *
 * @retval false (Stub, bis eine echte Ladeerkennung implementiert ist).
 */
bool isBatteryCharging();

// ==========================================================================
// IMU / MPU6050 (GY-521) – KALIBRIERUNG & SENSORFUSION
// ==========================================================================

/**
 * @brief Führt eine Offset-Kalibrierung der IMU durch (Ruhesituation).
 *
 * @details
 * - Liest @p samples Rohmessungen im Ruhezustand (Rover darf sich nicht
 *   bewegen),
 * - bildet Mittelwerte für sämtliche Achsen,
 * - berücksichtigt Überkopf-Montage bei der Z-Achse des Accelerometers
 *   (statistisch erwartete -1g),
 * - speichert die Offsets intern und setzt das Kalibrier-Flag.
 *
 * @param samples Anzahl der Samples, die für die Mittelwertbildung genutzt
 *                werden (Standard: 500).
 *
 * @warning
 * Während der Kalibrierung muss das Fahrzeug unbedingt ruhig stehen, sonst
 * werden falsche Offsets ermittelt.
 */
void imuCalibrate(uint16_t samples = 500);

/**
 * @brief Aktualisiert IMU-Daten und berechnet gefilterte Lagedaten.
 *
 * @details
 * - Liest aktuelle Accel- und Gyro-Rohwerte,
 * - wendet Offsets an und transformiert in den Rover-Frame (Überkopf-Setup),
 * - berechnet Accel-only-Winkel (Pitch/Roll),
 * - führt Komplementärfilterung mit Gyro-Integration durch,
 * - aktualisiert interne Zustände (Pitch, Roll, YawRate).
 *
 * Diese Funktion sollte in jedem Loop-Durchlauf oder in einem festen
 * Zeitraster aufgerufen werden.
 */
void imuUpdate();

// --------------------------------------------------------------------------
// Getter – Zugriff auf gefilterte IMU-Werte
// --------------------------------------------------------------------------

/**
 * @brief Liefert den gefilterten Pitch-Winkel (Nase hoch/runter).
 *
 * @return Pitch in Grad (positiv = Nase hoch).
 */
float getPitch();

/**
 * @brief Liefert den gefilterten Roll-Winkel (seitliches Kippen).
 *
 * @return Roll in Grad (Vorzeichen gemäß definierter Achsenkonvention).
 */
float getRoll();

/**
 * @brief Liefert die gefilterte Gierrate (YawRate).
 *
 * @details
 * - basiert auf der Z-Achse des Gyroskops,
 * - Vorzeichen so gewählt, dass positive Werte einer definierten
 *   Drehrichtung (z. B. Linksdrehung) entsprechen.
 *
 * @return Gierrate in Grad pro Sekunde [°/s].
 */
float getYawRate();

/**
 * @brief Diagnosefunktion für die IMU (Pitch/Roll/YawRate).
 *
 * @details
 * - ruft intern @c imuUpdate() auf,
 * - gibt die aktuellen Werte in lesbarer Form (reduzierte Frequenz)
 *   auf die serielle Schnittstelle aus,
 * - kann bei Überschreiten von Schwellwerten (z. B. Pitch > 45°)
 *   Warnungen ausgeben.
 *
 * @note
 * Eignet sich zur Verifikation der Einbaulage, Filterparameter und
 * Kalibrierung.
 */
void imuTest();

} // namespace HAL::Sensor
