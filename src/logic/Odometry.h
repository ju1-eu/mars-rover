#pragma once

namespace Logic {

class Odometry {
  public:
    Odometry();

    /**
     * @brief Aktualisiert die Position basierend auf Geschwindigkeit und Kurs.
     * * @param yawDeg      Aktueller absoluter Winkel in Grad (0 =
     * Startrichtung).
     * @param dt          Vergangene Zeit seit letztem Aufruf in Sekunden.
     * @param voltage     Aktuelle Batteriespannung zur
     * Geschwindigkeits-Korrektur.
     * @param isMoving    Flag: Fährt der Rover gerade? (false = Stillstand).
     */
    void update(float yawDeg, float dt, float voltage, bool isMoving);

    // Getter für die Koordinaten
    float getX() const { return _x; }
    float getY() const { return _y; }

    // Setzt alles auf Start (0,0)
    void reset();

  private:
    float _x; // X-Position in cm
    float _y; // Y-Position in cm

    // -- -KALIBRIERUNGSDATEN(Feintuning Runde 2)-- -
    // War 31.50f -> zu schnell (7cm zu kurz gefahren).
    // Jetzt: 30.40f
    static constexpr float REF_SPEED_CM_S = 30.40f;

    // Spannung lassen wir gleich, da sie beim Test ähnlich war
    static constexpr float REF_VOLTAGE = 8.08f;
};

} // namespace Logic
