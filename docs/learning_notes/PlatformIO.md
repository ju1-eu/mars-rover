# ⚙️ PlatformIO Troubleshooting & Upgrade Guide

Dieses Dokument beschreibt die notwendige Sequenz, um die PlatformIO-Toolchain zu aktualisieren und den spezifischen **macOS Python-Pfadfehler (PEP 668)** zu beheben, der für die Kompilierung der ESP32-S3-Firmware (`intelhex` ModuleNotFoundError) notwendig war.

## 1. Systemwartung und Upgrade (Standard)

Diese Schritte stellen sicher, dass die PlatformIO-Kernkomponenten und alle benötigten Plattformen auf dem neuesten Stand sind.

| Befehl | Zweck | Anmerkung |
| :--- | :--- | :--- |
| `pip install -U platformio` | Aktualisiert den **PlatformIO Core** (die CLI-Engine) über den Python Package Manager. | Stellt sicher, dass das Basiswerkzeug aktuell ist. |
| `pio upgrade` | Führt ein internes Upgrade der PlatformIO-Struktur aus. | Bestätigt die neueste Version (`6.1.18+`). |
| `pio platform update espressif32` | Aktualisiert die **Espressif 32 Development Platform** (GCC, Framework). | Stellt sicher, dass die Build-Tools aktuell sind (z.B. von v6.10 auf v6.12). |
| `platformio run` | Führt einen vollständigen Build für alle Umgebungen (`uno`, `xiao_esp32s3`) aus. | Dient als erste Testphase nach dem Upgrade. |

---

## 2. Kritische Fehlerbehebung: macOS Python-Pfad (PEP 668)

Dieser Abschnitt löst den Fehler **`ModuleNotFoundError: No module named 'intelhex'`**, der durch den macOS-Systemschutz (`externally-managed-environment`) verursacht wird.

| Befehl | Ziel | Ergebnis |
| :--- | :--- | :--- |
| `pip install intelhex --user --break-system-packages` | **Injektion des `intelhex`-Moduls** in den Benutzerpfad. | Umgeht den Systemschutz (PEP 668) und installiert die kritische Abhängigkeit, die das `esptool.py` benötigt. |
| `pio platform uninstall espressif32` | **Deinstallation** der Plattform. | Entfernt alle internen, möglicherweise fehlerhaften Tool-Kopien (wie die alte `tool-esptoolpy`). |
| `pio platform install espressif32` | **Neuinstallation** der Plattform. | Stellt sicher, dass alle notwendigen Abhängigkeiten (Toolchains, Frameworks) neu und konsistent geladen werden. |
| `python3 -m site --user-site` | **Pfadidentifikation** (Zusatzschritt) | Dient der Ermittlung des genauen Pfades (z.B. `/Users/jan/Library/Python/3.14/...`) für den Workaround. |

---

## 3. Validierung und Deployment

Dieser Schritt validiert die erfolgreiche Kompilierung durch Nutzung des Workarounds.

| Befehl | Zweck | Status |
| :--- | :--- | :--- |
| `platformio run --environment xiao_esp32s3` | **Build-Validierung.** Führt die Kompilierung unter Verwendung des **`PYTHONPATH` Workarounds** aus der `add_python_path.py` aus. | **SUCCESS** (Kompilierung fehlerfrei) |
| `pio run --target upload` | **Deployment.** Versucht, alle Umgebungen hochzuladen. | **SUCCESS** für ESP32S3, **FAILED** für Uno (wenn nicht verbunden). |
| `pio run --environment uno` | **Isolierter Uno-Check.** Prüft nur die Kompilierbarkeit der AVR-Firmware. | **SUCCESS** (nach Code-Korrektur der STL- und `printf`-Probleme). |

