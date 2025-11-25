/**
 * @file    MPU6050_Rover_Filtered.cpp
 * @brief   IMU-Sensordatenverarbeitung und Sensorfusion für Rover-Navigation.
 *
 * @details
 * ZWECK:
 * Dieses Modul implementiert die Lagebestimmung (Attitude & Heading Reference)
 * für einen mobilen Roboter unter Verwendung eines MPU-6050 Sensors (GY-521).
 * Es fusioniert Rohdaten von Beschleunigungssensor und Gyroskop mittels eines
 * Komplementärfilters, um stabile, driftarme Winkel für die Lageregelung
 * bereitzustellen.
 *
 * HARDWARE-VORAUSSETZUNGEN & MONTAGE:
 * - [cite_start]Sensor: GY-521 Breakout Board (MPU-6050)[cite: 3].
 * - Kommunikation: I2C Bus (SCL/SDA) bei 400kHz.
 * - [cite_start]Versorgungsspannung (VCC): 3.3V - 5V (Board-Regler)[cite: 63].
 * - Einbaulage (KRITISCH):
 *   1. X-Achse: Zeigt in Fahrtrichtung (Vorne).
 *   2. Y-Achse: Zeigt zur Seite (Querachse).
 *   3. Z-Achse: Zeigt zum Boden (Überkopf-Montage / Upside-Down).
 *
 * KOORDINATEN-TRANSFORMATION (Chip -> Fahrzeug):
 * Da der Sensor "kopfüber" montiert ist, werden die Achsen wie folgt
 * transformiert:
 * - Accel Z: Invertiert (Erwartungswert -1g wird zu +1g im Fahrzeug-Frame).
 * - Gyro Z (Gierrate): Negiert, um die korrekte Drehrichtung im Fahrzeug-System
 *   zu gewährleisten (Rechtsdrehung = positiv nach Rechte-Hand-Regel).
 * - Pitch (Nicken): Invertiert, damit "Nase hoch" als positiver Winkel
 *   ausgegeben wird.
 *
 * ALGORITHMUS:
 * - [cite_start]Initialisierung: Power-Management-Reset und
 *   Gyro/Accel-Konfiguration[cite: 30, 43].
 * - Kalibrierung: Ermittelt statische Offsets (Bias) beim Start
 *   (Ruhelage erforderlich).
 * - Filterung: Komplementärfilter (α = 0.98) kombiniert die Kurzzeitpräzision
 *   des Gyroskops mit der Langzeitstabilität des Beschleunigungssensors.
 *
 * AUSGABEWERTE:
 * - Pitch (Nase): Neigungswinkel um die Querachse [Grad].
 * - Roll (Wank): Neigungswinkel um die Längsachse [Grad].
 * - YawRate (Gierrate): Rotationsgeschwindigkeit um die Hochachse [Grad/s]
 *   (für PID-Lenkung).
 *
 * [cite_start]@see MPU-6000 and MPU-6050 Product Specification [cite: 27]
 */

#include <Arduino.h>
#include <Wire.h>

// ===== Konfiguration =====

/**
 * @brief I2C-Adresse des MPU-6050.
 *
 * @note Standardadresse der GY-521-Module bei AD0 = LOW.
 */
constexpr uint8_t MPU_ADDR = 0x68;

/**
 * @brief Empfindlichkeit des Beschleunigungssensors (±2g).
 *
 * @details
 * Rohwert 16384 LSB entspricht 1 g.
 */
constexpr float ACC_SENS = 16384.0f;

/**
 * @brief Empfindlichkeit des Gyroskops (±250 °/s).
 *
 * @details
 * Rohwert 131 LSB entspricht 1 °/s.
 */
constexpr float GYRO_SENS = 131.0f;

/**
 * @brief Filterkoeffizient des Komplementärfilters.
 *
 * @details
 * FILTER_ALPHA gibt den Anteil der Gyro-Integration an;
 * (1 - FILTER_ALPHA) ist der Anteil der Beschleunigungssensor-Werte.
 *
 * Typisch:
 *  - 0.95–0.99: starke Glättung, wenig Rauschen, langsame Reaktion.
 */
constexpr float FILTER_ALPHA = 0.98f; // 98% Gyro, 2% Accel

// ===== Globale Variablen =====

/// Offset (Bias) des Beschleunigungssensors in X [LSB].
float ax_offset = 0;
/// Offset (Bias) des Beschleunigungssensors in Y [LSB].
float ay_offset = 0;
/// Offset (Bias) des Beschleunigungssensors in Z [LSB].
float az_offset = 0;

/// Offset (Bias) des Gyros in X [LSB].
float gx_offset = 0;
/// Offset (Bias) des Gyros in Y [LSB].
float gy_offset = 0;
/// Offset (Bias) des Gyros in Z [LSB].
float gz_offset = 0;

/// Flag: true, sobald die Kalibrierung erfolgreich abgeschlossen wurde.
bool calib_done = false;

/// Gefilterter Pitch-Winkel (Nase) im Fahrzeug-Koordinatensystem [°].
float pitch_angle = 0.0f;

/// Gefilterter Roll-Winkel (Wank) im Fahrzeug-Koordinatensystem [°].
float roll_angle = 0.0f;

/// Zeitstempel des letzten IMU-Updates [µs], für dt-Berechnung.
unsigned long last_micros = 0;

// ===== I2C Helpers =====

/**
 * @brief Schreibt ein einzelnes Register des MPU-6050 über I2C.
 *
 * @param reg   Registeradresse im MPU-6050.
 * @param value Zu schreibender Wert.
 *
 * @note
 * Blockierender Aufruf; setzt eine zuvor initialisierte Wire-Schnittstelle
 * voraus (`Wire.begin()`).
 */
void writeReg(uint8_t reg, uint8_t value) {
    Wire.beginTransmission(MPU_ADDR);
    Wire.write(reg);
    Wire.write(value);
    Wire.endTransmission();
}

/**
 * @brief Liest zusammenhängende Rohdaten aus dem MPU-6050.
 *
 * @details
 * Startet bei Register 0x3B und liest:
 *  - Beschleunigung X, Y, Z (je 16 Bit),
 *  - Temperatur (16 Bit, wird hier ignoriert),
 *  - Gyro X, Y, Z (je 16 Bit).
 *
 * @param[out] ax Rohwert Beschleunigung X [LSB].
 * @param[out] ay Rohwert Beschleunigung Y [LSB].
 * @param[out] az Rohwert Beschleunigung Z [LSB].
 * @param[out] gx Rohwert Gyro X [LSB].
 * @param[out] gy Rohwert Gyro Y [LSB].
 * @param[out] gz Rohwert Gyro Z [LSB].
 */
void readRaw(int16_t &ax, int16_t &ay, int16_t &az, int16_t &gx, int16_t &gy,
             int16_t &gz) {
    Wire.beginTransmission(MPU_ADDR);
    Wire.write(0x3B);
    Wire.endTransmission(false);
    Wire.requestFrom((int)MPU_ADDR, 14);
    ax = (Wire.read() << 8) | Wire.read();
    ay = (Wire.read() << 8) | Wire.read();
    az = (Wire.read() << 8) | Wire.read();
    Wire.read();
    Wire.read(); // Temp ignorieren
    gx = (Wire.read() << 8) | Wire.read();
    gy = (Wire.read() << 8) | Wire.read();
    gz = (Wire.read() << 8) | Wire.read();
}

/**
 * @brief Führt eine einfache, blockierende Offsets-Kalibrierung durch.
 *
 * @details
 * - Liest @c n Messungen in Ruhelage,
 * - mittelt die Rohwerte für Beschleunigung und Gyro,
 * - setzt die globalen Offset-Variablen entsprechend.
 *
 * Besonderheit Z-Achse:
 * - Erwartet bei Überkopf-Montage etwa -1 g (≈ -ACC_SENS),
 * - @c az_offset wird daher um ACC_SENS verschoben, sodass nach Abzug
 *   ein Wert nahe 0 g resultiert.
 *
 * @pre
 *  - Rover/Sensor liegt ruhig (keine Bewegung),
 *  - Einbaulage entspricht der in der Datei beschriebenen Orientierung.
 *
 * @post
 *  - @c calib_done ist auf @c true gesetzt,
 *  - globale Offsets sind initialisiert.
 */
void calibrate() {
    Serial.println(F("Kalibrierung... (bitte ruhig halten)"));
    long sax = 0, say = 0, saz = 0, sgx = 0, sgy = 0, sgz = 0;
    const int n = 1000;

    for (int i = 0; i < n; i++) {
        int16_t ax, ay, az, gx, gy, gz;
        readRaw(ax, ay, az, gx, gy, gz);
        sax += ax;
        say += ay;
        saz += az;
        sgx += gx;
        sgy += gy;
        sgz += gz;
        delay(2);
    }
    ax_offset = sax / (float)n;
    ay_offset = say / (float)n;
    // Z-Offset: Erwartet -1g (-16384) wegen Überkopf
    az_offset = (saz / (float)n) + ACC_SENS;

    gx_offset = sgx / (float)n;
    gy_offset = sgy / (float)n;
    gz_offset = sgz / (float)n;

    calib_done = true;
    Serial.println(F("Fertig."));
}

/**
 * @brief Initialisiert serielle Schnittstelle, I2C-Bus und IMU.
 *
 * @details
 * - Startet die serielle Kommunikation (115200 Baud),
 * - initialisiert den I2C-Bus (400 kHz),
 * - weckt den MPU-6050 aus dem Sleep-Modus,
 * - führt eine Offsets-Kalibrierung durch,
 * - setzt den Startzeitpunkt für die spätere dt-Berechnung.
 *
 * @warning Während der Kalibrierung muss der Rover absolut ruhig stehen.
 */
void setup() {
    Serial.begin(115200);
    Wire.begin();
    Wire.setClock(400000);

    // MPU wecken (Power Management Register)
    writeReg(0x6B, 0x00);

    calibrate();
    last_micros = micros();
}

/**
 * @brief Hauptschleife zur kontinuierlichen Sensorfusion.
 *
 * @details
 * Zyklischer Ablauf:
 *  1. Zeitdifferenz @c dt aus @c micros() berechnen,
 *  2. Rohdaten (Accel + Gyro) einlesen,
 *  3. in physikalische Größen umrechnen (g, °/s),
 *  4. in Fahrzeug-Koordinatensystem transformieren,
 *  5. Beschleunigung in Winkel (Pitch/Roll) umrechnen,
 *  6. Komplementärfilter anwenden,
 *  7. Ausgabe der gefilterten Winkel und der Gierrate (reduzierte Rate).
 *
 * Die eigentliche Filterfrequenz ist durch die Loop-Geschwindigkeit
 * bestimmt; die Ausgabe wird auf ca. 10 Hz begrenzt, um den seriellen
 * Monitor nicht zu überlasten.
 */
void loop() {
    if (!calib_done)
        return;

    // 1. Zeitdelta berechnen (dt in Sekunden)
    unsigned long now = micros();
    float dt = (now - last_micros) / 1000000.0f;
    last_micros = now;

    // 2. Rohdaten lesen
    int16_t ax_raw, ay_raw, az_raw, gx_raw, gy_raw, gz_raw;
    readRaw(ax_raw, ay_raw, az_raw, gx_raw, gy_raw, gz_raw);

    // 3. Physikalische Werte (Chip-Frame)
    float ax_g = (ax_raw - ax_offset) / ACC_SENS;
    float ay_g = (ay_raw - ay_offset) / ACC_SENS;
    float az_g = (az_raw - az_offset) / ACC_SENS;

    float gx_dps = (gx_raw - gx_offset) / GYRO_SENS;
    float gy_dps = (gy_raw - gy_offset) / GYRO_SENS;
    float gz_dps = (gz_raw - gz_offset) / GYRO_SENS;

    // 4. Transformation -> Fahrzeug-Frame
    // Basierend auf Ihren Tests:
    // X = Fahrtrichtung, Z = Unten
    float ax_v = ax_g;
    float ay_v = ay_g;
    float az_v = -az_g; // Vektor zeigt jetzt virtuell nach oben

    // Rotationsraten für Filter (aufpassen auf Vorzeichen!)
    // Wankrate (Roll rate) = Rotation um X (Fahrtrichtung) = gx_dps
    float roll_rate = gx_dps;
    // Nickrate (Pitch rate) = Rotation um Y (Querachse) = gy_dps
    float pitch_rate = gy_dps;
    // Gierrate (Yaw rate) = Rotation um Z = -gz_dps (bestätigt durch Test 2)
    float yaw_rate = -gz_dps;

    // 5. Winkel aus Beschleunigung (Accel Only)
    // Pitch invertiert (-), damit "Nase hoch" = positiv ist (Resultat Test 1)
    float acc_pitch =
        -atan2f(-ax_v, sqrtf(ay_v * ay_v + az_v * az_v)) * 180.0f / PI;
    float acc_roll = atan2f(ay_v, az_v) * 180.0f / PI;

    // 6. Komplementärfilter (FUSION)
    // Formel: Gefiltert = alpha * (Gefiltert + Gyro*dt) + (1-alpha) * Accel
    pitch_angle = FILTER_ALPHA * (pitch_angle + pitch_rate * dt) +
                  (1.0f - FILTER_ALPHA) * acc_pitch;
    roll_angle = FILTER_ALPHA * (roll_angle + roll_rate * dt) +
                 (1.0f - FILTER_ALPHA) * acc_roll;

    // 7. Ausgabe (reduziert auf 10Hz für Lesbarkeit, Filter läuft aber schnell)
    static unsigned long last_print = 0;
    if (millis() - last_print > 100) {
        last_print = millis();

        Serial.print(F("Pitch (Nase): "));
        Serial.print(pitch_angle, 1); // Sollte stabil sein
        Serial.print(F("° | Roll (Wank): "));
        Serial.print(roll_angle, 1);
        Serial.print(F("° | YawRate: "));
        Serial.println(yaw_rate, 1);
    }
}
