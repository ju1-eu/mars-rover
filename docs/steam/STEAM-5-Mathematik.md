# Mathematics (Mathematik)

Diese Einheit behandelt das Thema: **"Algorithmen, Vektoren & Regelungstechnik"**.

---

# Lektion 5: Mathematik in Bewegung – Kinematik & Regelung

**Zielgruppe:** Mathematik-Leistungskurs / Informatik.
**Lernziel:** Anwenden von Algebra und einfacher Analysis (Integration) auf reale Probleme. Verstehen, wie abstrakte Formeln Motorgeschwindigkeiten steuern.
**Bezug zur Firmware:** `src/logic/Motion.cpp`, `DriveAssistant`.

---

## 1. Kinematik: Die Geometrie der Panzer-Steuerung

Anders als ein Auto hat der GalaxyRVR kein Lenkrad. Er nutzt **differentielle Lenkung** (Differential Drive). Um eine Kurve zu fahren, müssen sich die Räder unterschiedlich schnell drehen.

### Das mathematische Modell (Kurvenfahrt)
Die Firmware definiert eine Funktion, die einen Kurvenradius $r$ (von 0.0 bis 1.0) in Motorwerte umrechnet.

**Gegeben:**
* $v$: Zielgeschwindigkeit (PWM 0–255)
* $r$: Kurvenfaktor ($0.0 = \text{gerade}$, $1.0 = \text{drehen auf der Stelle}$)

**Die Formel aus der Dokumentation:**
Die Geschwindigkeit des kurveninneren Rades $v_\text{in}$ berechnet sich zu:
$$v_\text{in} = v \cdot (1.0 - r)$$

**Rechenbeispiel für Schüler:**
Der Roboter soll mit Halbgas ($v=100$) eine leichte Rechtskurve ($r=0.2$) fahren.
* Linkes Rad (Außen): $100$ (bleibt unverändert)
* Rechtes Rad (Innen): $100 \cdot (1.0 - 0.2) = 100 \cdot 0.8 = 80$



**Erkenntnis:**
Die Lenkung ist eine **lineare Funktion**. Schüler können den Graphen zeichnen: X-Achse = Kurvenfaktor, Y-Achse = PWM.

---

## 2. Regelungstechnik: Der P-Regler (DriveAssistant)

Der `DriveAssistant` in `src/logic/` ist ein perfektes Beispiel für angewandte Mathematik. Er soll den Rover geradeaus halten, auch wenn ein Motor mechanisch etwas schwächer ist oder der Boden uneben ist.

### Der Regelkreis
Wir nutzen einen **Proportional-Regler (P-Regler)**.

1.  **Sollwert:** Gierrate (Drehung) = $0^\circ/s$ (Wir wollen geradeaus fahren).
2.  **Istwert:** Das Gyroskop misst z.B. $+5^\circ/s$ (Der Rover driftet nach rechts ab).
3.  **Fehler ($e$):** $\text{Soll} - \text{Ist} = -5$.

### Die Regel-Gleichung
$$Korrektur = e \cdot K_p$$
*(Wobei $K_p$ der Proportionalverstärker ist)*

Wenn $K_p = 2.0$:
$$\text{Korrektur} = -5 \cdot 2.0 = -10$$

### Anwendung auf die Motoren
* $Motor_L = \text{Basis} - \text{Korrektur} = 100 - (-10) = 110$ (Beschleunigen!)
* $Motor_R = \text{Basis} + \text{Korrektur} = 100 + (-10) = 90$ (Abbremsen!)

**Diskussion "Tuning":**
Was passiert, wenn der Schüler den Faktor $K_p$ zu hoch wählt (z.B. 50)?
* *Mathematisch:* Die Korrektur wird riesig.
* *Physikalisch:* Der Roboter schießt über das Ziel hinaus, steuert gegen, schießt wieder vorbei. Er beginnt zu **oszillieren** (schwingen).

---

## 3. Signalverarbeitung: Schwellenwerte & Deadzones

Mathematik hilft uns auch, Rauschen zu filtern. Die Dokumentation erwähnt eine **Deadzone** (Totzone). Sensoren liefern selten exakt 0, sondern vielleicht 0.01 oder -0.02 (Rauschen).

**Die mathematische Funktion (Thresholding):**

$$
f(x) =
\begin{cases}
0 & \text{wenn } |x| < \text{Deadzone} \\
x & \text{wenn } |x| \ge \text{Deadzone}
\end{cases}
$$

**Warum ist das wichtig?**
Ohne diese Formel würden die Motoren im Stillstand ständig leise "brummen" oder zucken, weil der Regler versucht, das Rauschen von 0.01 auszugleichen. Die Mathematik sorgt für "Ruhe".

---

## 4. Analysis: Integration (Vom Tempo zur Strecke)

Um eine 180°-Wende (**Smart Turn**) durchzuführen, reicht die Gierrate (Geschwindigkeit) nicht. Wir brauchen den Winkel (Position).

**Das Prinzip:**
$$
\text{Winkel} = \int \text{Gierrate} \, dt
$$

In der Firmware (diskret in C++) wird das Integral zur Summe:
$$
\text{Winkel}_\text{neu} = \text{Winkel}_\text{alt} + (\text{Gyro-Wert} \cdot \Delta t)
$$

Dies verdeutlicht den Schülern die Bedeutung des Faktors **Zeit ($\Delta t$)**. Wenn der Loop nicht stabil läuft (siehe Lektion 2 "Timing"), wird $\Delta t$ ungenau und der berechnete Winkel falsch. Der Roboter dreht sich dann z.B. nur 160° statt 180°.
