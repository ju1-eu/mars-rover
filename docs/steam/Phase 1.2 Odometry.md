# Phase 1: Präzision & Orientierung (Math & Physics)

Hinweis: Magnetfeldsensor

Ja, deine Intuition ist absolut richtig: **Du musst die Zeit für eine abgemessene Strecke messen.**

Da der Motor keinen Encoder hat (Open Loop für Geschwindigkeit), weiß die Software nicht, wie schnell „PWM 100“ in *cm/s* wirklich ist. Das müssen wir empirisch ermitteln (Kalibrierung).

Der **Magnetfeldsensor** (Kompass) ist für die Odometrie *hilfreich*, aber nicht zwingend für die Wegstrecke ($v$), sondern für den Winkel ($\theta$). Er verhindert, dass der Winkel über lange Zeit wegdriftet (was das Gyroskop alleine tun würde). Für kurze Strecken reicht das Gyroskop aus Phase 1.


-----

### Schritt 1: Die Kalibrierung (Das "Experiment")

Bevor wir coden, müssen wir die Hardware vermessen.

1.  **Aufbau:** Klebe eine Start- und eine Ziellinie auf den Boden (Abstand: z.B. **200 cm**).
2.  **Durchführung:**
      * Lade den Akku voll (wichtig\!).
      * Lasse den Rover mit deiner Standard-Geschwindigkeit (`CRUISE_SPEED = 100`) geradeaus fahren.
      * Stoppe die Zeit ($t$) vom Start bis zum Ziel mit einer Stoppuhr.
3.  **Berechnung:**
    $$v_{\text{cm/s}} = \frac{\text{Strecke (cm)}}{\text{Zeit (s)}}$$

Berechnung der Geschwindigkeit ($v$)

1.  **Durchschnittszeit ($\bar{t}$):**
    $$\frac{7,25 + 7,38 + 7,28}{3} = 7,303 \text{ Sekunden}$$

2.  **Geschwindigkeit ($v$):**
    $$v = \frac{\text{Weg}}{\text{Zeit}} = \frac{200 \text{ cm}}{7,303 \text{ s}} \approx \mathbf{27,39 \text{ cm/s}}$$

3.  **Spannungs-Referenz:**
    Dieser Wert gilt exakt bei **8,08 V**.
    Da DC-Motoren bei sinkender Spannung langsamer drehen, nutzen wir diesen Wert als Referenz für die Kompensation.


-----


Das ist der perfekte nächste Schritt. Dieser Test ist die **"Wahrheitsprobe"** für unsere mathematischen Annahmen aus Phase 1.2.

Wir schreiben die `main.cpp` so um, dass der Rover **automatisch stoppt**, sobald er *glaubt*, 200 cm gefahren zu sein.

**Der Test-Ablauf:**

1.  Rover an die Startlinie stellen.
2.  Reset drücken.
3.  Rover fährt los (PID-geregelt geradeaus).
4.  Rover stoppt (wenn Odometrie sagt: "200cm erreicht").
5.  **Du misst nach:** Ist er wirklich bei 200 cm stehengeblieben?
      * Wenn er bei **190 cm** steht: Er "denkt", er ist schneller als er ist.
      * Wenn er bei **210 cm** steht: Er "denkt", er ist langsamer als er ist.


So interpretierst du das Ergebnis

1.  Der Rover fährt los und stoppt.
2.  Auf dem Serial Monitor steht: `Gemessene Strecke (Intern): 200.xx cm`.
3.  **Du misst am Boden** vom Startpunkt bis zur Mitte der Räder.

**Das Ergebnis:**

  * **Fall A: Er steht bei 200 cm (+/- 5cm).**
    Perfekt\! Deine Kalibrierung (`27.39f` in `Odometry.h`) stimmt exakt.
  * **Fall B: Er steht bei 180 cm (Zu kurz gefahren).**
    Der Rover *glaubt* er sei 200cm gefahren, war aber in Wirklichkeit langsamer.
      * *Diagnose:* Der Wert `REF_SPEED_CM_S` ist **zu hoch**.
      * *Korrektur:* Neuer Wert = $27.39 \times \frac{180}{200}$.
  * **Fall C: Er steht bei 220 cm (Zu weit gefahren).**
    Der Rover *glaubt* er sei erst 200cm gefahren, war aber schneller.
      * *Diagnose:* Der Wert `REF_SPEED_CM_S` ist **zu niedrig**.
      * *Korrektur:* Neuer Wert = $27.39 \times \frac{220}{200}$.


Das ist Präzisionsarbeit\! Wir nähern uns dem perfekten Wert an (Konvergenz).

**Die Analyse:**

  * **Software dachte:** "Ich bin 200 cm gefahren." (Stopp bei 200).
  * **Realität war:** Er ist **193 cm** gefahren (7 cm zu kurz).
  * **Diagnose:** Die Software zählt die cm immer noch etwas zu schnell hoch. Sie "glaubt", der Rover sei schneller, als er physikalisch ist, und schaltet die Motoren deshalb einen Tick zu früh ab. Wir müssen den Referenzwert also leicht **senken**.

### Die Berechnung (Feintuning)

$$V_{neu} = V_{alt} \cdot \frac{\text{Tatsächliche Strecke}}{\text{Software Strecke}}$$

Einsetzen deiner Werte:
$$V_{neu} = 31.50 \cdot \frac{193}{200}$$
$$V_{neu} = 31.50 \cdot 0.965 \approx \mathbf{30.40}$$

### Anzupassende Datei: `src/logic/Odometry.h`

Ändere den Wert `REF_SPEED_CM_S` auf **30.40f**.

```cpp
// ...
private:
    float _x;
    float _y;

    // --- KALIBRIERUNGSDATEN (Feintuning Runde 2) ---
    // War 31.50f -> zu schnell (7cm zu kurz gefahren).
    // Jetzt: 30.40f
    static constexpr float REF_SPEED_CM_S = 30.40f;

    static constexpr float REF_VOLTAGE    = 8.08f;
};
// ...
```

### Nächster Schritt

1.  Wert ändern.
2.  Hochladen.
3.  Noch einmal testen.

Jetzt solltest du im Bereich von ± 1-2 cm landen. Das ist für "Dead Reckoning" (ohne Encoder\!) ein hervorragender Wert. Wenn das passt, können wir die Test-Schleife in der `main.cpp` rauswerfen und die echte Logik (Missionen) einbauen.

---

## 1.2 Koppelnavigation (Dead Reckoning)

Der Rover soll seine Position $(x, y)$ im Raum schätzen. Da wir keine Encoder an den Rädern haben, nutzen wir ein physikalisches Modell basierend auf Zeit und Geschwindigkeit (aus PWM geschätzt) sowie dem Gyroskop-Winkel $\theta$.

* **Mathematik (in jedem Zeitschritt $\Delta t$):**
    $$x_{neu} = x_{alt} + v \cdot \cos(\theta) \cdot \Delta t$$
    $$y_{neu} = y_{alt} + v \cdot \sin(\theta) \cdot \Delta t$$
* **Architektur:** Neue Klasse `Odometry` in `src/logic/`.
* **Feature:** Der Rover kann angewiesen werden: "Fahre 1 Meter vor und komm zurück zum Startpunkt $(0,0)$".
