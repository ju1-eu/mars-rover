# autonomen mobilen Roboter (AMR)

Wir nutzen hier das **Master-Slave-Prinzip**, um die Stärken von Raspberry Pi (Intelligence) und ESP32/Arduino (Real-Time) zu kombinieren.

Dieser Aufbau ist der Industriestandard für Service-Roboter, Logistik-Shuttles und Forschungsplattformen.

### Die Architektur-Übersicht



Wir teilen den Roboter in zwei Gehirnhälften:
1.  **High-Level Control (Raspberry Pi):** Das "strategische Gehirn". Es plant, sieht und entscheidet.
2.  **Low-Level Control (ESP32/Arduino):** Das "Rückenmark". Es führt aus, regelt und schützt.

---

### Schicht 1: Der "Low-Level Controller" (ESP32 oder Arduino Portenta)
Hier läuft **kein Linux**, sondern Firmware (C++ / FreeRTOS).

* **Aufgabe:** Harte Echtzeit-Kontrolle der Hardware.
* **Zykluszeit:** 1ms bis 10ms (100 Hz - 1000 Hz Loop).
* **Was passiert hier?**
    * **PID-Regelung:** Der Pi sendet den Befehl "Fahre 0.5 m/s". Der ESP32 misst via Encoder die Raddrehzahl und passt die Spannung (PWM) an die Motoren tausendmal pro Sekunde an, damit die Geschwindigkeit auch bergauf gehalten wird.
    * **Odometrie-Berechnung:** Er zählt die Radumdrehungen und berechnet die relative Position ($x, y, \theta$), um dem Pi zu sagen: "Ich habe mich 10 cm bewegt."
    * **Safety Watchdog:** Wenn der Pi abstürzt und 500ms lang kein Befehl kommt -> **Notstopp** der Motoren.

### Schicht 2: Die Schnittstelle (Die Kommunikation)
Wie reden die beiden miteinander? Auf Profi-Level nutzen wir oft **micro-ROS** oder ein robustes serielles Protokoll.

* **Physisch:** USB-Kabel (Serial over USB) oder UART (GPIO zu GPIO).
* **Logisch (Protokoll):**
    * Der Pi sendet `cmd_vel` (Command Velocity): Eine Nachricht, die lineare (x) und angulare (z) Geschwindigkeit vorgibt.
    * Der ESP32 sendet `odom` (Odometrie) und `imu_data` (Gyroskop/Beschleunigung) zurück an den Pi.

### Schicht 3: Der "High-Level Computer" (Raspberry Pi 4/5 oder CM4)
Hier läuft **Ubuntu Server** mit **ROS 2** (Robot Operating System).

* **Aufgabe:** Wahrnehmung und Planung.
* **Zykluszeit:** 10ms bis 100ms.
* **Was passiert hier?**
    * **Lidar / Kamera:** Der Pi liest den Laserscanner aus.
    * **SLAM (Simultaneous Localization and Mapping):** Der Pi nimmt die Laserdaten und die Odometrie vom ESP32, um eine Karte des Raumes zu bauen und sich darin zu verorten (z. B. mit `Nav2` Stack).
    * **Pfadplanung:** Du sagst "Fahre in die Küche". Der Pi berechnet den Weg um das Sofa herum und sendet einfache Geschwindigkeitsbefehle an den ESP32.

---

### Zusammenfassung des Datenflusses

Hier ist der komplette Loop, wenn der Roboter einem Hindernis ausweicht:

1.  **Sensor (Lidar):** Sieht das Hindernis -> Daten an **Raspberry Pi**.
2.  **Planung (Pi):** Navigations-Software erkennt "Weg blockiert", berechnet neue Kurve -> Sendet `cmd_vel: linear 0.2, angular 0.5` an **ESP32**.
3.  **Kommunikation:** Daten laufen über USB/Serial.
4.  **Regelung (ESP32):** Empfängt Zielgeschwindigkeit. Merkt "linkes Rad muss schneller drehen als rechtes".
5.  **Aktor (Motor):** ESP32 gibt Strom auf die Motortreiber.
6.  **Feedback:** Encoder messen echte Drehung -> ESP32 korrigiert Abweichung -> Meldet Position an **Pi** zurück.

### Profi-Tipp: Warum scheitern hier Anfänger?
Anfänger lassen oft den Raspberry Pi *alles* machen (auch PWM für Motoren). Das Problem: Sobald der Pi kurz "nachdenkt" (z. B. Bildverarbeitung oder WLAN-Suche), stockt das Signal an den Motor für Millisekunden. Der Roboter ruckelt oder fährt ungenau.
**Die Regel:** Trenne *immer* "Denken" (Pi) und "Muskeln" (ESP32).
