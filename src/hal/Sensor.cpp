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

// Für Ultraschall (persistenter Speicher)
float g_lastDistanceCm = 999.0f;

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

float getUltrasonicDistance() { return g_lastDistanceCm; }

// Veraltete Test-Funktion (kann für Diagnose bleiben)
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
bool irLeftBlocked() { return digitalRead(Pin::IR_Left) == LOW; }
bool irRightBlocked() { return digitalRead(Pin::IR_Right) == LOW; }

void irTest() {
    // ... (Code wie gehabt) ...
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

bool isBatteryCharging() { return false; }

// ====================== IMU (Kalibrierung & Logik) ======================

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
float getPitch() { return g_pitch; }
float getRoll() { return g_roll; }
float getYawRate() { return g_lastGyro_dps_rover.z; }

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
