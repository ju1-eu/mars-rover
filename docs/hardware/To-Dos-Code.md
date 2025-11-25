# To-Do-Checkliste

## 🟠 Phase: Routine für kommende Lektionen (6–13)
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
