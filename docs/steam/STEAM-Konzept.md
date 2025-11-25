# Was ist STEAM?

* STEAM steht für **Science, Technology, Engineering, Arts, Mathematics**.
* Es ist kein einzelnes Fach, sondern ein **interdisziplinärer Ansatz**: echte Probleme werden so bearbeitet, dass Naturwissenschaft, Technik, Ingenieurdenken, Gestaltung/Kunst und Mathematik miteinander verknüpft werden.
* Ziel: **Problemlösen, Kreativität, kritisches Denken, Teamarbeit** – nicht nur Formeln lernen, sondern Dinge bauen, testen, verbessern.

Im deutschsprachigen Raum ist STEAM die „MINT-Erweiterung“: zu Mathematik, Informatik, Naturwissenschaft und Technik kommt explizit der kreative Anteil (Design, Medien, Storytelling, Kunst).

---

Das Projekt **GalaxyRVR Profi** eignet sich hervorragend für fortgeschrittene **STEAM-Bildung** (Science, Technology, Engineering, Arts, Mathematics).

### 1. Science (Naturwissenschaften): Physik & Sensorik
Das Projekt bietet eine praktische Anwendung physikalischer Gesetzmäßigkeiten durch die integrierte Sensorik.

* **Kinematik & Trägheit:** Die Firmware nutzt eine IMU (MPU6050), um Beschleunigung und Drehraten zu messen. Das Konzept der **Sensor-Fusion (Komplementärfilter)** wird im `src/hal/`-Layer angewendet, um stabile Neigungswinkel (Pitch/Roll) zu berechnen.
* **Akustik:** Der Einsatz des Ultraschallsensors zur Hinderniserkennung demonstriert das Prinzip der Laufzeitmessung von Schallwellen (`Obstacle Avoidance`).
* **Lernziel:** Verstehen, wie physikalische Rohdaten (Spannungswerte, I2C-Signale) in reale physikalische Größen (Winkel, Distanz) umgewandelt werden.

### 2. Technology (Technologie): Modernes Embedded Development
Hier liegt der Schwerpunkt des "Profi"-Ansatzes. Statt veraltetem Arduino-C wird moderner Industriestandard gelehrt.

* **C++ Standards:** Nutzung von **C++17** (Arduino Uno) und **C++20** (ESP32) führt Lernende an moderne Syntax und Features heran, statt bei "C mit Klassen" stehenzubleiben.
* **Toolchain-Kompetenz:** Abkehr von der simplen Arduino IDE hin zu **PlatformIO**, mit Environment-Management (`env:uno` vs. `env:uno-debug`) und Build-Konfigurationen.
* **Cross-Platform Development:** Die Architektur unterstützt parallel zwei völlig unterschiedliche Chipsätze (ATmega328P vs. ESP32-S3), was die Bedeutung von Portabilität in der Technologie verdeutlicht.

### 3. Engineering (Ingenieurwissenschaften): Systemarchitektur
Das Projekt lehrt strukturiertes Ingenieursdenken durch das Prinzip der **Separation of Concerns** (Trennung der Zuständigkeiten).

* **Schichten-Architektur:** Die strikte Trennung in **HAL** (Hardware), **Logic** (Verhalten) und **Application** (Main) zeigt, wie komplexe Systeme wartbar bleiben.
* **State Machines (FSM):** Die Steuerung erfolgt über einen Endlichen Automaten. Dies ist ein fundamentales Konzept im Engineering, um definierte Zustände (Cruise, Smart Turn, Safety) sicher zu verwalten, anstatt unvorhersehbaren "Spaghetti-Code" zu schreiben.
* **Problemlösung unter Restriktionen:** Die Lösung des Hardware-PWM-Mangels am Arduino Uno (nur 6 Kanäle) durch eine hybride Lösung aus Hardware- und Software-PWM (`SoftPWM`) ist ein klassisches Embedded-Engineering-Problem.

### 4. Arts (Kunst & Gestaltung): Clean Code & Dokumentation
Das "A" in STEAM steht oft für Design, aber in der Informatik auch für die "Kunst des sauberen Codes" und der Kommunikation.

* **Code als Handwerk:** Die **Coding Guidelines** (z. B. `constexpr`, keine `delay()`, starke Typisierung mit `enum class`) vermitteln Ästhetik und Lesbarkeit im Code.
* **Technische Dokumentation:** Das Projekt legt großen Wert auf Struktur (`kanban.board`, Versionierung, klare Ordnerstruktur `docs/`, `src/`). Lernende erfahren, dass gute Technik ohne gute Dokumentation wertlos ist.
* **Struktur:** Die Organisation des Projekts ist visuell und logisch so aufbereitet, dass sie intuitiv erfassbar ist – ein Aspekt von Design im Software-Kontext.

### 5. Mathematics (Mathematik): Algorithmen & Regelung
Mathematik ist im `logic/`-Layer das operative Herzstück des Rovers.

* **Differentielle Lenkung:** Die Berechnung der Kurvenfahrt ist angewandte Algebra. Die Formel $v_\text{in} = v \cdot (1.0 - r)$ zeigt linear-proportionale Zusammenhänge zwischen Kurvenradius und Radgeschwindigkeit.
* **Regelungstechnik (PID/P-Regler):** Der **DriveAssistant** nutzt einen P-Regler (`Korrektur = Gierrate * P_Faktor`), um den Rover geradeaus zu halten. Lernende sehen hier direkt die Auswirkung eines mathematischen Faktors auf das physikalische Verhalten (Oszillation vs. Stabilität).
* **Zeit-Mathematik:** Das Verständnis von `millis()`-Arithmetik ($t_\text{now} - t_\text{last} > \Delta t$) ist essenziell für das Verständnis von diskreter Zeit in digitalen Systemen.

---

## Zusammenfassung des STEAM-Kurses

Das Projekt **GalaxyRVR Profi** transformiert das Spielzeug "Mars Rover" in ein **Lehrmittel für Software-Engineering**.

| Ebene | Was der Schüler lernt (im Gegensatz zu Standard-Tutorials) |
| :--- | :--- |
| **Einsteiger** | "Code lässt die Lampe blinken." |
| **GalaxyRVR Profi** | "Ein **Zustandsautomat** steuert basierend auf **Sensor-Fusion** und **Regelkreisen** die Aktoren in **Echtzeit** (Non-Blocking)." |

Wir haben das Projekt **GalaxyRVR Profi** nun durch alle fünf Linsen betrachtet:

1.  **Science:** Sensoren und Physik (IMU, Schall).
2.  **Technology:** C++17, Non-Blocking I/O und Toolchains.
3.  **Engineering:** Schichten-Architektur und Hardware-Abstraktion.
4.  **Arts:** Clean Code, Struktur und Dokumentation.
5.  **Mathematics:** Kinematik, Regelkreise und Integrale.

Diese ganzheitliche Betrachtung macht aus einem einfachen Bausatz ein komplexes Lehrmittel für angehende Ingenieure und Informatiker.
