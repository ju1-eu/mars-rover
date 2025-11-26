# Kamera

Beim Raspberry Pi 5 hat sich der Anschluss für Kameras geändert, und das AI Kit (M.2 HAT+, Hailo-8L Beschleuniger) verdeckt mechanisch den Zugriff.

Damit dein Roboter-Setup ("Profi-Level") funktioniert, musst du auf **zwei Dinge** achten: Den **Kameratyp** (für Robotik) und das **Kabel** (für den Pi 5).


### 1. Die "Profi-Wahl" für bewegte Roboter: Raspberry Pi Global Shutter Camera
Wenn dein Roboter fährt und gleichzeitig Bilder für die KI (Hailo) analysieren soll, ist das deine erste Wahl.

* **Warum?** Standard-Kameras haben einen "Rolling Shutter" (sie lesen das Bild zeilenweise aus). Wenn der Roboter wackelt oder fährt, wird das Bild verzerrt (gerade Linien werden schief). Das verwirrt die KI.
* **Global Shutter:** Diese Kamera macht einen Schnappschuss des gesamten Bildes gleichzeitig. Keine Verzerrung, selbst bei hoher Geschwindigkeit.
* **Auflösung:** 1.6 Megapixel (klingt wenig, ist aber perfekt für KI, da Modelle wie YOLO ohnehin Bilder auf z.B. 640x640 Pixel runterskalieren).
* **Preis:** ca. 60 €.

### 2. Der "Allrounder" (Preis/Leistung): Raspberry Pi Camera Module 3
Die Standard-Wahl, wenn du auch mal hochauflösende Fotos machen willst.

* **Warum?** Sie hat Autofocus und HDR (High Dynamic Range), was bei schwierigem Licht (Schatten unter dem Tisch vs. helles Fenster) hilft.
* **Nachteil:** Rolling Shutter. Bei schneller Fahrt können Bilder leicht verzerren.
* **Variante:** Nimm unbedingt die **"Wide" (Weitwinkel)** Version (120 Grad Sichtfeld). Ein Roboter muss sehen, was *neben* ihm passiert, nicht nur einen Tunnelblick nach vorne haben.
* **Preis:** ca. 30 €.

---

### ⚠️ Das Wichtigste: Das Kabel-Problem beim Pi 5

Der Raspberry Pi 5 nutzt einen **kleineren Kamera-Anschluss** (CSI Mini) als der Pi 4. Das Kabel, das bei den Kameras in der Schachtel liegt, **passt nicht**.

1.  **Du musst kaufen:** Ein **"Raspberry Pi 5 Camera Cable"** (Mini zu Standard).
    * Länge: Nimm mindestens **30cm oder 50cm**. Da die Kamera am Rover vielleicht oben am Mast sitzt und der Pi unten im Chassis, sind die Standard-Kurzkabel oft zu knapp.
2.  **Die Montage mit dem AI Kit:**
    * Das AI Kit (der M.2 HAT mit dem Hailo Chip) wird *auf* den Pi geschraubt.
    * Die Kamera-Anschlüsse sitzen *darunter*.
    * **Reihenfolge:** Du musst **zuerst** das Kamera-Kabel in den Pi 5 stecken, **dann** das AI Kit darüber montieren und das Kabel durch den Schlitz im HAT führen. Nachträglich kommst du da mit den Fingern nicht mehr dran!



### Software-Integration (Hailo & Kamera)

Das Schöne am offiziellen **Raspberry Pi AI Kit** ist die Integration. Du nutzt die ganz normale Kamerasoftware (`rpicam-apps`).

Die Pipeline sieht so aus (und ist im offiziellen OS vorinstalliert):
1.  Kamera nimmt Bild auf (via `libcamerasrc`).
2.  Der Videostrom wird *automatisch* an den Hailo-Chip geleitet.
3.  Hailo führt YOLOv8 (Objekterkennung) aus.
4.  Das Ergebnis (Bounding Boxes um Hindernisse) kommt zurück zum Pi.
5.  Das alles passiert mit fast 0% CPU-Last für den Pi.

### Meine Empfehlung für deinen Einkaufszettel

1.  **Kamera:** **Raspberry Pi Global Shutter Camera** (bestes Futter für die KI bei Bewegung).
2.  **Kabel:** **Raspberry Pi 5 Camera Cable - 500mm** (lieber zu lang als zu kurz beim Roboterbau).
3.  **Halterung:** Ein einfaches Acryl-Case oder 3D-Druck-Teil für die Kamera, das man auf das Alu-Chassis schrauben kann.


---

## Halterung

**Pan/Tilt-Head** (Schwenk-Neige-Kopf).

Da du bereits das **AI Kit** (M.2 HAT+) auf dem Raspberry Pi 5 hast, rate ich dir dringend von sogenannten "Pan-Tilt HATs" ab, die man auf den Pi steckt. Das würde zu einem thermischen und mechanischen Chaos führen (Lüfter blockiert, kein Platz).

### 1. Die Hardware: Das "2-DOF Pan/Tilt Kit"
Du suchst nach einem Standard-Bausatz aus schwarzem Kunststoff oder Aluminium, der durch zwei kleine Servomotoren bewegt wird.

* **Der Begriff:** Suche nach **"Pan Tilt Servo Bracket Kit"** oder **"2 DOF PTZ Halterung"**.
* **Die Servos (Wichtig!):**
    * In den billigen Sets sind oft die blauen **SG90** Servos (Plastikgetriebe) dabei. **Nimm diese nicht.** Sie sind ungenau ("Jitter") und brechen schnell.
    * Kaufe das Kit leer oder mit **MG90S** Servos. Das "MG" steht für **Metal Gear**. Die sind robust, präzise und halten das Gewicht der Global Shutter Kamera stabil.



### 2. Das Problem & Die Lösung (Kamera-Montage)
Die meisten dieser günstigen Halterungen sind für quadratische Platinenkameras gedacht. Die Raspberry Pi Kameras sind aber rechteckige Riegel.

* **Die Bastel-Lösung:** Starkes doppelseitiges Montageband (Tesa Powerbond). Hält bombenfest auf der flachen Fläche des Neige-Servos.
* **Die Profi-Lösung (3D Druck):** Wenn du oder ein Bekannter einen 3D-Drucker hat, drucke einen "Raspberry Pi Camera Mount to Servo Adapter". Es gibt hunderte Vorlagen auf *Thingiverse*.
* **Der Adapter-Kauf:** Es gibt Acryl-Adapterplatten ("Mounting Plate for Raspberry Pi Camera"), die man auf den Servo schrauben kann.

### 3. Wie du es auf das Alu-Chassis schraubst
Die Pan/Tilt-Kits haben unten eine flache Basis (den unteren Servo).
* **Befestigung:** Du musst meistens **zwei kleine Löcher (3mm)** in dein Alu-Chassis bohren, die passend zu den Löchern des unteren Servos sind.
* **Position:** Montiere den Turm möglichst weit **vorne** oder **hoch oben** (auf einer Distanzhülse/Abstandshalter), damit der Roboter nicht nur sein eigenes Chassis sieht, wenn er nach unten schaut.

---

### 4. Die Verkabelung (Architektur-Entscheidung)

Wo schließt du die Servos an? Hier machen viele den Fehler, sie an den Pi zu hängen.

* **Nicht an den Pi 5:** Der Pi hat kein gutes Hardware-PWM an den Pins, und du willst keine Jumper-Kabel quer über das empfindliche AI-Kit spannen.
* **An den ESP32 (Empfohlen):**
    * Du hast den ESP32 bereits für die Motoren. Er ist ein Meister des PWM (Pulsweitenmodulation).
    * Schließe die Signalkabel der Servos (Orange/Gelb) an zwei freie GPIO-Pins des ESP32 an.
    * **Stromversorgung:** Die Servos brauchen 5V. Ziehe diese **NICHT** vom 5V-Pin des ESP32 (der brennt durch). Ziehe sie direkt vom 5V-Ausgang deines Step-Down-Wandlers (den du für den Pi hast).

### 5. Warum ist das "Profi-Level"? (ROS 2 Integration)

In der Universität und Forschung ist ein beweglicher Kopf nicht nur Spielerei, sondern Teil der **Kinematischen Kette (TF Tree)**.

Wenn du in ROS 2 den Kopf nach links drehst, passiert Folgendes:
1.  Der Node sendet Winkel an den ESP32 -> Servo dreht.
2.  Gleichzeitig veröffentlicht ROS den neuen Zustand (`joint_states`).
3.  Der **Robot State Publisher** berechnet: "Die Kamera schaut jetzt nach links."
4.  Wenn die KI (Hailo) jetzt einen Ball erkennt, weiß ROS 2: "Der Ball ist nicht vor dem Roboter, sondern links neben ihm", und trägt ihn an der korrekten Stelle in die Karte ein.

**Zusammenfassung Einkaufsliste Halterung:**
1.  **"Pan Tilt Camera Mount Bracket Kit"** (ohne Servos kaufen, kostet ca. 5-8 €).
2.  **2x MG90S Servos** (Metallgetriebe, ca. 5-6 € pro Stück).
3.  **Montagematerial:** M3 Schrauben/Muttern + Bohrer für das Alu-Chassis.
