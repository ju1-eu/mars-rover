# 📜 CHANGELOG (Änderungsprotokoll)

Alle bemerkenswerten Änderungen an diesem Projekt werden in dieser Datei dokumentiert.

Das Format basiert auf [Keep a Changelog](https://keepachangelog.com/en/1.0.0/) und die Versionierung folgt **SemVer** (Major.Minor.Patch).

## [1.1.0] - 2025-11-22

### 🎁 Hinzugefügt (Added)

* Einführung der **bedingten Kompilierung** in `src/main.cpp` (`#if defined(ARDUINO_ARCH_ESP32)`), um STL-Code nur für ressourcenstarke Architekturen zu aktivieren.
* Neuer **`PYTHONPATH` Workaround** (`extra_scripts = pre:add_python_path.py`) zur Behebung des `intelhex`-Fehlers auf macOS.

### 🔄 Geändert (Changed)

* `platformio.ini`: Der Build-Flag für die ESP32S3-Umgebung wurde auf den unterstützten Standard **`-std=gnu++2a` (C++20)** zurückgesetzt (vorher fehlerhafter Versuch mit C++23).
* `src/main.cpp`: Die Test-Funktion `verarbeiteDaten_AVR_Dummy()` wurde als Platzhalter für die `uno`-Umgebung hinzugefügt, um Kompilierungsfehler zu vermeiden.

### 🩹 Behoben (Fixed)

* **Fataler Fehler:** Der `fatal error: vector: No such file or directory` in der `uno`-Umgebung wurde durch die bedingte Kompilierung behoben.
* **Build-Fehler:** Der `error: 'class HardwareSerial' has no member named 'printf'` in der `uno`-Umgebung wurde durch die Implementierung von plattformspezifischer `Serial.print` Logik behoben.
* **Upload-Fehler:** Der `ModuleNotFoundError: No module named 'intelhex'` auf macOS wurde durch die erzwungene Installation via `pip install intelhex --user --break-system-packages` und die korrekte Pfad-Injektion gelöst.

## [1.0.0] - 2025-11-20

### 🎁 Hinzugefügt (Added)

* Initialer Release der Firmware mit **Schichten-Architektur** (HAL, Logic, Application).
* Erste Implementierung des Motor-Antriebsstrangs (`HAL::Motor`, `Logic::Motion`).
* Definition der `platformio.ini` mit zwei Environments (`uno`, `xiao_esp32s3`).
* Einführung der Dokumentation (`README.md`, `CONTRIBUTING.md`).

### 🩹 Behoben (Fixed)

* Initialer Upload-Port-Konflikt auf dem Uno (Upload-Port manuell in `platformio.ini` spezifiziert).
