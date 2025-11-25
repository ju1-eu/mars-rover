/**
 * @file       Sensor.cpp
 * @brief      Implementierung der Sensor-HAL (IMU, Ultraschall, IR, Batterie).
 *
 * @details
 * Dieses Modul ist die "Wahrnehmung" des Rovers. Es abstrahiert die rohen
 * elektrischen Signale (Spannungspegel, I2C-Register, Pulsdauern) in
 * physikalische Größen (Zentimeter, Grad, Volt).
 *
 * Architektur-Einordnung & Didaktik:
 * - **Hardware-Abstraktion:** Versteckt Register-Magie (MPU6050) und
 * Timing-Tricks (Ultraschall) vor der Logik-Schicht.
 * - **Sensor-Fusion:** Implementiert einen Komplementärfilter, um die
 * Schwächen einzelner Sensoren (Gyro-Drift, Accel-Rauschen) auszugleichen.
 *
 * @dependency Wire (I2C)
 */

#include "hal/Sensor.h"
#include "Config.h"
#include "Pins.h"
#include <Arduino.h>
#include <Wire.h>

// ==========================================================================
// INTERNE KONSTANTEN & HELPER (Anonymes Namespace)
// ==========================================================================
namespace {

// --- Batterie-Einstellungen (2S Li-Ion) ---
constexpr float BATTERY_MIN_V = 6.6f; // 3.3V pro Zelle (Leer)
constexpr float BATTERY_MAX_V = 8.4f; // 4.2V pro Zelle (Voll)
constexpr float ADC_REF_V = 5.0f;     // Referenzspannung Arduino (ggf. messen!)
constexpr float BATTERY_DIVIDER = 2.0f; // Spannungsteiler 1:1

// --- IMU (MPU6050) Register & Adressen ---
constexpr uint8_t MPU_ADDR = 0x68;
constexpr uint8_t REG_PWR_MGMT1 = 0x6B;
constexpr uint8_t REG_ACCEL_XOUT = 0x3B;
constexpr uint8_t REG_WHO_AM_I = 0x75;

// --- Umrechnungsfaktoren (Sensitivity) ---
// Einstellung: Accel +/- 2g, Gyro +/- 250deg/s
constexpr float ACC_LSB_PER_G = 16384.0f;
constexpr float GYRO_LSB_PER_DPS = 131.0f;

// --- Komplementärfilter (Sensor Fusion) ---
// Alpha bestimmt das Vertrauen in das Gyroskop.
// 0.98 = 98% Gyro (kurzfristig präzise), 2% Accel (langfristig stabil,
// korrigiert Drift).
constexpr float FILTER_ALPHA = 0.98f;

// --- Interne Zustandsvariablen ---
bool imuAvailable = false;
bool imuCalibrated = false;
float g_lastDistanceCm = 999.0f; // Startwert "Weit weg"

// Hilfsstruktur für 3D-Vektoren
struct Vec3f {
    float x;
    float y;
    float z;
};

// Kalibrierungs-Offsets (Nullpunkt-Verschiebung)
Vec3f g_accOffsetLSB{0, 0, 0};
Vec3f g_gyroOffsetLSB{0, 0, 0};

// Gefilterte Orientierung (Output)
float g_pitch = 0.0f; // Nase hoch/runter
float g_roll = 0.0f;  // Seitliches Kippen
unsigned long g_lastFilterMicros = 0;

// Rohwerte im Rover-Koordinatensystem (für Getter)
Vec3f g_lastGyro_dps{0, 0, 0};

// ----------------------------------------------------------------------
// Low-Level I2C Helper
// ----------------------------------------------------------------------

void mpuWriteReg(uint8_t reg, uint8_t value) {
    Wire.beginTransmission(MPU_ADDR);
    Wire.write(reg);
    Wire.write(value);
    Wire.endTransmission();
}

/**
 * @brief Liest alle 6 Achsen + Temp in einem Rutsch (Burst Read).
 * @details Effizienter als einzelne Registerabfragen.
 */
void readImuRaw(int16_t &ax, int16_t &ay, int16_t &az, int16_t &gx, int16_t &gy,
                int16_t &gz) {
    Wire.beginTransmission(MPU_ADDR);
    Wire.write(REG_ACCEL_XOUT);
    Wire.endTransmission(false); // Restart condition

    // 14 Bytes anfordern: Ax, Ay, Az, Temp, Gx, Gy, Gz (je 2 Byte)
    Wire.requestFrom((int)MPU_ADDR, 14);

    if (Wire.available() == 14) {
        ax = (Wire.read() << 8) | Wire.read();
        ay = (Wire.read() << 8) | Wire.read();
        az = (Wire.read() << 8) | Wire.read();
        Wire.read();
        Wire.read(); // Temperatur ignorieren
        gx = (Wire.read() << 8) | Wire.read();
        gy = (Wire.read() << 8) | Wire.read();
        gz = (Wire.read() << 8) | Wire.read();
    }
}

/**
 * @brief Startet den MPU6050.
 * @return true wenn erfolgreich verbunden.
 */
bool imuInitHard() {
    Wire.begin();
    Wire.setClock(400000); // Fast Mode (400kHz)

    // Timeout setzen, damit der Rover nicht einfriert, falls ein Kabel wackelt
    Wire.setWireTimeout(3000 /* us */, true /* reset_on_timeout */);

    // 1. Verbindungstest
    Wire.beginTransmission(MPU_ADDR);
    if (Wire.endTransmission() != 0)
        return false;

    // 2. Wake Up (Sleep Mode deaktivieren)
    mpuWriteReg(REG_PWR_MGMT1, 0x00);
    delay(10);

    // 3. ID Check
    Wire.beginTransmission(MPU_ADDR);
    Wire.write(REG_WHO_AM_I);
    Wire.endTransmission(false);
    Wire.requestFrom((int)MPU_ADDR, 1);

    if (Wire.available() && Wire.read() == 0x68) {
        return true;
    }
    return false;
}

} // namespace

// ==========================================================================
// PUBLIC API IMPLEMENTIERUNG
// ==========================================================================
namespace HAL::Sensor {

/**
 * @brief Initialisiert alle Sensoren.
 */
void init() {
    // IR Sensoren
    pinMode(Pin::IR_Left, INPUT);
    pinMode(Pin::IR_Right, INPUT);

    // IMU
    if (imuInitHard()) {
        imuAvailable = true;
        Serial.println(F("IMU: MPU6050 verbunden."));
    } else {
        imuAvailable = false;
        Serial.println(F("IMU: FEHLER! Keine Verbindung (Check wiring)."));
    }

    // Ultraschall Pins werden dynamisch in update() gesetzt
}

// ----------------------------------------------------------------------
// ULTRASCHALL (Single Pin Trig/Echo Logic)
// ----------------------------------------------------------------------

void ultrasonicUpdate() {
    static unsigned long lastMeas = 0;
    unsigned long now = millis();

    // Limitierung auf 10Hz (alle 100ms), um Echos abklingen zu lassen
    if (now - lastMeas < 100)
        return;
    lastMeas = now;

    // 1. Trigger Senden (10us High-Puls)
    pinMode(Pin::Ultrasonic_Trig, OUTPUT);
    digitalWrite(Pin::Ultrasonic_Trig, LOW);
    delayMicroseconds(2);
    digitalWrite(Pin::Ultrasonic_Trig, HIGH);
    delayMicroseconds(10);
    digitalWrite(Pin::Ultrasonic_Trig, LOW);

    // 2. Echo Empfangen (Pin auf Input schalten)
    pinMode(Pin::Ultrasonic_Echo, INPUT);

    // Timeout: 18ms entspricht ca. 3m Reichweite (Schall ~340m/s)
    unsigned long duration = pulseIn(Pin::Ultrasonic_Echo, HIGH, 18000);

    if (duration == 0) {
        // Timeout -> Kein Hindernis
        g_lastDistanceCm = 999.0f;
    } else {
        // Umrechnung: Weg = Zeit * Schallgeschw. / 2 (Hin & Zurück)
        // 343 m/s = 0.0343 cm/us -> /2 = 0.01715
        float dist = duration * 0.01715f;

        // Einfacher Plausibilitätsfilter (Gegen Rauschen < 2cm)
        if (dist > 2.0f && dist < 400.0f) {
            g_lastDistanceCm = dist;
        }
    }
}

float getUltrasonicDistance() { return g_lastDistanceCm; }

void ultrasonicTest() {
    ultrasonicUpdate();
    static float lastLog = 0;
    // Logge nur bei Änderung > 2cm um Serial nicht zu fluten
    if (abs(g_lastDistanceCm - lastLog) > 2.0f) {
        Serial.print(F("[Test] US Distanz: "));
        Serial.print(g_lastDistanceCm);
        Serial.println(F(" cm"));
        lastLog = g_lastDistanceCm;
    }
}

// ----------------------------------------------------------------------
// BATTERIE
// ----------------------------------------------------------------------

float getBatteryVoltage() {
    // 10-Bit ADC Wert (0..1023)
    int raw = analogRead(Pin::Battery);

    // Umrechnung in Spannung am Pin
    float vPin = (raw / 1023.0f) * ADC_REF_V;

    // Umrechnung auf Batteriespannung (Spannungsteiler korrigieren)
    return vPin * BATTERY_DIVIDER;
}

uint8_t getBatteryPercentage() {
    float v = getBatteryVoltage();

    // Clamping 0..100%
    if (v <= BATTERY_MIN_V)
        return 0;
    if (v >= BATTERY_MAX_V)
        return 100;

    // Lineare Interpolation (für Li-Ion grob okay)
    return (uint8_t)((v - BATTERY_MIN_V) / (BATTERY_MAX_V - BATTERY_MIN_V) *
                     100);
}

// ----------------------------------------------------------------------
// IMU (Kalibrierung & Fusion)
// ----------------------------------------------------------------------

void imuCalibrate(uint16_t samples) {
    if (!imuAvailable)
        return;

    Serial.println(F("IMU: Kalibrierung... (Rover stillhalten!)"));

    long buffAx = 0, buffAy = 0, buffAz = 0;
    long buffGx = 0, buffGy = 0, buffGz = 0;

    for (uint16_t i = 0; i < samples; i++) {
        int16_t ax, ay, az, gx, gy, gz;
        readImuRaw(ax, ay, az, gx, gy, gz);
        buffAx += ax;
        buffAy += ay;
        buffAz += az;
        buffGx += gx;
        buffGy += gy;
        buffGz += gz;
        delay(3); // Kleine Pause zwischen Samples
    }

    // Mittelwerte berechnen
    g_accOffsetLSB.x = buffAx / samples;
    g_accOffsetLSB.y = buffAy / samples;

    // WICHTIG: Z-Achse Kalibrierung ("Upside Down" Montage)
    // Wenn der Sensor kopfüber montiert ist, zeigt Z nach UNTEN.
    // Die Schwerkraft zieht aber auch nach UNTEN.
    // Der Sensor misst auf Z also ca. -1g (-16384 LSB).
    // Der Offset ist die Abweichung vom Erwartungswert.
    // Wir wollen, dass (Messung - Offset) = -1g ergibt.
    // Also: Offset = Messung - (-16384) = Messung + 16384.
    g_accOffsetLSB.z = (buffAz / samples) + ACC_LSB_PER_G;

    g_gyroOffsetLSB.x = buffGx / samples;
    g_gyroOffsetLSB.y = buffGy / samples;
    g_gyroOffsetLSB.z = buffGz / samples;

    imuCalibrated = true;
    g_lastFilterMicros = micros();
    Serial.println(F("IMU: Kalibrierung abgeschlossen."));
}

void imuUpdate() {
    if (!imuAvailable || !imuCalibrated)
        return;

    // 1. Zeitbasis (dt) für Integration berechnen
    unsigned long now = micros();
    float dt = (now - g_lastFilterMicros) / 1000000.0f;
    g_lastFilterMicros = now;

    // 2. Rohdaten lesen
    int16_t axr, ayr, azr, gxr, gyr, gzr;
    readImuRaw(axr, ayr, azr, gxr, gyr, gzr);

    // 3. Physikalische Einheiten & Offsets anwenden
    // Beschleunigung in g
    float ax = (axr - g_accOffsetLSB.x) / ACC_LSB_PER_G;
    float ay = (ayr - g_accOffsetLSB.y) / ACC_LSB_PER_G;
    float az = (azr - g_accOffsetLSB.z) / ACC_LSB_PER_G;

    // Gierraten in °/s (deg per sec)
    float gx = (gxr - g_gyroOffsetLSB.x) / GYRO_LSB_PER_DPS;
    float gy = (gyr - g_gyroOffsetLSB.y) / GYRO_LSB_PER_DPS;
    float gz = (gzr - g_gyroOffsetLSB.z) / GYRO_LSB_PER_DPS;

    // 4. Koordinaten-Transformation (Rover Frame)
    // MPU6050 ist "Kopfüber" montiert.
    // Sensor X = Rover X (Vorne)
    // Sensor Y = Rover Y (Rechts, wenn von oben gesehen? Nein, Y ist Links bei
    // MPU Standard) Sensor Z = Zeigt in den Boden. Wir mappen alles auf ein
    // Standard "Rechte-Hand-System" (X=Vorne, Y=Links, Z=Oben).

    float roverAx = ax;
    float roverAy = ay;
    float roverAz = -az; // Invertieren, damit +Z nach oben zeigt

    // Gyro Raten müssen passend rotiert werden
    float roverGx = gx;
    float roverGy = gy;
    float roverGz = -gz;

    // Speichern für Yaw-Regelung (DriveAssistant braucht das)
    g_lastGyro_dps.z = roverGz;

    // 5. Winkel aus Beschleunigung berechnen (Trigonometrie)
    // Nur gültig, wenn der Rover steht oder gleichförmig fährt (keine
    // Fliehkräfte).
    float accPitch =
        -(atan2(-roverAx, sqrt(roverAy * roverAy + roverAz * roverAz)) * 180.0 /
          PI);
    float accRoll = (atan2(roverAy, roverAz) * 180.0 / PI);

    // 6. SENSOR FUSION (Komplementärfilter)
    // Idee: Wir integrieren das Gyro (Winkel = Winkel_alt + Rate * Zeit).
    // Aber das driftet weg. Deshalb ziehen wir den Winkel langsam (zu 2%)
    // in Richtung des Accelerometer-Winkels (Schwerkraft-Vektor).

    g_pitch = FILTER_ALPHA * (g_pitch + roverGy * dt) +
              (1.0f - FILTER_ALPHA) * accPitch;
    g_roll = FILTER_ALPHA * (g_roll + roverGx * dt) +
             (1.0f - FILTER_ALPHA) * accRoll;
}

float getPitch() { return g_pitch; }
float getRoll() { return g_roll; }
float getYawRate() { return g_lastGyro_dps.z; }

void imuTest() {
    imuUpdate();
    static unsigned long lastP = 0;
    if (millis() - lastP > 200) {
        lastP = millis();
        Serial.print(F("[IMU] P:"));
        Serial.print(g_pitch, 1);
        Serial.print(F(" R:"));
        Serial.print(g_roll, 1);
        Serial.print(F(" YawRate:"));
        Serial.println(getYawRate(), 1);
    }
}

} // namespace HAL::Sensor
