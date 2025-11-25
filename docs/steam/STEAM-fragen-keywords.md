# Schlüsselfragen und keywords

Checkliste für die Lernzielkontrolle

### 1. Science (Physik & Sensorik)
*Thema: Trägheit, IMU & Datenfusion*

**Keywords:**
* IMU (Inertial Measurement Unit)
* Akzelerometer (Beschleunigungssensor)
* Gyroskop (Drehratensensor)
* Drift (Messfehler über Zeit)
* Sensor-Fusion / Komplementärfilter
* I2C-Bus (SDA/SCL)

**Schlüsselfragen:**
* Warum reicht ein einzelner Sensor (nur Gyro oder nur Beschleunigung) nicht aus, um den Winkel stabil zu messen?
* Was passiert physikalisch mit dem Beschleunigungssensor, wenn der Rover über Kopfsteinpflaster fährt?
* Warum driftet ein Gyroskop und wie korrigiert der Komplementärfilter diesen Fehler mathematisch?

---

### 2. Technology (Informatik & Echtzeit)
*Thema: Multitasking & Zustandsautomaten*

**Keywords:**
* Non-Blocking I/O
* Polling vs. Interrupts
* `millis()` / Zeit-Delta ($\Delta t$)
* Finite State Machine (FSM) / Endlicher Automat
* C++17 / C++20 Standards
* PlatformIO (Build-Umgebungen)

**Schlüsselfragen:**
* Warum ist der Befehl `delay(2000)` in einem Robotik-System gefährlich?
* Wie kann ein Single-Core-Prozessor (wie der Arduino) scheinbar mehrere Aufgaben gleichzeitig erledigen?
* Welche Zustände (States) hat der Rover und welche Bedingung (Transition) führt vom Zustand `CRUISE` in `OBSTACLE_AVOIDANCE`?

---

### 3. Engineering (Systemarchitektur)
*Thema: Modularität & Hardware-Restriktionen*

**Keywords:**
* Separation of Concerns (Trennung der Zuständigkeiten)
* HAL (Hardware Abstraction Layer)
* Logic Layer vs. Application Layer
* Software-PWM vs. Hardware-PWM
* Treiber-Entwicklung
* Portabilität

**Schlüsselfragen:**
* Warum sollte die `main.cpp` nicht direkt wissen, an welchem Pin der Motor angeschlossen ist?
* Wie löst man das Problem, dass der Arduino Uno nur 6 Hardware-PWM-Pins hat, aber mehr benötigt werden?
* Was ist der Vorteil, wenn man Hardware-Zugriffe (HAL) von der Bewegungslogik trennt?

---

### 4. Arts (Code-Qualität & Gestaltung)
*Thema: Clean Code & Dokumentation*

**Keywords:**
* Clean Code
* Magische Zahlen (Magic Numbers)
* `constexpr` (Kompilierzeit-Konstante)
* Starke Typisierung (`enum class`)
* Wartbarkeit & Lesbarkeit
* Namenskonventionen

**Schlüsselfragen:**
* Warum ist `motor.setSpeed(Speed::FAST)` besserer Code als `analogWrite(5, 255)`?
* Warum definieren wir Pins in einer zentralen Datei (`Pins.h`) und nicht dort, wo wir sie gerade brauchen?
* Wie hilft eine visuelle Ordnerstruktur (docs, src, include) neuen Entwicklern beim Einstieg?

---

### 5. Mathematics (Algorithmen & Regelung)
*Thema: Kinematik, Vektoren & P-Regler*

**Keywords:**
* Differentielle Lenkung (Differential Drive)
* Kinematik (Bewegungslehre)
* P-Regler (Proportional-Regler)
* Gierrate (Yaw Rate)
* Integration (Aufsummieren über Zeit)
* Totzone (Deadzone)
* Oszillation (Schwingen)

**Schlüsselfragen:**
* Wie berechnet sich die Geschwindigkeit des inneren Rades bei einer Kurvenfahrt ($v_{in} = v \cdot (1.0 - r)$)?
* Was passiert mit dem Fahrverhalten, wenn der Proportional-Faktor ($K_p$) im Regler zu hoch gewählt wird?
* Warum brauchen wir eine "Totzone" bei der Sensor-Auswertung und was würde ohne sie passieren?

