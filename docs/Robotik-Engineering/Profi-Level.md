# Roboterprogrammierung auf "Profi-Level"

Der **Profi-Level** (auch Robotik-Engineering genannt) ist ein fundamentaler Shift von der *Bedienung* zur *Softwarearchitektur*. Es geht darum, Roboter nicht als isolierte Maschinen, sondern als intelligente, datengetriebene Agenten in einem IT-Ökosystem zu verstehen.

### 1\. Die zwei Welten der Profi-Programmierung

Auf Expertenniveau spaltet sich die Welt in zwei Hauptströmungen. Ein Profi muss meist eine beherrschen und die andere verstehen:

#### A. Die "klassische" Industrie-Automatisierung (Hard Real-Time)

Hier zählt Millisekunden-Präzision, 24/7-Stabilität und Sicherheit.

  * **Sprachen:** Nicht Python, sondern **IEC 61131-3** (SPS-Standards wie Strukturierter Text) oder herstellerspezifische Hochsprachen wie **KRL** (KUKA), **RAPID** (ABB) oder **KAREL** (Fanuc).
  * **Profi-Skill:** Du schreibst keinen Code "am Roboter". Du nutzt **Offline-Programmierung (OLP)**. Du simulierst die gesamte Zelle (in Tools wie Siemens Process Simulate, Fanuc ROBOGUIDE), validierst Kollisionen und Taktzeiten digital (Digitaler Zwilling) und spielst den Code erst dann auf die echte Hardware.
  * **Warum?** Weil Stillstandzeit in einer Automobilstraße 10.000 € pro Minute kosten kann. Man "probiert" nicht an der echten Maschine.

#### B. Die moderne "Advanced Robotics" (ROS & Middleware)

Hier geht es um Flexibilität, Kameras, KI und Autonomie (z. B. Logistik-Roboter, Start-ups).

  * **Der Standard:** **ROS 2** (Robot Operating System). Das ist kein Betriebssystem, sondern eine Middleware, die es erlaubt, dass ein Laserscanner (Lidar) mit dem Fahrgestell und dem Greifarm "spricht".
  * **Sprachen:** **C++** (für Treiber, harte Echtzeit und Performance) und **Python** (für die Logik, KI-Anbindung und Skripte).
  * **Profi-Skill:** Du programmierst keine fixen Pfade, sondern **Verhalten**. Beispiel: "Fahre zu Kiste X" (egal wo sie steht, dank Navigation Stack) und "Greife Objekt Y" (egal wie es liegt, dank Computer Vision).

### 2\. Der "Tech Stack" eines Experten (Was du können musst)

| Ebene | Was der Laie sieht | Was der Profi macht (Backend) |
| :--- | :--- | :--- |
| **Logik** | "Der Roboter bewegt sich." | **State Machines** & **Behavior Trees**: Komplexe Entscheidungsbäume, die Fehler abfangen (z. B. "Greifer leer -\> Versuche es erneut oder rufe Hilfe"). |
| **Bewegung** | "Er fährt von A nach B." | **Pfadplanung & Kinematik**: Nutzung von Algorithmen (wie RRT\* oder OMPL), um kollisionsfreie Pfade in Echtzeit zu berechnen (Inverse Kinematik). |
| **Sicherheit** | "Er stoppt, wenn ich komme." | **Safety-Integration (ISO 10218 / ISO/TS 15066)**: Konfiguration von Sicherheitszonen auf Steuerungsebene (PROFIsafe, CIP Safety). Das ist rechtlich bindend. |
| **DevOps** | (Unsichtbar) | **CI/CD für Roboter**: Automatisierte Tests in der Cloud/Simulation (Docker, Jenkins), bevor Software auf eine Flotte von 100 Robotern ausgerollt wird. |

### 3\. Aktuelle Trends & Standards (2024/2025)

  * **Sim2Real:** Das Training von KI-Modellen in der Simulation (z. B. NVIDIA Isaac Sim), die dann auf den echten Roboter übertragen werden ("Zero-Shot Transfer").
  * **No-Code / Low-Code:** Profis bauen die komplexen Backends, damit der Endanwender später nur noch grafische Blöcke schieben muss (Demokratisierung der Robotik).
  * **Herstellerunabhängigkeit:** Der Trend geht weg von proprietären Steuerungen hin zu offenen Standards, wo eine SPS (z. B. via CODESYS) den Roboter direkt steuert, ohne dessen eigenen Controller nutzen zu müssen.

### Fazit

Roboter programmieren auf Profi-Level heißt heute: **Du bist weniger Mechaniker, sondern Software-Architect.** Du löst Probleme der Integration (Sensorik, Safety, IT-Security) und der Skalierbarkeit.

Möchtest du tiefer in die **ROS 2 Architektur** einsteigen oder interessiert dich eher der **industrielle OLP-Workflow** (z. B. für Automobilfertigung)?

[Getting Started with ROS 2](https://www.google.com/search?q=https://www.youtube.com/watch%3Fv%3D4vv3L7XzB6k)
Dieses Video ist relevant, da es einen praktischen Einstieg in ROS 2 bietet, was, wie oben beschrieben, der aktuelle Industriestandard für flexible und moderne Robotik-Entwicklung ist.
