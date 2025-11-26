#include "logic/Odometry.h"
#include <Arduino.h> // Für PI, cos, sin

namespace Logic {

Odometry::Odometry() : _x(0.0f), _y(0.0f) {}

void Odometry::reset() {
    _x = 0.0f;
    _y = 0.0f;
}

void Odometry::update(float yawDeg, float dt, float voltage, bool isMoving) {
    // 1. Wenn Motoren aus sind, keine Berechnung (verhindert Drift durch
    // Rauschen)
    if (!isMoving) {
        return;
    }

    // 2. Spannungskompensation
    // Ist der Akku leerer (z.B. 7.2V), fahren wir langsamer als 27.39 cm/s.
    // Formel: v_real = v_ref * (u_aktuell / u_ref)
    float speedFactor = voltage / REF_VOLTAGE;
    float currentSpeed = REF_SPEED_CM_S * speedFactor;

    // 3. Wegstrecke in diesem Zeitschritt (s = v * t)
    float distanceStep = currentSpeed * dt;

    // 4. Winkel umrechnen (Grad -> Radiant für sin/cos)
    // 0 Grad = Osten (X+), 90 Grad = Norden (Y+) in Standard-Mathe
    // Rover-Logik: 0 Grad = Vorne (X+).
    float yawRad = yawDeg * (PI / 180.0f);

    // 5. Neue Position berechnen (Vektoraddition)
    _x += distanceStep * cos(yawRad);
    _y += distanceStep * sin(yawRad);
}

} // namespace Logic
