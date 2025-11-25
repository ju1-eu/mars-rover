/**
 * @file    Sensor.cpp
 * @brief   Implementierung der Sensor-HAL (IMU, Ultraschall, IR, Batterie).
 *
 * @details
 * Dieses Modul kapselt alle Sensorzugriffe des Rovers in einer Hardware
 * Abstraktionsschicht (HAL). Es stellt eine konsolidierte Schnittstelle für:
 *
 *  - Distanzmessung per Ultraschall (Single-Pin-Trig/Echo),
 *  - Hinderniserkennung über IR-Sensoren (digital),
 *  - Batteriespannungs- und Ladezustandsmessung (ADC),
 *  - Lagedetektion (Pitch/Roll) und Gierrate (YawRate) mittels MPU6050 (GY-521)
 *    inklusive Kalibrierung und Komplementärfilter.
 *
 * Ziel:
 *  - Einheitlicher Zugriff auf Sensordaten,
 *  - klare Trennung von Hardware-Zugriff und Applikationslogik,
 *  - Wiederverwendbarkeit in Diagnosetools und Fahrlogik (DriveAssistant).
 */

#include "hal/Sensor.h"
#include "Config.h"
#include "Pins.h"
#include <Arduino.h>
#include <Wire.h>

// ---------------------------------------------------------------------------
// Datei-lokale Konstanten & Variablen (Anonymes Namespace)
// ---------------------------------------------------------------------------
namespace {

// ======================= BATTERIE / ADC =======================

/**
 * @brief Untere Spannungsgrenze für das 2s-Batteriepack (vollständig entladen).
 *
 * @details
 * Unterhalb dieses Wertes wird der Ladezustand (SOC) auf 0 % gesetzt.
 */
constexpr float BATTERY_MIN_V = 6.6f;

/**
 * @brief Obere Spannungsgrenze für das 2s-Batteriepack (voll geladen).
 *
 * @details
 * Oberhalb dieses Wertes wird der Ladezustand (SOC) auf 100 % gesetzt.
 */
constexpr float BATTERY_MAX_V = 8.4f;

/**
 * @brief Referenzspannung am ADC-Eingang.
 *
 * @details
 * Kalibrierter Wert (z. B. gemessen mit Multimeter) an AREF bzw. VCC.
 */
constexpr float ADC_REF_V = 5.246f;

/**
 * @brief Teilerfaktor des Spannungsteilers für die Batteriemessung.
 *
 * @details
 * Bei einem 1:1-Teiler gilt @c BATTERY_DIVIDER = 2.0 .
 */
constexpr float BATTERY_DIVIDER = 2.0f;

// ======================= IMU (MPU6050) =======================

/** @brief I²C-Adresse des MPU6050. */
constexpr uint8_t MPU_ADDR = 0x68;
/** @brief Registeradresse: Power Management 1. */
constexpr uint8_t REG_PWR_MGMT1 = 0x6B;
/** @brief Registeradresse: Accel X High Byte (Start der Sensordaten). */
constexpr uint8_t REG_ACCEL_XOUT_H = 0x3B;
/** @brief Registeradresse: WHO_AM_I (Identifikation). */
constexpr uint8_t REG_WHO_AM_I = 0x75;

/** @brief LSB/g-Faktor für den Beschleunigungssensor (±2g). */
constexpr float ACC_LSB_PER_G = 16384.0f;
/** @brief LSB/(°/s)-Faktor für das Gyroskop (±250 °/s). */
constexpr float GYRO_LSB_PER_DPS = 131.0f;

// Filter-Parameter

/**
 * @brief Filterkoeffizient des Komplementärfilters.
 *
 * @details
 * @c FILTER_ALPHA = 0.98 bedeutet:
 *  - 98 % Gyro-Anteil (kurzfristig stabil, aber driftend),
 *  - 2 % Accel-Anteil (langfristig stabil, aber rauschanfällig).
 */
constexpr float FILTER_ALPHA = 0.98f; // 98% Gyro, 2% Accel

bool imuAvailable = false;  ///< Flag: IMU über I²C erreichbar.
bool imuCalibrated = false; ///< Flag: Offsets wurden kalibriert.

// Für Ultraschall (persistenter Speicher)
float g_lastDistanceCm = 999.0f; ///< Letzte gültige Distanzmessung [cm].

/**
 * @brief Einfache 3D-Vektorstruktur für interne Berechnungen.
 */
struct Vec3f {
    float x;
    float y;
    float z;
};

// Offsets (IMU-Kalibrierwerte)
Vec3f g_accOffsetLSB{0.0f, 0.0f, 0.0f};
Vec3f g_gyroOffsetLSB{0.0f, 0.0f, 0.0f};

// Globale Winkel-Status (persistieren zwischen Funktionsaufrufen)
float g_pitch = 0.0f; // Nase hoch/runter
float g_roll = 0.0f;  // Seitlich kippen
unsigned long g_lastFilterMicros = 0;

// Letzte Werte im Rover-Frame (für Getter)
Vec3f g_lastAcc_g_rover{0.0f, 0.0f, 0.0f};
Vec3f g_lastGyro_dps_rover{0.0f, 0.0f, 0.0f}; // Z = Yaw Rate

// ---------- Low-Level I²C ----------

/**
 * @brief Schreibt ein Byte in ein Register des MPU6050.
 *
 * @param reg   Zielregisteradresse.
 * @param value Zu schreibender Wert.
 */
void mpuWriteReg(uint8_t reg, uint8_t value) {
    Wire.beginTransmission(MPU_ADDR);
    Wire.write(reg);
    Wire.write(value);
    Wire.endTransmission();
}

/**
 * @brief Liest mehrere aufeinanderfolgende Register des MPU6050.
 *
 * @param startReg Startadresse des ersten Registers.
 * @param buf      Zielpuffer für gelesene Bytes.
 * @param len      Anzahl der zu lesenden Bytes.
 */
void mpuReadBytes(uint8_t startReg, uint8_t *buf, uint8_t len) {
    Wire.beginTransmission(MPU_ADDR);
    Wire.write(startReg);
    Wire.endTransmission(false);
    Wire.requestFrom(static_cast<int>(MPU_ADDR), static_cast<int>(len));
    for (uint8_t i = 0; i < len && Wire.available(); ++i) {
        buf[i] = Wire.read();
    }
}

/**
 * @brief Liest Rohdaten (Accel + Gyro) aus dem MPU6050.
 *
 * @details
 * Es werden 14 Byte ab @c REG_ACCEL_XOUT_H gelesen:
 *  - 6 Byte Accel (X, Y, Z),
 *  - 2 Byte Temperatur (ignoriert),
 *  - 6 Byte Gyro (X, Y, Z).
 *
 * @param ax  Beschleunigung X (Raw-LSB).
 * @param ay  Beschleunigung Y (Raw-LSB).
 * @param az  Beschleunigung Z (Raw-LSB).
 * @param gx  Gyro X (Raw-LSB).
 * @param gy  Gyro Y (Raw-LSB).
 * @param gz  Gyro Z (Raw-LSB).
 */
void readImuRaw(int16_t &ax, int16_t &ay, int16_t &az, int16_t &gx, int16_t &gy,
                int16_t &gz) {
    uint8_t buf[14];
    mpuReadBytes(REG_ACCEL_XOUT_H, buf, 14);
    ax = (static_cast<int16_t>(buf[0]) << 8) | buf[1];
    ay = (static_cast<int16_t>(buf[2]) << 8) | buf[3];
    az = (static_cast<int16_t>(buf[4]) << 8) | buf[5];
    gx = (static_cast<int16_t>(buf[8]) << 8) | buf[9];
    gy = (static_cast<int16_t>(buf[10]) << 8) | buf[11];
    gz = (static_cast<int16_t>(buf[12]) << 8) | buf[13];
}

/**
 * @brief Initialisiert den MPU6050 und prüft die Verbindung.
 *
 * @details
 * Durchführung:
 *  1. Startet I²C mit 100 kHz (konservativ),
 *  2. Aktiviert Wire-Timeout, um Bus-Hänger zu vermeiden,
 *  3. Prüft Erreichbarkeit per @c endTransmission() ,
 *  4. Führt einen Hardware-Reset durch,
 *  5. Weckt den Sensor auf (Power-Management),
 *  6. Liest @c WHO_AM_I und verifiziert, dass der Rückgabewert 0x68 ist.
 *
 * Bei Erfolg:
 *  - setzt Statusvariablen zurück,
 *  - initialisiert Zeitbasis für den Filter.
 *
 * @retval true  IMU erfolgreich initialisiert.
 * @retval false Fehler bei I²C/WHO_AM_I – IMU nicht verfügbar.
 */
bool imuInit() {
    Wire.begin();
    Wire.setClock(100000); // Sicherheits-Geschwindigkeit (100kHz)

    // --- NEU: TIMEOUT ---
    Wire.setWireTimeout(3000, true);
    delay(100);

    // 1. Verbindung prüfen
    Wire.beginTransmission(MPU_ADDR);
    if (Wire.endTransmission() != 0)
        return false;

    // 2. HARD-RESET
    mpuWriteReg(REG_PWR_MGMT1, 0x80);
    delay(100);

    // 3. Aufwecken
    mpuWriteReg(REG_PWR_MGMT1, 0x00);
    delay(100);

    // 4. WHO_AM_I Check
    Wire.beginTransmission(MPU_ADDR);
    Wire.write(REG_WHO_AM_I);
    if (Wire.endTransmission(false) != 0)
        return false;
    if (Wire.requestFrom(static_cast<int>(MPU_ADDR), 1) != 1)
        return false;
    if (Wire.read() != 0x68)
        return false;

    imuCalibrated = false;
    g_pitch = 0.0f;
    g_roll = 0.0f;
    g_lastFilterMicros = micros();
    return true;
}

} // namespace

// ---------------------------------------------------------------------------
// Implementierung HAL::Sensor
// ---------------------------------------------------------------------------
namespace HAL::Sensor {

/**
 * @brief Initialisiert alle Sensoren (IR, Ultraschall, IMU).
 *
 * @details
 * - Setzt IR-Pins als Eingänge,
 * - belässt Ultraschall-Pins zur Laufzeit-Konfiguration in @c
 * ultrasonicUpdate(),
 * - versucht, die IMU zu initialisieren:
 *   - bei Erfolg: @c imuAvailable = true,
 *   - bei Fehler: @c imuAvailable = false und Fehlermeldung auf Serial.
 *
 * @note
 * Sollte einmalig im @c setup() der Anwendung aufgerufen werden.
 */
void init() {
    pinMode(Pin::IR_Left, INPUT);
    pinMode(Pin::IR_Right, INPUT);
    // Ultraschall Pins werden dynamisch gesetzt in update()

    if (imuInit()) {
        imuAvailable = true;
        Serial.println(F("IMU: MPU6050 bereit (Upside-Down Config)."));
    } else {
        imuAvailable = false;
        Serial.println(F("IMU: FEHLER - MPU6050 nicht gefunden."));
    }
}

// ====================== ULTRASCHALL ======================

/**
 * @brief Aktualisiert die Ultraschallmessung (nicht-blockierend im
 * 100-ms-Raster).
 *
 * @details
 * Ablauf:
 *  1. Zeitgesteuerte Ausführung (alle ~100 ms),
 *  2. Sendet 10-µs-Triggerpuls über @c Pin::Ultrasonic_Trig (OUTPUT),
 *  3. Schaltet den Pin auf INPUT und misst mit @c pulseIn() die Echo-Pulsdauer,
 *  4. Rechnet Pulsdauer über einen Kalibrierfaktor in Distanz (cm) um,
 *  5. Führt Plausibilitätsprüfung (2–400 cm) durch und aktualisiert
 *     @c g_lastDistanceCm .
 *
 * Bei @c duration == 0 (Timeout):
 *  - wird @c g_lastDistanceCm auf 999 cm gesetzt („kein Echo“).
 *
 * @note
 * Diese Funktion sollte zyklisch aus der Hauptschleife aufgerufen werden.
 */
void ultrasonicUpdate() {
    static unsigned long lastMeasure = 0;
    unsigned long now = millis();

    // Messung alle 100 ms (schnell genug für Bremsweg)
    if (now - lastMeasure < 100UL)
        return;
    lastMeasure = now;

    // 1. Trigger
    pinMode(Pin::Ultrasonic_Trig, OUTPUT);
    digitalWrite(Pin::Ultrasonic_Trig, LOW);
    delayMicroseconds(2);
    digitalWrite(Pin::Ultrasonic_Trig, HIGH);
    delayMicroseconds(10);
    digitalWrite(Pin::Ultrasonic_Trig, LOW);

    // 2. Echo
    pinMode(Pin::Ultrasonic_Echo, INPUT);
    unsigned long duration =
        pulseIn(Pin::Ultrasonic_Echo, HIGH, 18000); // Max ~3m

    if (duration == 0) {
        g_lastDistanceCm = 999.0f; // Kein Echo = weit weg
        return;
    }

    float distance = duration * 0.01715f; // Kalibrierter Wert

    // Plausibilitätsfilter
    if (distance > 2.0f && distance < 400.0f) {
        g_lastDistanceCm = distance;
    }
}

/**
 * @brief Liefert die letzte gültige Ultraschall-Entfernung.
 *
 * @return Entfernung in Zentimetern.
 *         Bei fehlendem Echo wurde zuvor 999.0f gesetzt.
 */
float getUltrasonicDistance() { return g_lastDistanceCm; }

/**
 * @brief Diagnosefunktion für Ultraschallmessung.
 *
 * @details
 * - Ruft intern @c ultrasonicUpdate() auf,
 * - gibt nur bei signifikant geänderter Distanz (> 1 cm) einen Log-Eintrag
 *   auf Serial aus.
 *
 * @note
 * Geeignet für Hardware-Tests, ohne den seriellen Monitor zu fluten.
 */
void ultrasonicTest() {
    ultrasonicUpdate(); // Ruft die Logik auf
    static float lastPrintDist = -1.0f;

    if (fabs(g_lastDistanceCm - lastPrintDist) > 1.0f) {
        Serial.print(F("Ultrasonic: "));
        Serial.print(g_lastDistanceCm);
        Serial.println(F(" cm"));
        lastPrintDist = g_lastDistanceCm;
    }
}

// ====================== IR SENSOREN ======================

/**
 * @brief Prüft, ob der linke IR-Sensor ein Hindernis detektiert.
 *
 * @retval true  Hindernis erkannt (LOW).
 * @retval false Kein Hindernis (HIGH).
 */
bool irLeftBlocked() { return digitalRead(Pin::IR_Left) == LOW; }

/**
 * @brief Prüft, ob der rechte IR-Sensor ein Hindernis detektiert.
 *
 * @retval true  Hindernis erkannt (LOW).
 * @retval false Kein Hindernis (HIGH).
 */
bool irRightBlocked() { return digitalRead(Pin::IR_Right) == LOW; }

/**
 * @brief Diagnosefunktion für die beiden IR-Hindernissensoren.
 *
 * @details
 * - Pollt beide IR-Eingänge,
 * - loggt Änderungen (oder erste Messung) mit einer Minimalperiode von 50 ms,
 * - gibt den Status beider Seiten (LEFT/RIGHT, BLOCKED/CLEAR) aus.
 *
 * @note
 * Eignet sich zur schnellen Prüfung von Sensorlage, Ausrichtung und
 * Empfindlichkeit (z. B. in einer Hardware-Diagnosephase).
 */
void irTest() {
    static unsigned long lastPrint = 0;
    static bool lastLeft = false;
    static bool lastRight = false;
    static bool firstPrint = true;

    unsigned long now = millis();
    if (!firstPrint && (now - lastPrint < 50UL))
        return;
    lastPrint = now;

    bool left = irLeftBlocked();
    bool right = irRightBlocked();

    if (firstPrint || left != lastLeft || right != lastRight) {
        Serial.print(F("IR Test - Left: "));
        Serial.print(left ? F("BLOCKED") : F("CLEAR"));
        Serial.print(F(" | Right: "));
        Serial.println(right ? F("BLOCKED") : F("CLEAR"));
        lastLeft = left;
        lastRight = right;
        firstPrint = false;
    }
}

// ====================== BATTERIE ======================

/**
 * @brief Ermittelt die aktuelle Batteriespannung.
 *
 * @details
 * Ablauf:
 *  1. Liest ADC-Rohwert an @c Pin::Battery .
 *  2. Rechnet in ADC-Spannung @c adcVoltage um (unter Verwendung von
 *     @c ADC_REF_V ).
 *  3. Multipliziert mit @c BATTERY_DIVIDER , um Packspannung zu erhalten.
 *  4. Rundet auf eine Nachkommastelle.
 *
 * @return Batteriespannung des 2s-Packs in Volt (eine Nachkommastelle).
 */
float getBatteryVoltage() {
    int adcValue = analogRead(Pin::Battery);
    float adcVoltage = static_cast<float>(adcValue) * ADC_REF_V / 1023.0f;
    float batteryVoltage = adcVoltage * BATTERY_DIVIDER;
    return static_cast<float>(static_cast<int>(batteryVoltage * 10.0f + 0.5f)) /
           10.0f;
}

/**
 * @brief Schätzt den Ladezustand (SOC) der Batterie in Prozent.
 *
 * @details
 * Verwendet eine lineare Interpolation zwischen:
 *  - @c BATTERY_MIN_V → 0 %,
 *  - @c BATTERY_MAX_V → 100 %.
 *
 * Werte außerhalb dieses Bereichs werden saturiert.
 *
 * @return Ladezustand in Prozent (0–100).
 */
uint8_t getBatteryPercentage() {
    float u = getBatteryVoltage();
    if (u <= BATTERY_MIN_V)
        return 0;
    if (u >= BATTERY_MAX_V)
        return 100;
    return static_cast<uint8_t>(
        (u - BATTERY_MIN_V) / (BATTERY_MAX_V - BATTERY_MIN_V) * 100.0f + 0.5f);
}

/**
 * @brief Platzhalter für Batterieladeerkennung.
 *
 * @details
 * Aktuell wird kein Hardware-Pin zur Erkennung eines Ladestatus ausgewertet.
 * Die Funktion liefert daher immer @c false .
 *
 * @retval false Aktuell keine Ladesituation erkennbar (Stub).
 */
bool isBatteryCharging() { return false; }

// ====================== IMU (Kalibrierung & Logik) ======================

/**
 * @brief Führt eine Offset-Kalibrierung der IMU in Überkopf-Montage durch.
 *
 * @details
 * Ablauf:
 *  - Prüft, ob die IMU verfügbar ist,
 *  - sammelt @p samples Messwerte in Roh-LSB,
 *  - bildet Mittelwerte für Accel- und Gyroachsen,
 *  - berücksichtigt bei der Z-Achse des Accelerometers die Überkopf-Montage:
 *    - statische Lage erwartet -1g → Offset = Mittelwert + 1g in LSB.
 *  - speichert Offsets in @c g_accOffsetLSB und @c g_gyroOffsetLSB,
 *  - setzt @c imuCalibrated = true.
 *
 * Während der Kalibrierung wird alle 50 Samples ein Punkt auf Serial
 * ausgegeben (Fortschrittsanzeige).
 *
 * @param samples Anzahl der Messungen für die Mittelwertbildung.
 *
 * @warning
 * Während der Kalibrierung muss der Rover unbedingt still stehen, damit
 * die Offsets gültig sind.
 */
void imuCalibrate(uint16_t samples) {
    if (!imuAvailable)
        return;
    Serial.println(F("IMU: Kalibriere (Überkopf)... Ruhig halten!"));

    double sumAx = 0, sumAy = 0, sumAz = 0;
    double sumGx = 0, sumGy = 0, sumGz = 0;

    for (uint16_t i = 0; i < samples; ++i) {
        int16_t ax, ay, az, gx, gy, gz;
        readImuRaw(ax, ay, az, gx, gy, gz);

        sumAx += ax;
        sumAy += ay;
        sumAz += az;
        sumGx += gx;
        sumGy += gy;
        sumGz += gz;

        if (i % 50 == 0)
            Serial.print(F("."));
        delay(3);
    }
    Serial.println();

    g_accOffsetLSB.x = sumAx / samples;
    g_accOffsetLSB.y = sumAy / samples;
    // Z-Achse Überkopf-Korrektur (-1g = -16384 LSB)
    g_accOffsetLSB.z = (sumAz / samples) + ACC_LSB_PER_G;

    g_gyroOffsetLSB.x = sumGx / samples;
    g_gyroOffsetLSB.y = sumGy / samples;
    g_gyroOffsetLSB.z = sumGz / samples;

    imuCalibrated = true;
    Serial.println(F("IMU: Kalibrierung fertig."));
}

/**
 * @brief Aktualisiert IMU-Werte und berechnet gefilterte Winkel (Pitch/Roll).
 *
 * @details
 * Ablauf pro Aufruf:
 *  1. Prüft, ob IMU verfügbar und kalibriert ist.
 *  2. Berechnet Zeitdelta @c dt auf Basis von @c micros() .
 *  3. Liest Rohdaten (Accel, Gyro) aus.
 *  4. Wendet Offsets an und skaliert auf g bzw. °/s.
 *  5. Transformiert in den Rover-Frame (Überkopf-Montage):
 *     - Z-Achse und Gierrate werden invertiert.
 *  6. Berechnet aus Accelerometer-Werten „Accel-only“-Winkel
 *     ( @c acc_pitch , @c acc_roll ).
 *  7. Führt Komplementärfilterung von Pitch/Roll durch:
 *     - Kombination aus integrierter Gyro-Rate und Accel-Winkel.
 *
 * Die gefilterten Winkel werden in @c g_pitch bzw. @c g_roll gehalten
 * und können über die Getter gelesen werden.
 */
void imuUpdate() {
    if (!imuAvailable || !imuCalibrated)
        return;

    unsigned long nowMicros = micros();
    float dt = (nowMicros - g_lastFilterMicros) / 1000000.0f;
    g_lastFilterMicros = nowMicros;

    int16_t ax_raw, ay_raw, az_raw, gx_raw, gy_raw, gz_raw;
    readImuRaw(ax_raw, ay_raw, az_raw, gx_raw, gy_raw, gz_raw);

    // Umrechnen & Offsets
    float ax_g = (ax_raw - g_accOffsetLSB.x) / ACC_LSB_PER_G;
    float ay_g = (ay_raw - g_accOffsetLSB.y) / ACC_LSB_PER_G;
    float az_g = (az_raw - g_accOffsetLSB.z) / ACC_LSB_PER_G;

    float gx_dps = (gx_raw - g_gyroOffsetLSB.x) / GYRO_LSB_PER_DPS;
    float gy_dps = (gy_raw - g_gyroOffsetLSB.y) / GYRO_LSB_PER_DPS;
    float gz_dps = (gz_raw - g_gyroOffsetLSB.z) / GYRO_LSB_PER_DPS;

    // Transformation (Überkopf)
    g_lastAcc_g_rover.x = ax_g;
    g_lastAcc_g_rover.y = ay_g;
    g_lastAcc_g_rover.z = -az_g;

    g_lastGyro_dps_rover.x = gx_dps;
    g_lastGyro_dps_rover.y = gy_dps;
    g_lastGyro_dps_rover.z = -gz_dps;

    // Winkel (Accel)
    float acc_pitch =
        -atan2f(-g_lastAcc_g_rover.x,
                sqrtf(g_lastAcc_g_rover.y * g_lastAcc_g_rover.y +
                      g_lastAcc_g_rover.z * g_lastAcc_g_rover.z)) *
        180.0f / PI;
    float acc_roll =
        atan2f(g_lastAcc_g_rover.y, g_lastAcc_g_rover.z) * 180.0f / PI;

    // Filter
    g_pitch = FILTER_ALPHA * (g_pitch + g_lastGyro_dps_rover.y * dt) +
              (1.0f - FILTER_ALPHA) * acc_pitch;
    g_roll = FILTER_ALPHA * (g_roll + g_lastGyro_dps_rover.x * dt) +
             (1.0f - FILTER_ALPHA) * acc_roll;
}

// Getter

/**
 * @brief Liefert den gefilterten Pitch-Winkel (Nase hoch/runter).
 *
 * @return Pitch in Grad (positiv = Nase hoch).
 */
float getPitch() { return g_pitch; }

/**
 * @brief Liefert den gefilterten Roll-Winkel (Seitliches Kippen).
 *
 * @return Roll in Grad (positiv = Kippung entsprechend Achsenkonvention).
 */
float getRoll() { return g_roll; }

/**
 * @brief Liefert die aktuelle Gierrate (YawRate) im Rover-Frame.
 *
 * @details
 * - basiert auf der Z-Achse des Gyroskops,
 * - Vorzeichen ist so gewählt, dass positive Werte einer Linksdrehung
 *   entsprechen (gemäß Tests und Koordinatensystem).
 *
 * @return Gierrate in °/s.
 */
float getYawRate() { return g_lastGyro_dps_rover.z; }

/**
 * @brief Diagnosefunktion für IMU (Pitch/Roll/YawRate + Kippalarm).
 *
 * @details
 * - Ruft @c imuUpdate() auf,
 * - gibt alle 100 ms die aktuellen Werte auf Serial aus,
 * - signalisiert via Text, wenn der Betrag von Pitch > 45° ist (Kippalarm).
 *
 * @note
 * Eignet sich zur Prüfung der Einbaulage, der Kalibrierung und der
 * Filterparameter.
 */
void imuTest() {
    imuUpdate();
    static unsigned long lastPrint = 0;
    if (millis() - lastPrint > 100) {
        lastPrint = millis();
        Serial.print(F("P/R: "));
        Serial.print(getPitch(), 1);
        Serial.print(F(" / "));
        Serial.print(getRoll(), 1);
        Serial.print(F(" | Yaw: "));
        Serial.println(getYawRate(), 1);
        if (abs(getPitch()) > 45.0f)
            Serial.println(F("!!! KIPP-ALARM !!!"));
    }
}

} // namespace HAL::Sensor
