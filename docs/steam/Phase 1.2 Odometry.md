# Phase 1: Präzision & Orientierung (Math & Physics)

## 1.2 Koppelnavigation (Dead Reckoning)

Der Rover soll seine Position $(x, y)$ im Raum schätzen. Da wir keine Encoder an den Rädern haben, nutzen wir ein physikalisches Modell basierend auf Zeit und Geschwindigkeit (aus PWM geschätzt) sowie dem Gyroskop-Winkel $\theta$.

* **Mathematik (in jedem Zeitschritt $\Delta t$):**
    $$x_{neu} = x_{alt} + v \cdot \cos(\theta) \cdot \Delta t$$
    $$y_{neu} = y_{alt} + v \cdot \sin(\theta) \cdot \Delta t$$
* **Architektur:** Neue Klasse `Odometry` in `src/logic/`.
* **Feature:** Der Rover kann angewiesen werden: "Fahre 1 Meter vor und komm zurück zum Startpunkt $(0,0)$".
