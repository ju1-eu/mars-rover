# 📜 CHANGELOG (Änderungsprotokoll)

Alle bemerkenswerten Änderungen an diesem Projekt werden in dieser Datei dokumentiert.

Das Format orientiert sich an [Keep a Changelog](https://keepachangelog.com/en/1.0.0/) und die Versionierung folgt **SemVer** (Major.Minor.Patch).

---

## [1.1.0] - 2025-11-22

### 🎁 Hinzugefügt (Added)

* Einführung der **bedingten Kompilierung** in `src/main.cpp`
  (`#if defined(ARDUINO_ARCH_ESP32) || defined(ARDUINO_ARCH_ESP32S3)`), um STL-Code (`<vector>`) nur auf ressourcenstarken ESP32-Plattformen zu aktivieren.
* Neuer **PYTHONPATH-Workaround** über `extra_scripts = pre:add_python_path.py`, um den `intelhex`-Importfehler in der PlatformIO-Umgebung unter macOS zu umgehen.

### 🔄 Geändert (Changed)

* `platformio.ini`: Der Build-Flag für die `xiao_esp32s3`-Umgebung wurde auf den unterstützten Standard **`-std=gnu++2a` (C++20)** zurückgesetzt (zuvor fehlerhafter Versuch mit C++23).
* `src/main.cpp`: Die Dummy-Funktion `verarbeiteDaten_AVR_Dummy()` wurde für die `uno`-Umgebung ergänzt, um STL-freien Code zu bieten und Kompilierungsfehler bei fehlender `<vector>`-Unterstützung zu vermeiden.

### 🩹 Behoben (Fixed)

* **Fataler Fehler:** `fatal error: vector: No such file or directory` in der `uno`-Umgebung durch saubere Trennung von STL- und Nicht-STL-Code per bedingter Kompilierung behoben.
* **Build-Fehler:** `error: 'class HardwareSerial' has no member named 'printf'` in der `uno`-Umgebung durch plattformspezifische `Serial.print`/`Serial.printf`-Logik behoben.
* **Upload-/Toolchain-Fehler:** `ModuleNotFoundError: No module named 'intelhex'` auf macOS durch explizite Installation (`pip install intelhex --user --break-system-packages`) und korrekte Pfad-Injektion via `add_python_path.py` behoben.

---

## [1.0.0] - 2025-11-20

### 🎁 Hinzugefügt (Added)

* Initialer Release der Firmware mit **Schichten-Architektur** (HAL, Logic, Application).
* Erste Implementierung des Motor-Antriebsstrangs (`HAL::Motor`, `Logic::Motion`).
* Definition der `platformio.ini` mit zwei Environments: `uno` und `xiao_esp32s3`.
* Einführung der Basisdokumentation (`README.md`, `CONTRIBUTING.md`).

### 🩹 Behoben (Fixed)

* Upload-Port-Konflikt auf dem Uno durch explizite Konfiguration von `upload_port` in der `platformio.ini` behoben.
