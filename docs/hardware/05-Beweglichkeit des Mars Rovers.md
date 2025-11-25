# Cornell-Notizen: Lektion 5 - Beweglichkeit (Profi-Architektur)

| **Fragen / Schlüsselbegriffe** | **Notizen & Details** |
| :--- | :--- |
| **Steuerungs-Prinzip** | \* **Differentiallenkung** (Skid-Steering): Keine Lenkachse; Steuerung erfolgt durch Geschwindigkeitsdifferenz der Seiten.<br>\* **Geradeaus:** $V_L = V_R$<br>\* **Kurve:** $V_L \neq V_R$ (z.B. ein Rad langsamer).<br>\* **Spin (Panzer-Wende):** $V_L = -V_R$ (Räder drehen gegenläufig). |
| **Hardware & Pins (HAL)** | \* **Problem:** ATmega328P hat nur an Pins 3, 5, 6, 9, 10, 11 Hardware-PWM.<br>\* **Lösung Galaxy RVR:** Nutzt Mischbetrieb.<br> \* **Links:** Pin 2 (Fwd/SoftPWM), Pin 3 (Rev/HardPWM).<br> \* **Rechts:** Pin 5 (Fwd/HardPWM), Pin 4 (Rev/SoftPWM).<br>\* **Treiber:** Kapselung in `src/hal/Motor.cpp` mittels `SoftPWM`-Lib. |
| **Software-Architektur (Layered)** | **1. HAL (Hardware Layer):**<br> \* *Aufgabe:* Rohe Pin-Steuerung (`SoftPWMSet`).<br> \* *Interface:* `HAL::Motor::setSpeed(int left, int right)` (-255 bis +255).<br>**2. Logic Layer (Motion Control):**<br> \* *Aufgabe:* Berechnet Motorwerte aus Befehlen.<br> \* *Beispiel:* `kurveRechts(speed, ratio)` $\rightarrow$ reduziert rechten Motor-Speed.<br>**3. Application Layer (Main):**<br> \* *Aufgabe:* Choreografie via State Machine (`enum class`). |
| **Zeit-Management (Non-Blocking)** | \* **Vermeidung:** Kein `delay()` in der `loop()`, da dies Sensoren blockieren würde.<br>\* **Ersatz:** Polling der Systemzeit.<br> `if (millis() - lastChange > duration) { ... }`<br>\* **Vorteil:** Der Roboter bleibt "ansprechbar" (z.B. für Not-Aus). |
| **Mathematik der Bewegung** | \* **Kurvenfahrt:** $V_{innen} = V_{soll} \cdot (1.0 - \text{ratio})$<br> \* *Ratio 0.0:* Geradeaus.<br> \* *Ratio 0.5:* Sanfte Kurve.<br> \* *Ratio 1.0:* Drehung um das innere Rad (steht still). |

-----

### Zusammenfassung

Lektion 5 transformiert die einfache Motoransteuerung in ein intelligentes **Motion-Control-System**. Durch die **Differentiallenkung** werden komplexe Manöver möglich. In der professionellen Umsetzung wird die Hardware-Komplexität (gemischte PWM-Pins, SoftPWM) vollständig in der **HAL (Hardware Abstraction Layer)** versteckt. Die Anwendungslogik nutzt stattdessen abstrakte Befehle (`move`, `turn`) und steuert den Ablauf über einen **nicht-blockierenden Zustandsautomaten** in C++17, was Multitasking (z.B. gleichzeitiges Messen und Fahren) ermöglicht.
