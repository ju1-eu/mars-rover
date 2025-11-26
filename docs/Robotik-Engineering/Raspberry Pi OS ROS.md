# Raspberry Pi 5 - OS und ROS

Empfehlung nach Industriestandard:
**OS:** **Ubuntu Server 24.04 LTS (64-bit)**
**ROS-Version:** **ROS 2 Jazzy Jalisco**

---

### Warum genau diese Kombination?

#### 1. Warum Ubuntu und nicht "Raspberry Pi OS"?
Zwar ist *Raspberry Pi OS* (basierend auf Debian) das "Haus-System" des Pi, aber **Ubuntu** ist das "Haus-System" von ROS.
* **Support:** Die ROS-Entwickler (Open Robotics) entwickeln primär auf Ubuntu. Die Pakete (`sudo apt install ros-jazzy-...`) sind für Ubuntu optimiert.
* **Einfachheit:** Auf anderen Systemen musst du ROS oft mühsam aus dem Quellcode kompilieren (dauert Stunden). Auf Ubuntu installierst du es in 5 Minuten per Binaries.
* **Pi 5 Support:** Ubuntu 24.04 unterstützt den Raspberry Pi 5 mittlerweile nativ und hervorragend.

#### 2. Warum "Server" und nicht "Desktop"?
Du baust einen autonomen Roboter, keinen Büro-PC.
* **Ressourcen:** Die Desktop-Version (mit grafischer Oberfläche/GUI) frisst ca. 1-2 GB RAM und 10-20% CPU nur für das Anzeigen von Fenstern. Diese Leistung brauchst du für SLAM (Mapping) und Navigation.
* **Headless-Betrieb:** Ein Profi-Roboter hat keinen Monitor. Du greifst per SSH (WLAN) oder VS Code Remote auf ihn zu.
* **Tipp:** Wenn du Visualisierung brauchst (z. B. Rviz, um die Karte zu sehen), installierst du ROS 2 auf deinem Laptop und verbindest ihn über das Netzwerk mit dem Roboter. Der Roboter rechnet, der Laptop zeigt an.

#### 3. Warum ROS 2 Jazzy (und nicht Humble)?
* **Humble Hawksbill:** War lange der Standard (läuft auf Ubuntu 22.04). Ist extrem stabil und hat die meisten Tutorials. **Aber:** Ubuntu 22.04 auf dem Pi 5 ist fummelig (alter Kernel).
* **Jazzy Jalisco:** Ist die aktuelle **LTS (Long Term Support)** Version (veröffentlicht Mai 2024, Support bis 2029). Sie läuft nativ auf dem modernen Ubuntu 24.04, das perfekt mit dem Pi 5 harmoniert.

---

### Die Kompatibilitäts-Matrix
ROS-Versionen sind strikt an OS-Versionen gekoppelt. Mischmasch funktioniert nicht gut.

| OS Version | ROS 2 Version | Status | Empfehlung für Pi 5 |
| :--- | :--- | :--- | :--- |
| **Ubuntu 24.04 (Noble)** | **Jazzy Jalisco** | LTS bis 2029 | **⭐ Beste Wahl** |
| Ubuntu 22.04 (Jammy) | Humble Hawksbill | LTS bis 2027 | Geht, aber Kernel-Updates nötig |
| Debian 12 (Bookworm) | Jazzy Jalisco | Tier 2 Support | Nur für Experten |

---

### Profi-Workflow: Wie installierst du das?:

1.  **Raspberry Pi Imager:** Lade das Tool auf deinen PC.
2.  **OS wählen:** Wähle unter "Other General-Purpose OS" -> "Ubuntu" -> **"Ubuntu Server 24.04 LTS (64-bit)"**.
3.  **Wichtig - Settings (Zahnrad):**
    * Setze hier schon den Hostnamen (z.B. `rover-brain`).
    * Aktiviere SSH.
    * Setze Benutzername und Passwort.
    * Konfiguriere dein WLAN.
4.  **Flashen:** Auf die SD-Karte schreiben.
5.  **Starten:** Karte in den Pi, Strom an. Nach 2 Minuten kannst du dich vom PC aus einloggen: `ssh user@rover-brain.local`.

### Ein Wort zu Docker (Für später)
Echte Profis installieren ROS oft gar nicht mehr direkt auf dem OS ("Bare Metal"), sondern nutzen **Docker Container**.
* *Vorteil:* Du kannst dein System nicht "zerschießen". Wenn du Mist baust, löschst du den Container und startest neu.
* *Nachteil:* Etwas steilere Lernkurve am Anfang (Hardware-Zugriff auf GPIO/USB durch den Container hindurch).
