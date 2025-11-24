/**
 * @file    Sensor.cpp
 * @brief   Implementierung der Sensor-HAL (IMU, Ultraschall, IR, Batterie).
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
constexpr float BATTERY_MIN_V = 6.6f;
constexpr float BATTERY_MAX_V = 8.4f;
constexpr float ADC_REF_V = 5.246f;
constexpr float BATTERY_DIVIDER = 2.0f;

// ======================= IMU (MPU6050) =======================
constexpr uint8_t MPU_ADDR = 0x68;
constexpr uint8_t REG_PWR_MGMT1 = 0x6B;
constexpr uint8_t REG_ACCEL_XOUT_H = 0x3B;
constexpr uint8_t REG_WHO_AM_I = 0x75;

constexpr float ACC_LSB_PER_G = 16384.0f;
constexpr float GYRO_LSB_PER_DPS = 131.0f;

// Filter-Parameter
constexpr float FILTER_ALPHA = 0.98f; // 98% Gyro, 2% Accel

bool imuAvailable = false;
bool imuCalibrated = false;

struct Vec3f {
    float x;
    float y;
    float z;
};

// Offsets
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
void mpuWriteReg(uint8_t reg, uint8_t value) {
    Wire.beginTransmission(MPU_ADDR);
    Wire.write(reg);
    Wire.write(value);
    Wire.endTransmission();
}

void mpuReadBytes(uint8_t startReg, uint8_t *buf, uint8_t len) {
    Wire.beginTransmission(MPU_ADDR);
    Wire.write(startReg);
    Wire.endTransmission(false);
    Wire.requestFrom(static_cast<int>(MPU_ADDR), static_cast<int>(len));
    for (uint8_t i = 0; i < len && Wire.available(); ++i) {
        buf[i] = Wire.read();
    }
}

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

// --- In src/hal/Sensor.cpp (im namespace anonymous ganz oben) ersetzen ---

bool imuInit() {
    Wire.begin();
    Wire.setClock(100000); // Sicherheits-Geschwindigkeit (100kHz)
    // --- NEU: TIMEOUT HINZUFÜGEN ---
    // Dies verhindert den "Freeze", wenn die Motoren stören.
    // 3000 us = 3 ms Timeout, true = Reset des Busses bei Fehler
    Wire.setWireTimeout(3000, true);
    delay(100); // Kurz warten nach Power-Up

    // 1. Verbindung prüfen
    Wire.beginTransmission(MPU_ADDR);
    if (Wire.endTransmission() != 0) {
        return false; // Keine Antwort (ACK) vom Sensor
    }

    // 2. HARD-RESET des Sensors (Register 0x6B, Bit 7 setzen)
    // Das behebt "verwirrte" Zustände des Chips
    mpuWriteReg(REG_PWR_MGMT1, 0x80);
    delay(100); // Warten bis Reset fertig

    // 3. Aufwecken (Sleep-Modus beenden)
    mpuWriteReg(REG_PWR_MGMT1, 0x00);
    delay(100); // Warten bis Oszillator stabil

    // 4. WHO_AM_I Check zur Bestätigung
    Wire.beginTransmission(MPU_ADDR);
    Wire.write(REG_WHO_AM_I);
    if (Wire.endTransmission(false) != 0)
        return false;
    if (Wire.requestFrom(static_cast<int>(MPU_ADDR), 1) != 1)
        return false;

    if (Wire.read() != 0x68)
        return false;

    // Alles sauber initialisiert
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

void init() {
    pinMode(Pin::IR_Left, INPUT);
    pinMode(Pin::IR_Right, INPUT);
    // Ultraschall Pins werden dynamisch gesetzt
    // Batterie Pin ist Analog Input (automatisch)

    if (imuInit()) {
        imuAvailable = true;
        Serial.println(F("IMU: MPU6050 bereit (Upside-Down Config)."));
    } else {
        imuAvailable = false;
        Serial.println(F("IMU: FEHLER - MPU6050 nicht gefunden."));
    }
}

// ====================== ULTRASCHALL ======================
void ultrasonicTest() {
    static unsigned long lastMeasure = 0;
    static float lastDistance = -1.0f;

    unsigned long now = millis();
    // Messung nur alle 2000 ms
    if (now - lastMeasure < 2000UL)
        return;
    lastMeasure = now;

    pinMode(Pin::Ultrasonic_Trig, OUTPUT);
    digitalWrite(Pin::Ultrasonic_Trig, LOW);
    delayMicroseconds(2);
    digitalWrite(Pin::Ultrasonic_Trig, HIGH);
    delayMicroseconds(10);
    digitalWrite(Pin::Ultrasonic_Trig, LOW);

    pinMode(Pin::Ultrasonic_Echo, INPUT);
    unsigned long duration =
        pulseIn(Pin::Ultrasonic_Echo, HIGH, 18000); // Max ~3m

    if (duration == 0)
        return; // Timeout

    float distance = duration * 0.01715f; // Kalibrierter Wert

    // Filterung unrealistischer Werte
    if (distance < 2.0f || distance > 300.0f)
        return;

    // Nur bei Änderung ausgeben
    if (lastDistance < 0.0f || fabs(distance - lastDistance) > 1.0f) {
        Serial.print(F("Ultrasonic: "));
        Serial.print(distance);
        Serial.println(F(" cm"));
        lastDistance = distance;
    }
}

// ====================== IR SENSOREN ======================
bool irLeftBlocked() { return digitalRead(Pin::IR_Left) == LOW; }
bool irRightBlocked() { return digitalRead(Pin::IR_Right) == LOW; }

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
float getBatteryVoltage() {
    int adcValue = analogRead(Pin::Battery);
    float adcVoltage = static_cast<float>(adcValue) * ADC_REF_V / 1023.0f;
    float batteryVoltage = adcVoltage * BATTERY_DIVIDER;
    return static_cast<float>(static_cast<int>(batteryVoltage * 10.0f + 0.5f)) /
           10.0f;
}

uint8_t getBatteryPercentage() {
    float u = getBatteryVoltage();
    if (u <= BATTERY_MIN_V)
        return 0;
    if (u >= BATTERY_MAX_V)
        return 100;
    return static_cast<uint8_t>(
        (u - BATTERY_MIN_V) / (BATTERY_MAX_V - BATTERY_MIN_V) * 100.0f + 0.5f);
}

bool isBatteryCharging() {
    // Einfache Dummy-Implementierung oder deine Logik hier einfügen
    return false;
}

// ====================== IMU (Kalibrierung & Logik) ======================

// --- In src/hal/Sensor.cpp (namespace HAL::Sensor) ersetzen ---

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

        // VISUELLER FORTSCHRITT: Alle 50 Samples einen Punkt drucken
        if (i % 50 == 0) {
            Serial.print(F("."));
        }
        delay(3); // Etwas mehr Zeit geben
    }
    Serial.println(); // Neue Zeile nach den Punkten

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

void imuUpdate() {
    if (!imuAvailable || !imuCalibrated)
        return;

    // 1. Zeitberechnung (dt)
    unsigned long nowMicros = micros();
    float dt = (nowMicros - g_lastFilterMicros) / 1000000.0f;
    g_lastFilterMicros = nowMicros;

    // 2. Rohdaten lesen
    int16_t ax_raw, ay_raw, az_raw, gx_raw, gy_raw, gz_raw;
    readImuRaw(ax_raw, ay_raw, az_raw, gx_raw, gy_raw, gz_raw);

    // 3. Umrechnen (Sensor-Frame) & Offsets
    float ax_g = (ax_raw - g_accOffsetLSB.x) / ACC_LSB_PER_G;
    float ay_g = (ay_raw - g_accOffsetLSB.y) / ACC_LSB_PER_G;
    float az_g = (az_raw - g_accOffsetLSB.z) / ACC_LSB_PER_G;

    float gx_dps = (gx_raw - g_gyroOffsetLSB.x) / GYRO_LSB_PER_DPS;
    float gy_dps = (gy_raw - g_gyroOffsetLSB.y) / GYRO_LSB_PER_DPS;
    float gz_dps = (gz_raw - g_gyroOffsetLSB.z) / GYRO_LSB_PER_DPS;

    // 4. Transformation -> Rover-Frame (X=Vorne, Z=Oben invertiert)
    g_lastAcc_g_rover.x = ax_g;  // Vorne
    g_lastAcc_g_rover.y = ay_g;  // Links
    g_lastAcc_g_rover.z = -az_g; // Oben (Invertiert)

    g_lastGyro_dps_rover.x = gx_dps;  // Roll Rate
    g_lastGyro_dps_rover.y = gy_dps;  // Pitch Rate
    g_lastGyro_dps_rover.z = -gz_dps; // Yaw Rate (Invertiert)

    // 5. Winkel berechnen (Accel-Basis)
    // Pitch: Nase hoch = positiv (daher erstes Minus bei atan2)
    float acc_pitch =
        -atan2f(-g_lastAcc_g_rover.x,
                sqrtf(g_lastAcc_g_rover.y * g_lastAcc_g_rover.y +
                      g_lastAcc_g_rover.z * g_lastAcc_g_rover.z)) *
        180.0f / PI;
    float acc_roll =
        atan2f(g_lastAcc_g_rover.y, g_lastAcc_g_rover.z) * 180.0f / PI;

    // 6. Komplementärfilter
    g_pitch = FILTER_ALPHA * (g_pitch + g_lastGyro_dps_rover.y * dt) +
              (1.0f - FILTER_ALPHA) * acc_pitch;
    g_roll = FILTER_ALPHA * (g_roll + g_lastGyro_dps_rover.x * dt) +
             (1.0f - FILTER_ALPHA) * acc_roll;
}

// Getter-Implementierungen
float getPitch() { return g_pitch; }
float getRoll() { return g_roll; }
float getYawRate() { return g_lastGyro_dps_rover.z; }

// Test-Ausgabe (nutzt jetzt Update + Print)
void imuTest() {
    // Im Diagnose-Modus rufen wir Update hier explizit auf, falls es im Loop
    // fehlt
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

// Veraltete Funktion als Wrapper behalten oder leer lassen, falls Header sie
// verlangt
void imuPrintAngles() {
    Serial.print(F("Winkel: P="));
    Serial.print(getPitch());
    Serial.print(F(" R="));
    Serial.println(getRoll());
}

} // namespace HAL::Sensor
