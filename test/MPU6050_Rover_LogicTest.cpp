/**
 * @file    MPU6050_Rover_LogicTest.cpp
 * @brief   Trockentest der Steuerungslogik (Kippschutz & Spurhalte-Assistent).
 *
 * @details
 * ZWECK:
 * Dieses Skript validiert die "Entscheidungsfindung" des Rovers, bevor echte
 * Motoren angeschlossen werden. Es simuliert die Motorreaktionen basierend auf
 * realen [cite_start]Sensordaten des GY-521 Moduls[cite: 3].
 *
 * HARDWARE-KONFIGURATION (ZWINGEND):
 * - [cite_start]Sensor: MPU-6050 (GY-521)[cite: 3, 4].
 * - Montage-Position: ÜBERKOPF (Chip zeigt zum Boden), X-Pfeil in
 *   Fahrtrichtung.
 * - [cite_start]Schnittstelle: I2C (SCL/SDA)[cite: 79, 84].
 *
 * FUNKTIONSWEISE & LOGIK:
 * 1. Transformation:
 *    Wandelt Sensordaten vom "Chip-Frame" (Überkopf) in das "Fahrzeug-Frame"
 *    um (Z-Achse und Gierrate invertiert).
 * 2. Sensor-Fusion:
 *    Nutzt einen Komplementärfilter (98% Gyro, 2% Accel), um stabile Pitch-
 *    und Roll-Winkel zu berechnen.
 * 3. Sicherheits-Logik (Safety):
 *    - Überwacht Pitch (Nase) UND Roll (Wank).
 *    - Bei Winkel > MAX_TILT wird ein NOT-AUS simuliert (Motoren = 0).
 * 4. Lenk-Logik (Steering):
 *    - Ein P-Regler nutzt das Gyroskop, um Drehungen (Gieren) entgegenzuwirken.
 *    - Differentielle Lenkung: Erhöht/Verringert virtuelle
 *      Motorgeschwindigkeiten.
 *
 * AUSGABE (Serial Monitor):
 * Zeigt den Sicherheitsstatus, die Neigung und die berechneten PWM-Werte
 * für linken/rechten Motor zur Überprüfung der Lenkrichtung.
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
 * @brief Empfindlichkeit des Beschleunigungssensors (±2 g).
 *
 * @details Rohwert 16384 LSB entspricht 1 g.
 */
constexpr float ACC_SENS = 16384.0f;

/**
 * @brief Empfindlichkeit des Gyroskops (±250 °/s).
 *
 * @details Rohwert 131 LSB entspricht 1 °/s.
 */
constexpr float GYRO_SENS = 131.0f;

/**
 * @brief Filterkoeffizient des Komplementärfilters.
 *
 * @details
 * FILTER_ALPHA gibt den Anteil der Gyro-Integration an;
 * (1 - FILTER_ALPHA) ist der Anteil der Beschleunigungssensor-Werte.
 */
constexpr float FILTER_ALPHA = 0.98f;

// --- Logik-Parameter ---

/**
 * @brief Maximal zulässiger Neigungswinkel, bevor ein Not-Aus ausgelöst wird.
 *
 * @details
 * Wird sowohl auf Pitch als auch auf Roll angewendet.
 * Bei Überschreitung wird die Motorleistung auf 0 gesetzt.
 */
constexpr float MAX_TILT = 45.0f;

/**
 * @brief Proportional-Verstärkung des Lenkreglers.
 *
 * @details
 * STEER_KP bestimmt, wie stark der Rover auf eine gemessene Gierrate
 * (YawRate) reagiert. Höhere Werte führen zu aggressiverem Gegensteuern.
 */
constexpr float STEER_KP = 2.0f;

/**
 * @brief Basis-PWM für die (simulierte) Motoransteuerung.
 *
 * @details
 * Wert im Bereich 0–255. Dient als Grundgeschwindigkeit; der Regler
 * addiert/subtrahiert eine Korrektur zur Differenzlenkung.
 */
constexpr int BASE_SPEED = 150;

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
    Wire.read();
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
    writeReg(0x6B, 0x00); // Wake up
    calibrate();
    last_micros = micros();
}

/**
 * @brief Hauptschleife: Sensorfusion, Sicherheitslogik und Lenk-Simulation.
 *
 * @details
 * Zyklischer Ablauf:
 *  1. dt aus @c micros() bestimmen,
 *  2. Rohdaten (Accel + Gyro) einlesen,
 *  3. in physikalische Größen umrechnen (g, °/s),
 *  4. in Fahrzeug-Koordinatensystem transformieren,
 *  5. Pitch- und Roll-Winkel aus Beschleunigung berechnen,
 *  6. Komplementärfilter anwenden,
 *  7. Kippschutz und Lenklogik berechnen,
 *  8. simulierte Motor-PWM-Werte ausgeben.
 *
 * Die Loop ist für einen Trockentest gedacht: Motoren sind nicht
 * angeschlossen, das Verhalten wird ausschließlich als Text auf dem
 * seriellen Monitor visualisiert.
 */
void loop() {
    if (!calib_done)
        return;

    // 1. Zeitdifferenz (dt) ermitteln
    unsigned long now = micros();
    float dt = (now - last_micros) / 1000000.0f;
    last_micros = now;

    // 2. Rohdaten einlesen
    int16_t ax_raw, ay_raw, az_raw, gx_raw, gy_raw, gz_raw;
    readRaw(ax_raw, ay_raw, az_raw, gx_raw, gy_raw, gz_raw);

    // 3. Umrechnung in physikalische Größen (Chip-Frame)
    float ax_g = (ax_raw - ax_offset) / ACC_SENS;
    float ay_g = (ay_raw - ay_offset) / ACC_SENS;
    float az_g = (az_raw - az_offset) / ACC_SENS;
    float gx_dps = (gx_raw - gx_offset) / GYRO_SENS;
    float gy_dps = (gy_raw - gy_offset) / GYRO_SENS;
    float gz_dps = (gz_raw - gz_offset) / GYRO_SENS;

    // 4. Transformation -> Fahrzeug-Frame
    float ax_v = ax_g;
    float ay_v = ay_g;
    float az_v = -az_g;

    // Rotationsraten im Fahrzeug-Frame
    float roll_rate = gx_dps;  // Rotation um X (Fahrtrichtung)
    float pitch_rate = gy_dps; // Rotation um Y (Querachse)
    float yaw_rate = -gz_dps;  // Rotation um Z, Linksdrehung = positiv

    // 5. Winkel aus Beschleunigung (Accel only)
    float acc_pitch =
        -atan2f(-ax_v, sqrtf(ay_v * ay_v + az_v * az_v)) * 180.0f / PI;
    float acc_roll = atan2f(ay_v, az_v) * 180.0f / PI;

    // 6. Komplementärfilter
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

        // Differentielle Motoren-Simulation
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
