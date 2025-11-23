# 🤝 Leitfaden für Code-Beiträge (CONTRIBUTING)

Willkommen beim **GalaxyRVR Profi**-Projekt\! Wir freuen uns über Ihren Beitrag zur Verbesserung der Firmware. Um die Qualität und die strikte Einhaltung der Architekturstandards zu gewährleisten, bitten wir Sie, die folgenden Richtlinien zu beachten.

## 1\. Architektur-Regeln (Separation of Concerns)

Das Projekt nutzt eine **3-Schichten-Architektur**. Jeder Beitrag muss in die dafür vorgesehene Ebene integriert werden.

| Schicht | Zweck des Codes | Verzeichnis | Verbotene Abhängigkeiten |
| :--- | :--- | :--- | :--- |
| **Anwendung (Application)** | High-Level-Verhalten, FSM-Logik, Startsequenz (`setup()`). | `src/main.cpp` | Direkter Zugriff auf Hardware-Pins oder Timer. |
| **Logik & Verhalten (Logic)** | Kinematik-Berechnung, PID-Regler, Zustandsverwaltung (Was soll der Roboter tun?). | `src/logic/` | Zugriff auf `Pin::` oder `SoftPWM`. Kennt nur die HAL. |
| **HAL (Hardware Abstraction Layer)** | Direkte Pin-Ansteuerung, ADC-Lesen, Timer-Steuerung. | `src/hal/` | Logik über die reine Hardware-Bedienung hinaus (z.B. Umrechnung von Encoder-Ticks in m/s). |

-----

## 2\. Coding Guidelines (Qualitätssicherung)

Alle Beiträge müssen die etablierten **Embedded C++**-Standards des Projekts erfüllen:

### 2.1. C++ Syntax und Features

  * **Compiler-Standards:** Halten Sie die Build-Flags (`platformio.ini`) ein: C++17 für AVR (`uno`), C++20 für ESP32S3.
  * **Typisierung:** Verwenden Sie **`enum class`** für alle Zustände, um Typkonflikte zu vermeiden.
  * **Ressourcenmanagement:** Nutzen Sie **`constexpr`** für alle systemweiten Konstanten (`Config.h`), um den RAM-Verbrauch zur Laufzeit zu minimieren.
  * **STL:** Die Standard Template Library (`std::vector`, etc.) ist nur für die Umgebung **`env:xiao_esp32s3`** zulässig.

### 2.2. Non-Blocking I/O

  * **Verbot von `delay()`:** Die Funktion `delay()` ist in der gesamten Laufzeitlogik (`loop()`) strengstens untersagt, da sie das System blockiert.
  * **Zeitsteuerung:** Alle zeitkritischen Aufgaben müssen über **Polling der Systemzeit** (`millis()`) gesteuert werden:
    $$\text{Aktion ausführen, WENN } t_{now} - t_{last} > \Delta t$$

### 2.3. Kommentierung und Dokumentation

  * **Doxygen:** Alle öffentlichen Funktionen (`.h`-Dateien) und komplexen Logikblöcke (`.cpp`-Dateien) müssen im **Doxygen-Stil** kommentiert werden (mit `@brief`, `@param`, `@return`).
  * **Klarheit:** Kommentare müssen das **"Warum"** erklären (Intention, Nebenbedingungen), nicht das "Was" (tautologische Code-Beschreibung).
  * **Sprache:** Interne Kommentare sind in Deutsch zulässig, Doxygen-Header-Dateien sollten jedoch idealerweise in Englisch verfasst werden, um die internationale Wartbarkeit zu gewährleisten.

-----

## 3\. Workflow für Beiträge (Pull Requests)

Folgen Sie diesen Schritten, um Ihren Beitrag zur Prüfung einzureichen:

1.  **Issue erstellen:** Melden Sie den Fehler, den Sie beheben, oder das Feature, das Sie hinzufügen möchten, in einem [Issue-Ticket](https://www.google.com/search?q=).
2.  **Branch erstellen:** Erstellen Sie einen Feature-Branch von der Hauptentwicklungslinie (`develop` oder `main`):
    `git checkout -b feature/Ihr-Feature-Name`
3.  **Code-Änderungen:** Implementieren Sie die Änderungen und stellen Sie sicher, dass alle Kommentierungs- und Architekturregeln eingehalten werden.
4.  **Testen:** Führen Sie einen lokalen Build für **beide Umgebungen** aus, um Kompatibilität zu gewährleisten:
    `platformio run`
5.  **Pull Request (PR):** Erstellen Sie einen Pull Request und verlinken Sie ihn mit dem ursprünglichen Issue-Ticket. Beschreiben Sie kurz, welche Probleme gelöst wurden und welche Auswirkungen die Änderungen auf die RAM/Flash-Nutzung haben.

**Wichtig:** Pull Requests ohne korrekte architektonische Zuordnung oder funktionierenden Build für beide Environments werden abgelehnt.
