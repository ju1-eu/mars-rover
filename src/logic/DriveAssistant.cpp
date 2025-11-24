#include "logic/DriveAssistant.h"
#include "hal/Motor.h"
#include "hal/Sensor.h"
#include <Arduino.h>

namespace Logic::DriveAssistant {

constexpr float KP_YAW = 2.0f; //Sanftes Gegensteuern
constexpr float DEADZONE_DPS = 3.0f;//Ignoriert kleines Rauschen besser
constexpr float MAX_PITCH_DEG = 45.0f; // 90° für Test aufgebockt - Räder frei!

void init() {}

bool update(uint8_t baseSpeed) {
    float pitch = HAL::Sensor::getPitch();
    float yawRate = HAL::Sensor::getYawRate();

    // 1. Kipp-Schutz
    if (abs(pitch) > MAX_PITCH_DEG) {
        return false;
    }

    // 2. Drift berechnen
    float drift = -yawRate;
    if (abs(drift) < DEADZONE_DPS)
        drift = 0.0f;

    // 3. Regelung
    float correction = drift * KP_YAW;
    // Wir tauschen die Vorzeichen bei correction!
    int speedL = (int)baseSpeed - (int)correction;
    int speedR = (int)baseSpeed + (int)correction;

    speedL = constrain(speedL, -255, 255);
    speedR = constrain(speedR, -255, 255);

    HAL::Motor::setSpeed(speedL, speedR);

    // 4. AUSGABE (Visualisierung)
    static unsigned long lastDebug = 0;
    if (millis() - lastDebug > 250) { // 4x pro Sekunde
        lastDebug = millis();

        Serial.print(F("DA: Gier="));
        Serial.print(yawRate, 1);
        Serial.print(F("°/s "));

        if (drift == 0.0f) {
            Serial.print(F("[GERADEAUS]      "));
        } else if (correction > 0) {
            // Correction > 0 bedeutet: Linker Motor schneller -> Rechtskurve
            // Das tun wir, wenn Drift negativ war (Rover zog nach Links)
            Serial.print(F("[LENKE RECHTS ->]"));
        } else {
            Serial.print(F("[<- LENKE LINKS] "));
        }

        Serial.print(F(" | Motor L:"));
        Serial.print(speedL);
        Serial.print(F(" R:"));
        Serial.println(speedR);
    }

    return true;
}
} // namespace Logic::DriveAssistant
