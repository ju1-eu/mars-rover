# Hardware

Um einen Roboter zu bauen, der dem "Profi-Anspruch" (ROS 2, Navigation, Mapping) gerecht wird, brauchst du Hardware, die nicht nur Spielzeug ist, sondern verlässliche Daten liefert.

### 1. Welcher Raspberry Pi 5? (4GB vs. 8GB)

**Meine Empfehlung: Nimm die 8GB Version.**

* **Warum?**
    * **Kompilieren:** In der ROS 2-Entwicklung musst du oft Software direkt auf dem Roboter kompilieren ("bauen"). Das frisst RAM. Bei 4GB stürzt der Pi oft ab oder lagert auf die langsame SD-Karte aus (Swapping), was ewig dauert.
    * **Zukunftssicherheit (KI):** Wenn du später eine Kamera anschließt und Objekterkennung (YOLO, OpenCV) machen willst, sind 4GB sofort voll. Das OS und ROS 2 brauchen alleine schon ca. 1-1.5 GB für einen flüssigen Betrieb.
    * **Preisunterschied:** Der Aufpreis ist im Vergleich zum Gesamtprojekt gering, der Frustfaktor bei zu wenig RAM aber hoch.

**Zusatz-Tipp für Pi 5:**
* Du brauchst zwingend den **Active Cooler** (den Lüfter). Der Pi 5 wird sehr heiß.
* Besorge dir eine schnelle **MicroSD-Karte (A2 Klasse)** oder, noch besser, eine **NVMe SSD** mit einem HAT. SD-Karten sterben bei ROS-Logging (ständiges Schreiben von Daten) schnell.

---

### 2. Der Rover & Motoren (Das Herzstück)

Vermeide billige Plastik-Chassis mit den gelben "TT-Motoren" (die für 2 Euro). Die sind laut, haben zu viel Spiel im Getriebe und deren Encoder sind ungenau.

#### A. Die Motoren (Encoder sind Pflicht!)
Du suchst nach **DC Getriebemotoren mit Metallgetriebe und Hall-Effekt-Encodern**.
* **Spezifikation:** 12V Motoren sind Industriestandard (stärker als 6V).
* **Encoder-Typ:** "Hall-Sensor Quadrature Encoder". Sie haben meist 6 Kabel:
    * 2x Strom für Motor (+/-)
    * 2x Strom für Encoder-Chip (5V/GND)
    * 2x Signal (Phase A, Phase B) -> Diese gehen an den ESP32.
* **Empfehlung:** Such nach **"JGA25-370"** Motoren mit Encoder. Das ist der Sweetspot aus Preis/Leistung für Hobby-Profis.

#### B. Das Chassis
* **Typ:** Für den Anfang ist ein **Differential Drive** (2 angetriebene Räder + 1 Stützrad) am einfachsten zu programmieren und zu regeln. 4-Rad-Antrieb (Skid Steer) ist in der Odometrie ungenauer (Räder rutschen beim Drehen).
* **Kauf-Tipp:** Es gibt Alu-Chassis-Kits auf Amazon/AliExpress (oft "Smart Car Chassis Aluminium" genannt), die schon Löcher für Motoren und Sensoren haben. Achte darauf, dass genug Platz für den Akku und den Pi ist.

---

### 3. Die Elektronik-Brücke (Motor-Treiber)

Der ESP32 kann die Motoren nicht direkt mit Strom versorgen (er würde durchbrennen). Du brauchst einen **Motortreiber**.

* **Nicht kaufen:** Den alten **L298N** (der große mit dem riesigen Kühlkörper). Er ist veraltete Technik, frisst viel Spannung und wird heiß.
* **Kaufen:**
    * **TB6612FNG:** Klein, effizient, gut für kleinere Motoren.
    * **DRV8833:** Ebenfalls sehr gut und sicher.
    * **Cytron MD10C:** Wenn du starke Motoren hast. Das ist Profi-Klasse, aber man braucht einen pro Motor.

---

### 4. Stromversorgung (Der häufigste Fehler)

Ein Pi 5 und Motoren an derselben Powerbank? **Nein.** Wenn die Motoren anlaufen, bricht die Spannung ein und der Pi stürzt ab.

* **Der Akku:** Ein **LiPo-Akku (2S oder 3S)** aus dem Modellbau.
    * *2S (7.4V):* Gut für einfache 6V-9V Motoren.
    * *3S (11.1V):* Besser für 12V Motoren.
* **Spannungswandler (BEC / Buck Converter):**
    * Du brauchst einen starken **DC-DC Step-Down Converter** (mindestens 5A Output), der die 11.1V des Akkus auf **stabile 5.1V** für den Raspberry Pi 5 runterregelt. Der Pi 5 ist extrem zickig bei Unterspannung!

---

### 5. Sensorik für Mapping (SLAM)

Damit der Roboter weiß, wo er ist:

1.  **Lidar (Laserscanner):** Das Auge für die Karte.
    * *Einstieg:* **RPLIDAR A1M8** (ca. 80-100€). Der Klassiker. Dreht sich 360 Grad.
    * *Alternative:* **LD19** oder **LDLIDAR** (günstiger, oft um 60€, funktionieren auch gut mit ROS 2).
2.  **IMU (Gyroskop/Beschleunigung):** Unverzichtbar! Encoder zählen Radumdrehungen, aber wenn die Räder durchdrehen, "denkt" der Roboter, er fährt. Die IMU merkt, ob er sich wirklich dreht.
    * *Empfehlung:* **MPU6050** oder **BNO055** (teurer, aber hat eigenen Prozessor für bessere Daten). Ein MPU6050 für 3€ reicht für den Anfang.

### Zusammenfassung Einkaufsliste

1.  **Computer:** Raspberry Pi 5 (8GB) + Active Cooler + Gute 64GB SD-Karte.
2.  **Controller:** ESP32 DevKit V1 (gerne 2-3 Stück kaufen, falls einer kaputt geht).
3.  **Antrieb:** Alu-Chassis + 2x JGA25-370 Motoren (12V, mit Encoder).
4.  **Treiber:** 1x TB6612FNG (kann 2 Motoren steuern).
5.  **Power:** 3S LiPo Akku + 1x starker Step-Down Converter (12V -> 5V 5A USB-C für Pi).
6.  **Sensoren:** RPLIDAR A1 + MPU6050.
