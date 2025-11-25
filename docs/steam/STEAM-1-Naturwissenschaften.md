# Science (Naturwissenschaften)
Diese Einheit behandelt das Thema: **"Trägheit, Sensorik & Datenfusion"**.

---

# Lektion 1: Die Physik der Bewegung – IMU & Sensor-Fusion

**Zielgruppe:** Fortgeschrittene Schüler/Studenten (Informatik, Physik NWT).
**Lernziel:** Verstehen, wie rohe physikalische Messdaten (Beschleunigung, Drehung) erfasst, mathematisch bereinigt und für die Navigation genutzt werden.
**Bezug zur Firmware:** `src/hal/` (Hardware Abstraction Layer) und `src/logic/` (DriveAssistant).

---

## 1. Theoretische Grundlagen (Physik)

Der GalaxyRVR nutzt einen **MPU6050**, eine sogenannte **IMU (Inertial Measurement Unit)**. Um den Rover stabil zu halten oder Wendemanöver zu messen, müssen wir zwei physikalische Konzepte kombinieren, da beide Sensoren einzeln Schwächen haben.

### A. Der Beschleunigungssensor (Accelerometer)
Er misst die Beschleunigungskräfte, inklusive der Erdbeschleunigung $g$ ($9,81 m/s^2$).
* **Prinzip:** Ein mikromechanisches Masse-Feder-System. Wenn der Rover kippt, ändert sich die Komponente von $g$ auf den Achsen X, Y, Z.
* **Stärke:** Absolut stabil über lange Zeit (kein "Drift"). Er weiß immer, wo "Unten" ist.
* **Schwäche:** Extrem anfällig für Vibrationen (Motorrauschen, Bodenwellen). Jede Erschütterung verfälscht den gemessenen Winkel sofort.

### B. Das Gyroskop (Drehratensensor)
Er misst die Winkelgeschwindigkeit $\omega$ (Omega) in Grad pro Sekunde ($\deg/s$).
* **Prinzip:** Nutzung der Corioliskraft.
* **Stärke:** Sehr präzise bei schnellen Bewegungen; ignoriert Vibrationen weitgehend.
* **Schwäche:** **Drift**. Um den aktuellen Winkel zu erhalten, muss man die Rate über die Zeit integrieren (aufsummieren). Kleine Messfehler addieren sich auf – nach wenigen Sekunden "denkt" der Sensor, er dreht sich, obwohl er stillsteht.

---

## 2. Analyse der Firmware-Lösung

In der `README.md` wird beschrieben, dass die Firmware dieses physikalische Problem im **HAL (Hardware Abstraction Layer)** löst.

### Das Problem im Code
Würden wir für den `DriveAssistant` nur das Gyroskop nutzen, würde der Rover nach einer Minute Kurven fahren, weil der Sensor driftet. Nutzen wir nur den Beschleunigungssensor, "zappelt" die Lenkung bei jeder Bodenwelle.

### Die Lösung: Sensor-Fusion (Komplementärfilter)
Die Firmware implementiert einen Komplementärfilter direkt im Treiber (`src/hal/`). Dieser Filter kombiniert die physikalischen Vorteile beider Sensoren:

$$
\text{Winkel} = \alpha \cdot (\text{Winkel} + \text{Gyro} \cdot \Delta t) + (1 - \alpha) \cdot \text{Acc}
$$

* **Hochpass (Gyro):** Der Gyro-Anteil ($\alpha \approx 0.98$) dominiert kurzfristig. Er sorgt für schnelle Reaktion.
* **Tiefpass (Acc):** Der Accelerometer-Anteil ($1-\alpha \approx 0.02$) korrigiert langfristig den Drift des Gyroskops und zieht den Wert langsam zurück zur wahren Gravitationslinie.

---

## 3. Praktisches Experiment & Code-Verständnis

**Aufgabe:** Identifikation der Implementierung in der Architektur.

1.  **Hardware-Setup:**
    Die IMU (MPU6050) ist am Galaxy RVR Shield über den I2C-Bus an den Pins **A4 (SDA)** und **A5 (SCL)** angeschlossen.

2.  **Code-Analyse (Abstraktion):**
    Suchen Sie in der Datei `src/hal/ImuSensor.cpp` (hypothetischer Name basierend auf Struktur) nach der Berechnung.
    * *Frage:* Warum passiert dies in `src/hal/` und nicht in `src/main.cpp`?
    * *Antwort:* Nach dem Prinzip "Separation of Concerns". Die `main.cpp` (Application Layer) oder der `DriveAssistant` (Logic Layer) sollen sich nicht mit roher Physik herumschlagen. Sie fordern nur `getPitch()` oder `getYawRate()` an.

3.  **Verhaltens-Test (DriveAssistant):**
    Der `DriveAssistant` in `src/logic/` nutzt die **Gierrate (YawRate)** des Gyroskops.
    * **Szenario:** Heben Sie den Rover an und drehen Sie ihn in der Luft (Simulation einer Kursabweichung).
    * **Beobachtung:** Die Räder müssten sich gegenläufig drehen, um gegenzusteuern.
    * **Physik-Check:** Hier wird ein **P-Regler** verwendet: `Korrektur = Gierrate * P_Faktor`. Je schneller die Drehung (hohe physikalische Winkelgeschwindigkeit), desto stärker die Gegenreaktion der Motoren.

---

## 4. Transfer-Aufgabe für Schüler

**Szenario:**
Der Rover fährt über Kopfsteinpflaster (starke Vibration). Gleichzeitig fährt er eine langsame Kurve.

**Diskussion:**
Welcher Teil des Komplementärfilters ist in diesem Moment wichtiger?
1.  Der **Accelerometer-Teil** ist durch das Kopfsteinpflaster stark verrauscht (unbrauchbar für Momentaufnahmen).
2.  Der **Gyroskop-Teil** liefert trotz Wackeln saubere Daten über die Kurvendrehung.
3.  *Ergebnis:* Der hohe Faktor $\alpha$ (z.B. 0.98) sorgt dafür, dass der Rover trotz Vibration stabil durch die Kurve steuert, da er das "Rauschen" des Bodens (Accelerometer) kurzfristig ignoriert.

---

## Zusammenfassung der Lektion
Die Schüler lernen, dass **Roboter nicht die "Realität" sehen**, sondern nur verrauschte physikalische Daten. Erst durch Algorithmen (Sensor-Fusion) und Architektur (HAL-Kapselung) entsteht ein nutzbares Abbild der physikalischen Welt, mit dem der Roboter "intelligent" handeln kann.

