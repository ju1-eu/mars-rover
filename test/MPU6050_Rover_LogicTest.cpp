/**
 * @file    MPU6050_Rover_LogicTest.cpp
 * @brief   Trockentest der Steuerungslogik (Kippschutz & Spurhalte-Assistent).
 *
 * ZWECK:
 * Dieses Skript validiert die "Entscheidungsfindung" des Rovers, bevor echte
 * Motoren angeschlossen werden. Es simuliert die Motorreaktionen basierend auf
 * realen [cite_start]Sensordaten des GY-521 Moduls[cite: 3].
 *
 * HARDWARE-KONFIGURATION (ZWINGEND):
 * - [cite_start]Sensor: MPU-6050 (GY-521)[cite: 3, 4].
 * - Montage-Position: ÜBERKOPF (Chip zeigt zum Boden), X-Pfeil in
 * Fahrtrichtung.
 * - [cite_start]Schnittstelle: I2C (SCL/SDA)[cite: 79, 84].
 *
 * FUNKTIONSWEISE & LOGIK:
 * 1. Transformation: Wandelt Sensordaten vom "Chip-Frame" (Überkopf) in das
 * "Fahrzeug-Frame" um (Z-Achse und Gierrate invertiert).
 * 2. Sensor-Fusion: Nutzt einen Komplementärfilter (98% Gyro, 2% Accel), um
 * stabile Pitch- und Roll-Winkel zu berechnen.
 * 3. Sicherheits-Logik (Safety):
 * - Überwacht Pitch (Nase) UND Roll (Wank).
 * - Bei Winkel > 45° wird ein NOT-AUS simuliert (Motoren = 0).
 * 4. Lenk-Logik (Steering):
 * - Ein P-Regler nutzt das Gyroskop, um Drehungen (Gieren) entgegenzuwirken.
 * - Differentielle Lenkung: Erhöht/Verringert virtuelle Motorgeschwindigkeiten.
 *
 * AUSGABE (Serial Monitor):
 * Zeigt den Sicherheitsstatus, die Neigung und die berechneten PWM-Werte
 * für Linken/Rechten Motor zur Überprüfung der Lenkrichtung.
 */

#include <Arduino.h>
#include <Wire.h>

// ===== Konfiguration =====
constexpr uint8_t MPU_ADDR = 0x68;
constexpr float ACC_SENS = 16384.0f;
constexpr float GYRO_SENS = 131.0f;
constexpr float FILTER_ALPHA = 0.98f;

// --- NEU: Logik-Parameter ---
constexpr float MAX_TILT = 45.0f; // Ab 45 Grad -> Not-Aus
constexpr float STEER_KP = 2.0f;  // Lenk-Aggressivität
constexpr int BASE_SPEED = 150;   // Simulierter Basis-Speed (0-255)

// ===== Globale Variablen =====
float ax_offset = 0, ay_offset = 0, az_offset = 0;
float gx_offset = 0, gy_offset = 0, gz_offset = 0;
bool calib_done = false;

// Filter-Status
float pitch_angle = 0.0f;
float roll_angle = 0.0f;
unsigned long last_micros = 0;

// I2C Helpers
void writeReg(uint8_t reg, uint8_t value) {
    Wire.beginTransmission(MPU_ADDR);
    Wire.write(reg);
    Wire.write(value);
    Wire.endTransmission();
}

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
    Wire.read();
    gx = (Wire.read() << 8) | Wire.read();
    gy = (Wire.read() << 8) | Wire.read();
    gz = (Wire.read() << 8) | Wire.read();
}

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
    az_offset = (saz / (float)n) + ACC_SENS; // Z invertiert (Upside-Down)
    gx_offset = sgx / (float)n;
    gy_offset = sgy / (float)n;
    gz_offset = sgz / (float)n;
    calib_done = true;
    Serial.println(F("Fertig."));
}

void setup() {
    Serial.begin(115200);
    Wire.begin();
    Wire.setClock(400000);
    writeReg(0x6B, 0x00); // Wake up
    calibrate();
    last_micros = micros();
}

void loop() {
    if (!calib_done)
        return;

    unsigned long now = micros();
    float dt = (now - last_micros) / 1000000.0f;
    last_micros = now;

    int16_t ax_raw, ay_raw, az_raw, gx_raw, gy_raw, gz_raw;
    readRaw(ax_raw, ay_raw, az_raw, gx_raw, gy_raw, gz_raw);

    float ax_g = (ax_raw - ax_offset) / ACC_SENS;
    float ay_g = (ay_raw - ay_offset) / ACC_SENS;
    float az_g = (az_raw - az_offset) / ACC_SENS;
    float gx_dps = (gx_raw - gx_offset) / GYRO_SENS;
    float gy_dps = (gy_raw - gy_offset) / GYRO_SENS;
    float gz_dps = (gz_raw - gz_offset) / GYRO_SENS;

    // Transformation -> Fahrzeug-Frame
    float ax_v = ax_g;
    float ay_v = ay_g;
    float az_v = -az_g;

    // Rotation -> Fahrzeug-Frame
    float roll_rate = gx_dps;
    float pitch_rate = gy_dps;
    float yaw_rate = -gz_dps; // Linksdrehung = Positiv

    // Filter
    float acc_pitch =
        -atan2f(-ax_v, sqrtf(ay_v * ay_v + az_v * az_v)) * 180.0f / PI;
    float acc_roll = atan2f(ay_v, az_v) * 180.0f / PI;

    pitch_angle = FILTER_ALPHA * (pitch_angle + pitch_rate * dt) +
                  (1.0f - FILTER_ALPHA) * acc_pitch;
    roll_angle = FILTER_ALPHA * (roll_angle + roll_rate * dt) +
                 (1.0f - FILTER_ALPHA) * acc_roll;

    // ===== AUSGABE & LOGIK-SIMULATION =====
    static unsigned long last_print = 0;
    if (millis() - last_print > 200) { // 5x pro Sekunde
        last_print = millis();

        // 1. SICHERHEIT (Safety Logic)
        bool danger = false;
        if (abs(pitch_angle) > MAX_TILT || abs(roll_angle) > MAX_TILT) {
            danger = true;
            Serial.print(F("🚨 STATUS: NOT-AUS! (Zu steil) | "));
        } else {
            Serial.print(F("✅ STATUS: OK (Fahren)      | "));
        }

        // 2. LENKUNG (Steering Logic)
        // P-Regler: Fehler = Soll (0) - Ist (YawRate)
        float error = 0 - yaw_rate;
        float correction = error * STEER_KP;

        // Motoren-Simulation
        // Wenn correction negativ ist (Linksdrehung), muss Links schneller
        // werden
        int speed_L = BASE_SPEED - correction;
        int speed_R = BASE_SPEED + correction;

        // Begrenzung (nur für Simulation)
        speed_L = constrain(speed_L, 0, 255);
        speed_R = constrain(speed_R, 0, 255);

        if (danger) {
            speed_L = 0;
            speed_R = 0;
        }

        // Anzeige der Werte
        Serial.print(F("Pitch: "));
        Serial.print(pitch_angle, 0);
        Serial.print(F("° | YawRate: "));
        Serial.print(yaw_rate, 0);

        Serial.print(F(" => MOTOREN: L="));
        Serial.print(speed_L);
        Serial.print(F(" / R="));
        Serial.println(speed_R);
    }
}
