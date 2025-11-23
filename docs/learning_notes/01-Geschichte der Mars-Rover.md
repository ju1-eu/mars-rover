# Cornell-Notizen: Einführung & Geschichte der Mars-Rover

**Quelle:** SunFounder Galaxy RVR Dokumentation – Lektion 1
**Thema:** Historie der NASA-Rover & Einführung in das Rocker-Bogie-System

| Fragen / Schlüsselbegriffe | Notizen & Details |
| :--- | :--- |
| **Warum Mars-Erkundung?** | • **Ziel:** Suche nach Biosignaturen (mikrobielles Leben, vergangen oder gegenwärtig).<br>• **Vergleich:** Der Mars ist der erdähnlichste Planet im Sonnensystem.<br>• **Zukunft:** Vorbereitung technischer und physiologischer Grundlagen für bemannte Missionen. |
| **Die NASA-Rover<br>(Evolution)** | **1. Sojourner (1997)**<br>• *Größe:* Mikrowelle.<br>• *Mission:* Pathfinder.<br>• *Erfolg:* Proof-of-Concept (lief 83 Tage); bewies Mobilität auf fremden Planeten.<br><br>**2. Spirit & Opportunity (2004)**<br>• *Größe:* Golfwagen.<br>• *Mission:* "Roboter-Geologen".<br>• *Erfolg:* Nachweis von vergangenem Wasser. Liefen Jahre über die geplante Dauer hinaus.<br><br>**3. Curiosity (2012)**<br>• *Größe:* Kleinwagen (SUV).<br>• *Antrieb:* Radioisotopengenerator (RTG), unabhängig von Sonnenlicht.<br>• *Feature:* "Sky Crane" Landung & mobiles Chemielabor.<br><br>**4. Perseverance (2021)**<br>• *Upgrade:* Basiert auf der Curiosity-Plattform.<br>• *Besonderheit:* Mars Sample Return (Gesteinsproben) & Helikopter *Ingenuity*. |
| **Das Rocker-Bogie-System** | **Definition:** Passives Aufhängungssystem ohne Federn, Standard für alle NASA-Rover.<br><br>**Aufbau:**<br>1. *Rocker:* Hauptwippe (größerer Teil).<br>2. *Bogie:* Drehgestell mit Rädern (kleinerer Teil).<br>3. *Differential:* Verbindet beide Seiten mechanisch.<br><br>**Physik & Vorteile:**<br>• **Kletterfähigkeit:** Überwindet Hindernisse der Höhe $h_{obs}$:<br>$$h_{obs} \le 2 \cdot d_{wheel}$$<br>(Wobei $d_{wheel}$ der Raddurchmesser ist).<br>• **Bodenkontakt:** Gewährleistet durch die Geometrie, dass alle $n=6$ Räder permanenten Bodenkontakt haben.<br>• **Stabilität:** Minimiert die Neigung des Rumpfes ("Body") um den Faktor 0.5 im Vergleich zur Rad-Auslenkung, was für stabile Kameraaufnahmen essenziell ist. |
| **Der Galaxy RVR** | • **Konzept:** Bildungs-Kit zur Replikation der NASA-Technologie.<br>• **Specs:**<br> - Antrieb: 6 DC-Motoren (6WD).<br> - Reifen: Weiche Gummimischung für Traktion.<br> - Sicht: ESP32 CAM für FPV (First Person View).<br> - Sensoren: Ultraschall ($d = \frac{t \cdot c}{2}$) & IR-Sensoren.<br> - Energie: Hybrid (Akku + Solar-Demonstration). |

## Zusammenfassung

Diese Lektion verknüpft die historische Evolution der NASA-Mars-Rover – vom kleinen *Sojourner* bis zum komplexen *Perseverance* – mit den ingenieurtechnischen Anforderungen planetarer Erkundung. Das Kernproblem unwegsamen Geländes wird durch das **Rocker-Bogie-System** gelöst. Dieses federnlose Fahrwerk nutzt ein Differentialgetriebe und eine spezielle Geometrie, um Hindernisse bis zum doppelten Raddurchmesser ($2 \cdot d_{wheel}$) zu überwinden, während der Rumpf stabil bleibt. Der **SunFounder Galaxy RVR** adaptiert dieses Prinzip im Maßstab 1:1 für Bildungszwecke, um ein authentisches Verständnis für Robotikmechanik zu vermitteln.

