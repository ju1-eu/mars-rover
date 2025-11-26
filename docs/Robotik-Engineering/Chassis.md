# Chassis

### 1. Der "Gold-Standard" für Maker: WaveShare & Yahboom
Diese Hersteller bieten Chassis an, die explizit für Jetson Nano oder Raspberry Pi gedacht sind. Sie sind meist aus eloxiertem Aluminium und haben vernünftige Motoren vorinstalliert.

* **Empfehlung:** **WaveShare General Mobile Robot Chassis** (oder ähnliche Varianten).
* **Was du bekommst:**
    * Robustes Aluminium-Gehäuse (Plattenbauweise).
    * Oft **JGA25-370 Motoren** (12V) bereits montiert.
    * Hochwertige Räder (besserer Grip als billiges Plastik).
    * Manchmal sogar schon eine Montageplatte für Lidar.
* **Vorteil:** Die Löcher für den Pi und Motortreiber sind oft schon passend vorgebohrt.

### 2. Die "No-Name" Alu-Kits (Amazon / AliExpress)
Hier bekommst du das beste Preis-Leistungs-Verhältnis, musst aber genau hinschauen. Viele sehen gleich aus, haben aber unterschiedliche Motoren.

* **Suchbegriff:** `"Robot Chassis Aluminum Encoder JGA25"` oder `"Smart Car Chassis 12V Encoder"`.
* **Woran du das richtige Kit erkennst (Checkliste für die Produktfotos):**
    1.  **Der Motor:** Er darf nicht gelb sein (Plastik). Er muss ein silberner Zylinder sein (Metall).
    2.  **Die Kabel:** Zähle die Kabel am Motor!
        * 2 Kabel = Kein Encoder (Finger weg!)
        * **6 Kabel** = Mit Encoder (Kaufen!)
    3.  **Das Material:** Es muss Aluminium sein (meist schwarz oder blau eloxiert), kein Acryl (das bricht leicht und vibriert).

### 3. Konkrete Produkt-Beispiele (Typen)

Damit du weißt, wonach du suchen musst:

#### A. Das 2WD Differential Drive Chassis (Rund oder Rechteckig)
Ideal für den Einstieg und SLAM.
* **Aufbau:** 2 angetriebene Räder in der Mitte + 1 oder 2 Stützräder (Caster Wheels).
* **Warum gut?** Kann auf der Stelle drehen (Zero Radius Turn). Sehr einfach mathematisch zu modellieren für ROS.
* **Kosten:** ca. 40–70 €.

#### B. Das 4WD "Skid Steer" Chassis
Sieht aus wie ein kleiner Geländewagen.
* **Aufbau:** 4 Motoren, alle Räder angetrieben. Lenkt wie ein Panzer (linke Seite vorwärts, rechte rückwärts).
* **Warnung:** Für sauberes Mapping (SLAM) ist das schwieriger. Die Räder "rubbeln" über den Boden beim Drehen. Das verwirrt die Odometrie. Du brauchst zwingend eine IMU (Gyroskop), um das auszugleichen. Für den Anfang eher **A** (2WD) nehmen.

#### C. Mecanum-Wheels Chassis
Räder mit kleinen Rollen drauf, kann seitwärts fahren.
* **Urteil:** Cool für Demos, aber für präzise Navigation und Mapping oft ein Albtraum, weil die Räder schnell durchrutschen. Bleib lieber bei Gummireifen.

---

## Unterschied zwischen einem "Panzer" (Differential Drive) und einem "Auto" (Ackermann-Lenkung)

### 1\. Einkaufsliste für den empfohlenen "Differential Drive" (Panzer-Prinzip)

Das ist der Standard für ROS-Einsteiger, weil der Roboter auf der Stelle drehen kann.

**A. Das Chassis (High-End & Robust)**
Hier hast du Metall, gute Motoren und Encoder fix und fertig.

  * **Waveshare Mobile Robot Chassis (Aluminium):** Ein sehr beliebtes, fast "unkaputtbares" Chassis. Es hat eine Montageplatte, die perfekt für Raspberry Pi und Lidar vorgebohrt ist.
      * *Hinweis:* Oft als 3-Rad (2 Antrieb + 1 Stützrad) oder 4-Rad Version verfügbar. Für ROS ist die 3-Rad (2WD) oder 4WD Version super.
      * [Waveshare Robot Chassis bei BerryBase](https://www.berrybase.de/waveshare-smart-mobile-robot-chassis-kit-4x-raeder-4x-tt-motor-metall-chassis-stossdaempfend) (Achte auf die Version mit Encoder-Kabeln\! Wenn dort "TT Motor" steht, sind es oft die Gelben *ohne* Encoder. Für Profi-Anspruch musst du ggf. die Motoren gegen **JGA25-370 mit Encoder** tauschen, siehe unten).

**B. Die Profi-Motoren (Falls im Chassis nur "Spielzeug" steckt)**
Wenn du ein Chassis kaufst, wirf die gelben Plastikmotoren weg und bau diese ein:

  * **JGA25-370 DC Motor mit Encoder (12V):** Der Industriestandard für kleine Rover.
      * [JGA25-370 mit Encoder bei Amazon](https://www.google.com/search?q=https://www.amazon.de/s%3Fk%3DJGA25-370%2Bencoder%2B12v)
      * [JGA25-370 bei Eckstein Komponente](https://www.google.com/search?q=https://eckstein-shop.de/Suche%3Fq%3DJGA25-370)

**C. Lidar (Das Auge)**

  * **RPLIDAR A1M8:** Der Klassiker für Mapping.
      * [RPLIDAR A1M8 bei BerryBase](https://www.google.com/search?q=https://www.berrybase.de/rplidar-a1m8-360-grad-laser-scanner-kit-12m-reichweite)

-----

### 2\. Zu deiner Frage: "Was ist mit zwei Motoren für Antrieb und Räder für Lenkung?"

Du beschreibst hier die **Ackermann-Lenkung** (wie bei einem echten Auto: Hinterräder schieben, Vorderräder lenken).

**Kurze Antwort:**
Für einen *autonomen* Roboter (ROS 2) ist das **viel schwieriger** zu programmieren als der "Panzer-Antrieb".

**Die technische Erklärung:**

| Feature | Differential Drive (Empfohlen) | Ackermann-Lenkung (Deine Idee) |
| :--- | :--- | :--- |
| **Drehung** | **Zero-Turn:** Kann sich auf der Stelle drehen (0 Meter Radius). | **Wendekreis:** Braucht Platz (wie ein Auto beim Einparken). |
| **Mathematik** | Einfach. $v_L = v_R$ -\> Geradeaus. $v_L = -v_R$ -\> Drehen. | Komplex ("Non-holonomic constraint"). Der Roboter *kann* sich nicht seitlich bewegen oder auf der Stelle drehen. |
| **Navigation** | ROS-Standard (`diff_drive_controller`). Findet immer einen Weg, auch in engen Fluren. | Braucht spezielle Planer (`teb_local_planner`), die "Wende-Manöver" berechnen können (vor-zurück-vor). |
| **Mechanik** | Simpel (Nur Motoren). | Komplex (Braucht Servo, Lenkgestänge, Spurstangen). |

**Wann nimmt man Ackermann?**

  * Wenn du **schnell** fahren willst (Outdoor, \> 2 m/s). Differential Drive wird bei hohem Tempo instabil.
  * Wenn du ein **echtes Auto** simulieren willst (z. B. für autonomes Fahren Forschung).

**Links zu Ackermann-Chassis (Falls du die Herausforderung suchst):**
Es gibt fertige Kits, die "Rosmaster" oder "Ackermann Chassis" heißen.

  * **Yahboom Rosmaster R2 / X3:** Fertige Roboter mit Ackermann-Lenkung, die speziell für ROS gebaut sind.
      * [Yahboom Ackermann Chassis (Beispielsuche)](https://www.google.com/search?q=yahboom+ackermann+chassis+kaufen)

### Fazit & Empfehlung

Wenn das dein **erster** ROS-Roboter ist: **Bleib beim Differential Drive (2 Motoren + Stützrad).**
Der Frust bei der Ackermann-Lenkung entsteht, wenn der Roboter in einer Sackgasse steht. Der Differential-Drive-Roboter dreht sich einfach um 180° und fährt raus. Der Ackermann-Roboter muss mühsam rangieren (Rückwärtsgang -\> Lenken -\> Vorwärts -\> Lenken), was in der Software schwer fehlerfrei umzusetzen ist.

Video, das den Unterschied im Bewegungsablauf zeigt:
[Different types of mobile robot chassis explanation](https://www.google.com/search?q=https://www.youtube.com/watch%3Fv%3DbxGpiN-j_SA)
Dies ist relevant, da es visuell verdeutlicht, warum Differential Drive (Panzersteuerung) viel wendiger ist als Ackermann-Lenkung (Autosteuerung) und warum das für deine Navigation wichtig ist.

