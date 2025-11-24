# ESP32-CAM flashen

* **Arduino-Board**:
  – USB-Kabel an den R3
  – Mode/Upload-Schalter nach **rechts (Upload/ESP32 getrennt)**
  – Dann in der Arduino-IDE ganz normal flashen.

* **ESP32-CAM geht das nicht einfach über denselben USB-Port.**
  Der Mode-Schalter nach links verbindet nur RX/TX von Arduino und ESP32 für die Laufzeitkommunikation, aber dein PC hängt weiterhin nur am USB-Seriell-Wandler des Arduino. Darüber kannst du die ESP32-CAM nicht direkt flashen.([SunFounder Documentation][1])

  Die von SunFounder vorgesehene Methode zum **ESP32-Firmware-Update** ist OTA („over the air“):

  1. Rover einschalten, Mode-Schalter auf **Run/links**, Reset drücken.
  2. Mit dem Handy/Tablet ins WLAN `aiCAM` (PW `12345678`).
  3. Browser: `http://192.168.4.1`.
  4. Dort die `.bin`-Datei auswählen und Update starten.



### 1. Relevante Punkte aus der SunFounder-Seite

Aus der Galaxy-RVR-Dokumentation: ([SunFounder Documentation][1])

* Versorgung: **4,75–5,25 V**, empfohlen **5 V / 2 A**, damit das Kamerabild stabil bleibt.
* **GPIO 1 / GPIO 3** = serielle Pins (U0T/U0R) zum Code-Upload.
* **GPIO 0** wählt Flash-Modus vs. Normalbetrieb:
  – mit GND verbunden → Flash-Modus
  – sonst → normaler Start
* Hinweis: Im normalen Kamerabetrieb GPIO0 „in der Luft lassen“, weil er mit XCLK verbunden ist.

Wichtig: Der Hinweis „GPIO0 nicht fest auf HIGH/LOW legen“ bezieht sich auf den **Normalbetrieb**. Fürs Programmieren darfst du GPIO0 *temporär* mit GND brücken – danach muss der Jumper wieder weg.

---

### 2. AZ-Adapter + ESP32-CAM Verdrahtung

Mit deinem AZ-USB-C-zu-TTL-Adapter (FT232-kompatibel) geht das so: ([AZ-Delivery][2])

| Adapter (FT232) | ESP32-CAM   | Zweck            |
| --------------- | ----------- | ---------------- |
| VCC             | 5V          | Versorgung (5 V) |
| GND             | GND         | Masse            |
| TXD             | U0R (GPIO3) | PC → ESP32       |
| RXD             | U0T (GPIO1) | ESP32 → PC       |

* Logik-Jumper des Adapters auf **3,3 V** stellen (TX/RX-Pegel).
* 5 V-Pin des Adapters an **5V der CAM** (die CAM hat einen eigenen 3,3 V-Regler).

---

### 3. Boot-Modus (Flash) vs. Normalbetrieb

**Zum Flashen:**

1. Jumper zwischen **IO0 und GND** setzen.
2. USB-Kabel einstecken oder RST kurz nach GND → loslassen.
3. ESP32 startet im Bootloader.
4. Mit Arduino-IDE / `esptool.py` die Firmware auf `/dev/cu.usbserial…` schreiben.

**Für normalen Betrieb (z. B. später im Galaxy-RVR):**

* Jumper IO0–GND wieder entfernen → GPIO0 ist „floating“.
* Die SunFounder-Hinweise (GPIO0/XCLK) sind dann erfüllt.

---

### 4. Stromversorgung

* Fürs **Programmieren** reicht in der Praxis der 5 V-Ausgang des USB-TTL-Adapters (typisch 500 mA).
* Für längeren **Kamera- und WLAN-Betrieb mit Bild** gilt die SunFounder-Empfehlung: externe 5 V-Quelle mit ca. **2 A Reserve** (z. B. RVR-Akkupack), sonst drohen Bildstörungen.

Damit ist dein Plan:

* AZ-Adapter + Breadboard wie im AZ-Diagramm
* plus SunFounder-Hinweis, IO0 nur zum Flashen nach GND zu ziehen

technisch konsistent.

[1]: https://docs.sunfounder.com/projects/galaxy-rvr/de/latest/hardware/cpn_esp_32_cam.html "ESP32 CAM — SunFounder GalaxyRVR Kit for Arduino 1.0 Dokumentation"
[2]: https://www.az-delivery.de/en/products/usb-c-zu-ttl-serial-adapter-fur-3-3v-und-5v-kompatibel-mit-ft232rl?pr_prod_strat=e5_desc&pr_rec_id=f3ee3c956&pr_rec_pid=8873701245195&pr_ref_pid=4794545340512&pr_seq=uniform "USB-C zu TTL Serial Adapter für 3,3V und 5V kompatibel mit FT232RL
"

---

<https://github.com/sunfounder/ai-camera-firmware>


1. WLAN-Voreinstellung

   ```cpp
   int mode = AP;              // AP-Modus
   String ssid = "aiCAM";
   String password = "12345678";
   int port = 8765;            // WebSocket-Port
   ```

   → Die CAM spannt standardmäßig ein eigenes WLAN „aiCAM“ im AP-Modus auf.

2. WebSocket-Server

   ```cpp
   WS_Server ws_server = WS_Server();
   ...
   ws_server.begin(port, name, type, CHECK_TEXT);
   ```

   → Es gibt einen WebSocket-Server auf Port 8765.
   Dein `nc -vz 192.168.4.1 8765` = `open` passt exakt dazu.

3. HTTP-Server / Videostream

   ```cpp
   register_httpd(...);          // in camera_init()
   ...
   videoUrl = String("http://") + wifi.ip + ":9000/mjpg";
   ```

   → Der MJPEG-Videostream läuft auf Port 9000, Pfad `/mjpg`.
   Erwartbar: `http://192.168.4.1:9000/mjpg` (oder `/`) liefert ein Bild/Stream, nicht Port 80.

4. OTA-/Settings-Server

   ```cpp
   settingsBegin(VERSION, apChannel);
   ```

   → Hier wird der „Settings Server“ gestartet (die Seite, die SunFounder in der FAQ für Kanalwahl/Firmware-Update zeigt). In der OTA-Binärdatei (`ai-camera-firmware-v1.4.1-ota.bin`) ist dieser Teil so gebaut, dass du ihn unter `http://192.168.4.1` erreichst. ([docs.sunfounder.com][1])

   Dass dein Test

   ```bash
   nc -vz 192.168.4.1 80
   -> Connection refused
   ```

   sagt: Der Settings-/OTA-HTTP-Server läuft in deiner aktuellen Firmware offensichtlich **nicht** auf Port 80 – entweder:

   * es ist eine „normale“ Build-Variante ohne OTA-Webseite, oder
   * der Settings-Server ist in dieser Version deaktiviert / auf einen anderen Port gelegt.

5. Konsequenz für dich

* Mit **diesem** Sketch bekommst du sicher:

  * AP „aiCAM“ (oder später „GalaxyRVR“, wenn der Uno via UART `SET+SSID...` schickt),
  * WebSocket auf Port 8765,
  * MJPEG-Stream auf Port 9000.
* Die in der Doku beschriebene OTA-Seite unter `http://192.168.4.1` bekommst du **nur**, wenn auf dem ESP32-CAM eine der speziellen OTA-Binärdateien (`...-ota.bin`) geflasht ist.

6. Was tun, wenn du unbedingt die OTA-Webseite willst?

7. Einmalig per USB-Seriell-Programmer (oder ESP32-CAM-Programmer-Board) direkt den ESP32-CAM flashen:

- Board: "ESP32 Dev Module"
- Upload Speed: "921600"
- CPU Frequency: "240MHz (WiFi/BT)"
- Flash Frequency: "80MHz"
- Flash Mode: "QIO"
- Flash Size: "4MB (32Mb)"
- Partition Scheme: "Minimal SPIFFS (1.9MB APP with - OTA/190KB SPIFFS)"
- Core Debug Level: "None"
- PSRAM: "Enabled"
- Arduino Runs On: "Core 1"
- Events Run On: "Core 1"

8. Entweder:

   * die offizielle `ai-camera-firmware-v1.4.1-ota.bin` direkt mit `esptool.py` schreiben, oder
   * das GitHub-Projekt so kompilieren, wie SunFounder es für die OTA-Variante macht und das resultierende `.bin` flashen.

9. Danach kannst du künftig über `http://192.168.4.1` die Firmware als `.bin` hochladen (ohne nochmal seriell zu programmieren).


---


#  Schritt-für-Schritt-Anleitung


## 1. Hardware-Setup

**Verdrahtung FT232RL ↔ ESP32-CAM (AI-Thinker)**

Achte darauf, dass der Adapter auf **3,3 V Logikpegel** steht.
Die ESP32-CAM bekommt **5 V an 5V-Pin**, der Regler auf dem Modul macht daraus 3,3 V.

| FT232RL-Adapter | ESP32-CAM |
| --------------- | --------- |
| 5V              | 5V        |
| GND             | GND       |
| TXD             | U0R (RX0) |
| RXD             | U0T (TX0) |

Für den **Boot-Modus (Flash-Modus)**:

* GPIO0 (IO0) mit **GND brücken**
* Danach Reset / Power-On → der Chip startet im Download-Modus.
* Nach dem Flashen die Brücke IO0–GND wieder entfernen und einmal resetten.

Breadboard ist völlig ok – wichtig sind nur saubere Kontakte und gleiche Masse (GND gemeinsam).

---

## 2. Software-Voraussetzungen

1. Python / esptool:

```bash
python3 -m pip install --user esptool
```

2. PlatformIO (CLI, kommt mit VS Code-Extension automatisch):

```bash
pipx install platformio  # oder: python3 -m pip install --user platformio
```

3. Repo klonen:

```bash
cd ~/Projects           # oder dein Arbeitsordner
git clone https://github.com/sunfounder/ai-camera-firmware.git
cd ai-camera-firmware
```

---

## 3. PlatformIO-Projekt einrichten

Lege im Repo-Root eine Datei `platformio.ini` an:

```ini
[platformio]
src_dir = .

[env:esp32cam_ai_camera]
platform = espressif32 @ 6.9.0       ; Arduino-Core 2.0.17
framework = arduino
board = esp32dev
board_build.partitions = min_spiffs.csv
upload_speed = 921600
monitor_speed = 115200

build_flags =
    -DBOARD_HAS_PSRAM
    -mfix-esp32-psram-cache-issue

lib_deps =
    links2004/WebSockets@2.4.0
    bblanchon/ArduinoJson@7.4.2
```

Damit sind die Einstellungen aus der SunFounder-Doku nachgebildet (ESP32 Dev Module, Minimal SPIFFS, PSRAM an).

---

## 4. Firmware bauen (PlatformIO)

ESP32-CAM kann dabei bereits am FT232 hängen, IO0 muss für den Build selbst noch nicht gebrückt sein.

```bash
cd ~/Projects/ai-camera-firmware
pio run -e esp32cam_ai_camera
```

Wenn das durchläuft, liegen die Binaries hier:

```text
.pio/build/esp32cam_ai_camera/firmware.bin
.pio/build/esp32cam_ai_camera/bootloader.bin
.pio/build/esp32cam_ai_camera/partitions.bin
```

`boot_app0.bin` kommt aus dem Framework-Verzeichnis, z. B.:

```text
~/.platformio/packages/framework-arduinoespressif32/tools/partitions/boot_app0.bin
```

(Falls der Pfad abweicht: `find ~/.platformio -name boot_app0.bin`.)

---

## 5. Upload direkt mit PlatformIO (einfachste Variante)

1. ESP32-CAM in den **Boot-Modus** bringen:

   * IO0 ↔ GND brücken,
   * kurz USB ziehen / wieder stecken oder Reset-Taster (falls vorhanden) drücken.

2. Serielles Device suchen:

```bash
ls /dev/cu.usbserial*
# z.B. /dev/cu.usbserial-1410
```

3. Upload:

```bash
pio run -e esp32cam_ai_camera -t upload --upload-port /dev/cu.usbserial-1410
```

Wenn der Upload durch ist:

* Brücke IO0–GND entfernen,
* einmal resetten / neu mit 5 V versorgen,
* die Kamera sollte wieder als `aiCAM`-Access-Point erscheinen, Websocket auf Port 8765, MJPG-Stream unter `http://192.168.4.1:9000/mjpg`.

---

## 6. Optional: Manuell mit esptool.py flashen

Falls du die Binaries unabhängig von PlatformIO flashen willst:

1. Wieder IO0 ↔ GND brücken und Board resetten (Boot-Modus).
2. In den Build-Ordner gehen:

```bash
cd ~/Projects/ai-camera-firmware/.pio/build/esp32cam_ai_camera
```

3. Flash-Befehl (Pfad zu `boot_app0.bin` ggf. anpassen):

```bash
esptool.py --chip esp32 \
  --port /dev/cu.usbserial-1410 \
  --baud 921600 \
  --before default_reset --after hard_reset \
  write_flash -z \
    0x1000  bootloader.bin \
    0x8000  partitions.bin \
    0xe000  ~/.platformio/packages/framework-arduinoespressif32/tools/partitions/boot_app0.bin \
    0x10000 firmware.bin
```

Die Adressen `0x1000 / 0x8000 / 0xe000 / 0x10000` und die Option `write_flash` entsprechen dem Standard-Aufruf, den PlatformIO intern ebenfalls verwendet.

Danach IO0-Brücke lösen, resetten – die ESP32-CAM bootet mit der frisch geflashten SunFounder-Firmware.
