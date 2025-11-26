# Phase 2: Interaktion – Schritt 1: Die "Augen" einstellen

Unser erstes Ziel ist der **Wall Follower** (Wandverfolgung). Der Rover soll parallel zu einer Wand fahren. Dafür nutzen wir die **IR-Hindernissensoren** an der Front (Links/Rechts).

**Das Problem:**
Diese Sensoren sind *digital*. Sie liefern keine Entfernung in cm (z. B. "15 cm"), sondern nur "Ja/Nein" (`blocked` / `free`).
Damit wir einer Wand folgen können, muss der Schaltpunkt (die Distanz, ab der der Sensor "Ja" sagt) genau stimmen.

**Deine Aufgabe (Hardware-Setup):**

1.  **Roadmap aktualisieren:**
    Da wir auf einem neuen Branch sind, setzen wir das Signal auf "Arbeit begonnen".

      * Öffne `docs/steam/Roadmap.md`.
      * Ändere den Status von Phase 2 auf `🚧 In Progress`.
      * Committe das direkt:
        ```bash
        git add docs/steam/Roadmap.md
        git commit -m "docs: start phase 2 interaction workflow"
        ```

2.  **IR-Sensoren kalibrieren (Schraubenzieher nötig):**
    Wir müssen die Potentiometer auf den blauen IR-Modulen so drehen, dass sie bei ca. **10-15 cm Abstand** zur Wand auslösen.

      * Lade diesen minimalen Test-Sketch in deine `main.cpp` (temporär), um die Werte zu sehen:

<!-- end list -->

```cpp
#include "hal/Sensor.h"
#include <Arduino.h>

void setup() {
    Serial.begin(115200);
    HAL::Sensor::init();
}

void loop() {
    bool left = HAL::Sensor::irLeftBlocked();
    bool right = HAL::Sensor::irRightBlocked();

    Serial.print("IR Links: ");
    Serial.print(left ? "WAND" : "Frei");
    Serial.print(" | IR Rechts: ");
    Serial.println(right ? "WAND" : "Frei");

    delay(100);
}
```

**Durchführung:**

1.  Halte deine Hand oder einen Karton ca. **15 cm** vor den linken Sensor.
2.  Drehe am kleinen Potentiometer auf dem Modul, bis die LED auf dem Modul (und die Ausgabe "WAND") gerade so anspringt.
3.  Wiederhole das für rechts.

**Warum tun wir das?**
Für den "Wall Follower" nutzen wir einen **Zweipunkt-Regler (Bang-Bang Controller)**:

  * Sensor sieht Wand $\rightarrow$ Zu nah\! $\rightarrow$ Lenke leicht weg.
  * Sensor sieht *keine* Wand $\rightarrow$ Zu weit weg\! $\rightarrow$ Lenke leicht hin.

Damit das funktioniert, muss der "Schaltpunkt" präzise sein.

Sag Bescheid, wenn die Sensoren kalibriert sind, dann schreiben wir die `WallFollower`-Logikklasse\!
