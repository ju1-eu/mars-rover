# To-Do-Checkliste

### 🟢 Phase 1: Rückkehr zum Rover (Setup)
*Aktueller Status: Die Umgebung ist noch auf den ESP32 eingestellt.*

- [ ] **Environment wechseln:** In VS Code unten in der blauen Leiste von `env:xiao_esp32s3` auf **`env:uno`** zurückschalten.
- [ ] **Hardware prüfen:** USB-B-Kabel (Druckerkabel) am Rover anschließen und Batterie-Schalter auf **ON**.
- [ ] **Code aufräumen:** Den Inhalt von `src/main.cpp` (aktuell ESP32-Test) löschen oder als `test/esp32_hello_world.cpp` sichern.

---

### 🔵 Phase 2: Lektion 5 umsetzen (Motion Control)
*Ziel: Die abstrakte "Motion"-Schicht nutzen, statt roher Motorbefehle.*

- [ ] **Dateien prüfen:** Sicherstellen, dass folgende Dateien existieren und gefüllt sind:
    - [ ] `include/Motion.h` (Die Befehle)
    - [ ] `src/logic/Motion.cpp` (Die Übersetzung in Motor-Speed)
- [ ] **Main implementieren:** Den Code für die **Choreografie** (Geradeaus -> Kurve -> Drehen) in `src/main.cpp` einfügen (siehe Chatverlauf oben).
- [ ] **Upload & Test:** Code auf den Rover laden und prüfen, ob er die Manöver fährt.
- [ ] **Versionierung:** Wenn es klappt → `git commit` ("feat: Lektion 5 Motion Control abgeschlossen").

---

### 🟠 Phase 3: Routine für kommende Lektionen (6–13)
*Wende diesen Workflow für jede neue Lektion an:*

**1. Vorbereitung**
- [ ] **Hardware verstehen:** Welcher Sensor kommt neu dazu? (z. B. IR-Sensor in Lektion 6).
- [ ] **Pins definieren:** In `include/Pins.h` die neuen Pins eintragen (als `constexpr`).
- [ ] **Konstanten setzen:** Schwellenwerte (z. B. "Abstand für Stopp") in `include/Config.h` eintragen.

**2. Treiber schreiben (HAL)**
- [ ] **Header:** `src/hal/NeuerSensor.h` erstellen (Was kann der Sensor?).
- [ ] **Implementierung:** `src/hal/NeuerSensor.cpp` erstellen (Wie liest man ihn aus? `digitalRead`, `analogRead` etc.).
- [ ] **Isolation:** Teste den neuen Treiber kurz mit einer minimalen `main.cpp`, bevor du weitermachst.

**3. Logik integrieren**
- [ ] **Verhalten:** Erweitere den Zustandsautomaten (`enum class State`) in `src/main.cpp` um den neuen Modus (z. B. `State::HindernisVermeidung`).
- [ ] **Verknüpfung:** Rufe im neuen Zustand die Daten aus der HAL ab und entscheide über die Bewegung.

**4. Abschluss**
- [ ] **Refactoring:** Ist die `main.cpp` noch lesbar? Falls zu voll → Logik in `src/logic/` auslagern.
- [ ] **Doku:** `README.md` aktualisieren (neue Features abhaken).

---

## 📁 ergänzende Dokumentationsdateien


| Dateiname | Zweck | Format | Beispielhafte Inhalte |
| :--- | :--- | :--- | :--- |
| **`LICENSE.md`** | Rechtliche Lizenz des Projekts. | Standardtext oder Markdown | Enthält die Nutzungsbedingungen (z. B. MIT). |
| **`CONTRIBUTING.md`** | Richtlinien für Code-Beiträge. | Markdown | Erklärt den Git-Workflow, Code-Styles und Pull-Request-Prozess. |
| **`CHANGELOG.md`** | Chronologische Liste aller Änderungen und neuen Versionen. | Markdown | Detaillierte Auflistung von neuen Features, Fixes und Breaking Changes. |
| **`Doxyfile`** | Konfigurationsdatei für den Doxygen-Dokumentationsgenerator. | Konfigurationsdatei | Steuert, wie die Header-Kommentare in Handbücher umgewandelt werden. |
| **`docs/`** | Separates Verzeichnis für umfangreiche Dokumentation. | Markdown/HTML/PDF | Hier werden detaillierte Anleitungen, Architekturdiagramme oder Spezifikationen abgelegt. |
