# Arduino - Esp32 - Raspberry Pi

Im professionellen Kontext – also dort, wo es um Geld, Sicherheit und Skalierbarkeit geht – ordnen wir diese drei Plattformen sehr spezifisch ein. Sie sind längst nicht mehr nur "Spielzeug", aber sie erfüllen völlig andere Rollen als im Hobby-Keller.


### 1. Die fundamentale Unterscheidung: Gehirn vs. Reflexe
Bevor wir die Geräte einzeln betrachten, müssen wir verstehen, dass sie in der Robotik zwei unterschiedliche Nervensysteme bedienen:

* **Reflexe (Low-Level Control):** Harte Echtzeit. Ein Motor muss *jetzt* stoppen, nicht in 50 Millisekunden, wenn das Betriebssystem fertig ist. Das ist das Gebiet der **Mikrocontroller (Arduino, ESP32)**.
* **Gehirn (High-Level Control):** Berechnung. "Ist das vor mir eine Katze oder ein Karton?" oder "Plane eine Route durch die Lagerhalle". Das braucht Rechenpower und viel Speicher. Das ist das Gebiet der **Single Board Computer (Raspberry Pi)**.

---

### 2. Die Einordnung im Detail

#### **A. Arduino (Der zuverlässige Arbeiter)**
* **Status:** Früher reines Lehrmittel, heute mit der **"Arduino Pro"**-Reihe (z. B. Portenta H7, Opta PLC) auch industriell relevant.
* **Profi-Einsatz:**
    * Er wird als **Embedded Controller** genutzt, um Motortreiber direkt anzusteuern oder Sensordaten vorzufiltern.
    * **Beispiel:** Ein Arduino liest 1000-mal pro Sekunde einen Kraftsensor am Greifer aus und stoppt den Motor bei Überlast. Er schickt dann nur das fertige Signal "Greifer voll" an den Hauptcomputer.
    * **Industrie-Hardware:** Es gibt inzwischen Arduinos in DIN-Schienen-Gehäusen (z.B. Arduino Opta), die wie eine echte SPS (Speicherprogrammierbare Steuerung) funktionieren.

#### **B. ESP32 (Der IoT-Kommunikator)**
* **Status:** Der unangefochtene König der Preis-Leistung im Bereich "Connected Things". Ein Mikrocontroller mit integriertem WLAN/Bluetooth und zwei Kernen.
* **Profi-Einsatz:**
    * Er ist oft **unsichtbar**. In professionellen Produkten (z. B. Smart-Sensoren, WLAN-Steckdosen, Asset-Trackern) steckt oft ein ESP32-Chip, ohne dass "ESP32" draufsteht.
    * Er wird für **Edge-Computing** genutzt: Er sammelt Vibrationsdaten einer Maschine, wertet sie lokal aus (läuft die Maschine rund?) und sendet nur bei Problemen einen Alarm in die Cloud (MQTT).
    * Er ist aber *nicht* die erste Wahl für komplexe Bewegungssteuerung von großen Roboterarmen, da seine Echtzeitfähigkeit durch den WLAN-Stack (Netzwerkverkehr) beeinträchtigt werden kann.

#### **C. Raspberry Pi (Das kleine Gehirn / Gateway)**
* **Status:** Ein vollwertiger Linux-PC. Im Profi-Bereich nutzt man selten den "Bastel-Pi" (Modell B) mit SD-Karte, sondern das **Compute Module (CM4/CM5)**. Das ist der Pi ohne Anschlüsse, reduziert auf eine Platine, die man in robuste Industrie-Hardware steckt.
* **Profi-Einsatz:**
    * **ROS 2 Host:** Er ist der Standard-Computer für mobile Roboter (AMRs), um Navigation (SLAM) und Bildverarbeitung zu berechnen.
    * **IIoT-Gateway:** Er übersetzt zwischen alten Maschinen (die "SPS-Sprache" sprechen) und modernen IT-Systemen (Cloud/Datenbanken).
    * **Industrie-Hardware:** Produkte wie der **Revolution Pi** oder Steuerungen von WAGO basieren oft intern auf der Raspberry-Pi-Technologie, sind aber gehärtet (24V Stromversorgung, robustes Gehäuse, eMMC-Speicher statt wackeliger SD-Karte).

### 3. Die harte Grenze zur "echten" Industrie
Warum sieht man in einer VW-Fertigungsstraße trotzdem meist **Siemens, Beckhoff oder KUKA** Steuerungen und keine Raspberries?

1.  **Safety (Sicherheit):** Ein Raspberry Pi oder ESP32 ist (standardmäßig) nicht "Safety-zertifiziert" (SIL / PL Level). Wenn ein Roboter einen Menschen verletzen könnte, *darf* die Sicherheitselektronik gesetzlich nicht auf einem Standard-Pi laufen.
2.  **Echtzeit & Determinismus:** Linux (auf dem Pi) ist kein Echtzeit-Betriebssystem. Wenn Windows oder Linux ein Update machen oder hängen, steht die Maschine. Eine industrielle SPS läuft deterministisch (garantierte Reaktion in x Millisekunden).
3.  **Haftung & Lebensdauer:** Industrie-Komponenten werden garantiert 10-20 Jahre lang identisch nachproduziert. Beim Consumer-Markt ändert sich alle 2 Jahre der Chip.

### Zusammenfassung für dich
* **Arduino:** Dein Werkzeug für **harte Echtzeit** und direkte Hardware-Kontrolle (Motoren/Sensoren).
* **ESP32:** Dein Werkzeug für **drahtlose Sensoren** und IoT-Anwendungen (Daten senden).
* **Raspberry Pi:** Dein Werkzeug für **Intelligenz** (Kameras, Webserver, ROS, Datenbanken).

In einem professionellen Roboter arbeiten sie oft zusammen: Der **Pi** sagt "Fahr nach vorne", der **Arduino/ESP32** steuert daraufhin die Motoren und achtet darauf, dass nichts überhitzt.
