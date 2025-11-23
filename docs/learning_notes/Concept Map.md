# 🗺️ Concept Map: Wissensvernetzung

### 1. Kernkonzept: Wissensvernetzung (Knowledge Networking)
* **Definition:** Prozess des Sammelns, Organisierens und Beziehens von diskreten Wissenselementen, um ein kohärentes und anwendbares Gesamtverständnis zu schaffen.
* **Ziel:** Schaffung von **Transferwissen** und **Tiefenverständnis** statt bloßem Faktenwissen.

---

### 2. Architektonische Schichten (Strukturierung)
* **2.1. Daten-Schicht (Elemente)**
    * **Konzepte (Nodes):** Die grundlegenden Substantive und Ideen (z. B. "C++-Standard", "HAL", "SoftPWM").
    * **Attribute:** Spezifische Merkmale oder Eigenschaften eines Konzepts (z. B. "C++17", "PWM-Duty-Cycle [0-255]").
    * **Fakten:** Verifizierbare Aussagen (z. B. "Pin 2 kann kein Hardware-PWM").
* **2.2. Relations-Schicht (Verbindungen)**
    * **Hierarchie (IST-EIN):** Klassifizierung (z. B. "Motor ist-ein Aktor").
    * **Assoziation (KENNT):** Abhängigkeit (z. B. "Logik-Schicht kennt HAL-Schicht").
    * **Kausalität (VERURSACHT):** Ursache-Wirkung (z. B. "Non-Blocking I/O **verursacht** geringere Latenz").
    * **Präskriptiv (MUSS):** Regel oder Richtlinie (z. B. "HAL **muss** im `setup()` aufgerufen werden").
* **2.3. Anwendung-Schicht (Kontext)**
    * **Projekte/Anwendungsfälle:** Der übergeordnete Zweck (z. B. "GalaxyRVR Profi - Firmware").
    * **Workflows:** Sequenz von Schritten (z. B. "Workflow für Erweiterungen: Pin definieren $\rightarrow$ Treiber erstellen $\rightarrow$ Logik implementieren $\rightarrow$ FSM ergänzen").

---

### 3. Prinzipien der Vernetzung (Methodik)
* **3.1. Abstraktion und Hierarchie**
    * **Layering (Schichtung):** Trennung von Verantwortlichkeiten (Application $\rightarrow$ Logic $\rightarrow$ HAL).
    * **Kapselung:** Verbergen von Implementierungsdetails (z. B. anonymer Namespace in C++).
    * **API-Definition:** Klare, konsistente Schnittstellen (z. B. Doxygen-Kommentare in `Motor.h`).
* **3.2. Referenzierung und Dokumentation**
    * **Traceability (Rückverfolgbarkeit):** Verlinkung von Code zu externen Anforderungen/Dokumenten (z. B. Schaltplänen, Jira-Tickets).
    * **Metadaten:** Kontextinformationen (Autor, Version, Datum, `@warning`, `@safety`).
    * **Konsistente Terminologie:** Fachbegriffe (z. B. "Pivot-Turn", "Non-Blocking I/O").
* **3.3. Konflikt- und Sicherheitsmanagement**
    * **Grenzwertbegrenzung (Clamping):** Abfangen ungültiger Eingaben (z. B. `SpeedMax = 255`).
    * **Constraint-Dokumentation:** Erklären von Einschränkungen (z. B. "Pin X kann kein Hardware-PWM").
    * **Fehlerbehandlung:** Definition sicherer Zustände (z. B. `stop()` als Fail-Safe).

---

### 4. Anwendungsbeispiel: Motorsteuerung (Integrativer Fokus)
* **4.1. Applikationskonzept (Ziel):** **Fahren einer Testsequenz (FSM)**
    * *Verbindung zu:* Logik-Schicht (fordert `moveForward`)
    * *Verbindung zu:* Prinzipien (nutzt Non-Blocking Timing $t_{now} - t_{last} > \Delta t$)
* **4.2. Logik-Konzept (Umsetzung):** **Differenzial-Kinematik**
    * *Verbindung zu:* HAL (ruft `HAL::Motor::setSpeed`)
    * *Formel-Beziehung:* $PWM_L = -v, \quad PWM_R = +v$ (Panzer-Wende)
* **4.3. HAL-Konzept (Hardware-Zugriff):** **PWM-Treiber**
    * *Verbindung zu:* Pins (`Pin::MotorL_Forward`)
    * *Sicherheitsbeziehung:* Implementiert Clamping und sorgt für korrekte H-Brücken-Ansteuerung (Pin FWD/REV nicht gleichzeitig HIGH).
