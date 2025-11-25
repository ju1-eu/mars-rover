/**
 * @file       DriveAssistant.cpp
 * @brief      Logik-Modul für assistierte Geradeausfahrt und Kippschutz.
 *
 * @details
 * Dieses Modul implementiert einen Regelkreis (Closed Loop Control), der den
 * Rover softwareseitig stabilisiert. Es kompensiert mechanische Imperfektionen
 * und äußere Störungen.
 *
 * **Architektur & Regelungstechnik:**
 * - **Feed-Forward (Steuerung):** Der statische `BIAS_TRIM` gleicht permanente
 * Hardware-Asymmetrien (z.B. unterschiedliche Reibung im Getriebe) aus,
 * bevor der Regler eingreift.
 * - **Feedback (Regelung):** Der PID-Regler (hier als reiner P-Regler
 * konfiguriert) reagiert auf dynamische Störungen (Teppichkanten, Schlupf).
 *
 * @author     Jan Unger
 * @version    1.4.0 (Final Phase 1)
 * @date       2025-11-25
 */

#include "logic/DriveAssistant.h"
#include "hal/Motor.h"
#include "hal/Sensor.h"
#include "logic/PidController.h"
#include <Arduino.h>

namespace Logic::DriveAssistant {

// ==========================================================================
// 1. REGLER-KONFIGURATION (TUNING)
// ==========================================================================

/**
 * @brief   PID-Koeffizienten für die Gierraten-Stabilisierung.
 *
 * @details
 * Konfiguration für "Soft-Response" um Oszillationen (Zittern) zu vermeiden.
 * - $K_p = 0.6$: Sanfte proportionale Gegenreaktion.
 * - $K_i = 0.0$: Deaktiviert, um "Integral Windup" (Drift) zu verhindern.
 * - $K_d = 0.0$: Deaktiviert, um Rauschverstärkung ("Derivative Kick") bei
 * Vibrationen zu meiden.
 */
static Logic::PidController yawPid(0.6f, 0.0f, 0.0f);

/**
 * @brief   Schwellenwert für Signal-Rausch-Unterdrückung.
 * @unit    Grad pro Sekunde (°/s)
 *
 * @details
 * Gyroskope rauschen minimal, auch im Stillstand. Werte unterhalb dieses Limits
 * werden als "0.0" interpretiert, um unnötige Regel-Eingriffe zu verhindern.
 *
 * @hardware MPU6050 Rauschverhalten
 */
constexpr float DEADZONE_DPS = 0.3f;

/**
 * @brief   Statischer Geradeauslauf-Ausgleich (Feed-Forward Bias).
 *
 * @details
 * Dient der Kompensation mechanischer Asymmetrien (z.B. linker Motor läuft
 * schwergängiger als rechter).
 *
 * **Wirkungsweise:**
 * - Positiver Wert: Reduziert Links, erhöht Rechts -> Steuert nach LINKS.
 * - Negativer Wert: Erhöht Links, reduziert Rechts -> Steuert nach RECHTS.
 *
 * @note    Wurde empirisch auf 5 ermittelt (Rover zog leicht nach rechts).
 */
constexpr int BIAS_TRIM = 5;

/**
 * @brief   Sicherheits-Grenzwert für die Neigung.
 * @safety  Verhindert Überschläge bei Rampenfahrten.
 */
constexpr float MAX_PITCH_DEG = 45.0f;

// ==========================================================================
// 2. INTERNE ZUSTANDSVARIABLEN
// ==========================================================================

static unsigned long lastTime = 0; // Zeitstempel für Delta-t Berechnung

// ==========================================================================
// 3. IMPLEMENTIERUNG
// ==========================================================================

void init() {
    yawPid.reset();
    lastTime = millis();
}

bool update(uint8_t baseSpeed) {
    unsigned long now = millis();

    // A. Zeitbasis (Delta t) berechnen
    // Nötig für korrekte physikalische Einheiten im Regler (falls I/D genutzt).
    float dt = (now - lastTime) / 1000.0f;
    lastTime = now;

    // Schutz gegen Zeitsprünge (z.B. bei Debugging-Pausen)
    if (dt > 0.1f || dt <= 0.0f) {
        dt = 0.0f;
    }

    // B. Sensor-Fusion Abfrage
    float pitch = HAL::Sensor::getPitch();
    float yawRate = HAL::Sensor::getYawRate();

    // C. Sicherheits-Check (Safety Layer)
    if (abs(pitch) > MAX_PITCH_DEG) {
        // Rückgabe false signalisiert der Main-Loop: "Unsicherer Zustand!"
        return false;
    }

    // D. Regelungs-Logik
    float correction = 0.0f;

    // Deadzone-Filterung: Nur regeln, wenn Bewegung > Rauschen
    if (abs(yawRate) > DEADZONE_DPS) {
        // Sollwert: 0.0°/s (Keine Drehung)
        // Istwert:  yawRate (Aktuelle Drehung)
        correction = yawPid.compute(0.0f, yawRate, dt);
    } else {
        // WICHTIG: Reset im Ruhezustand
        // Verhindert, dass der Regler "Geisterfehler" aus der Vergangenheit
        // speichert, wenn der Rover eigentlich geradeaus fährt.
        yawPid.reset();
        correction = 0.0f;
    }

    // E. Aktuator-Mixing (Output Stage)
    // Die finale Geschwindigkeit setzt sich additiv zusammen:
    // $$ v_{out} = v_{basis} \pm (u_{pid} + u_{bias}) $$

    // Bias-Logik: Ein positiver BIAS_TRIM (5) bedeutet, wir wollen nach LINKS
    // korrigieren. Dafür muss Rechts schneller (+) und Links langsamer (-)
    // werden.
    int speedL =
        static_cast<int>(baseSpeed) - static_cast<int>(correction) - BIAS_TRIM;
    int speedR =
        static_cast<int>(baseSpeed) + static_cast<int>(correction) + BIAS_TRIM;

    // Hardware-Schutz: Werte auf zulässigen PWM-Bereich begrenzen
    speedL = constrain(speedL, -255, 255);
    speedR = constrain(speedR, -255, 255);

    HAL::Motor::setSpeed(speedL, speedR);

    // F. Telemetrie (Debug)
    // Ausgabe auf 5Hz limitiert, um den Bus nicht zu fluten
    static unsigned long lastDebug = 0;
    if (now - lastDebug > 200) {
        lastDebug = now;
        Serial.print(F("Gier:"));
        Serial.print(yawRate, 2);
        Serial.print(F(" | Bias:"));
        Serial.print(BIAS_TRIM);
        Serial.print(F(" -> L:"));
        Serial.print(speedL);
        Serial.print(F(" R:"));
        Serial.println(speedR);
    }

    return true; // System stabil
}

} // namespace Logic::DriveAssistant
