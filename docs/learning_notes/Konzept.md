# Was ist STEAM?

* STEAM steht für **Science, Technology, Engineering, Arts, Mathematics**.
* Es ist kein einzelnes Fach, sondern ein **interdisziplinärer Ansatz**: echte Probleme werden so bearbeitet, dass Naturwissenschaft, Technik, Ingenieurdenken, Gestaltung/Kunst und Mathematik miteinander verknüpft werden.
* Ziel: **Problemlösen, Kreativität, kritisches Denken, Teamarbeit** – nicht nur Formeln lernen, sondern Dinge bauen, testen, verbessern.

Im deutschsprachigen Raum ist STEAM die „MINT-Erweiterung“: zu Mathematik, Informatik, Naturwissenschaft und Technik kommt explizit der kreative Anteil (Design, Medien, Storytelling, Kunst).

---

## Brücke: STEAM × GalaxyRVR

Aus der Dokumentation zum GalaxyRVR-Kit:
Du hast einen Mars-Rover mit Rocker-Bogie-Fahrwerk, TT-Motoren, Ultraschall- und IR-Sensoren, Kamera (ESP32-CAM), RGB-LED-Streifen, Solarpanel und Akku. Dazu gibt es 13 Arduino-Lektionen (Mechanik, Motoren, Sensorik, Kamera, Energie etc.).

Das lässt sich direkt auf STEAM mappen:

| Bereich         | GalaxyRVR-Bezug                                                                                      |
| --------------- | ---------------------------------------------------------------------------------------------------- |
| **Science**     | Physik der Bewegung (Reibung, Steigung), Sensorprinzipien (Ultraschall, IR), Energie (Akku, Solar).  |
| **Technology**  | Arduino-IDE, R3-Board, ESP32-CAM, WLAN/App-Steuerung, Firmware, Libraries.                           |
| **Engineering** | Rocker-Bogie-Mechanik, Aufbau und Verschraubung, Kabelführung, Energiepfad, Fehlersuche.             |
| **Arts**        | Design des Rovers (Optik, LED-Effekte), Mission-Story, Video/Foto mit Kamera, Interface-Gestaltung.  |
| **Mathematics** | Weg-Zeit-Berechnungen, Drehzahl, Übersetzungen, Energie- und Laufzeitabschätzungen, Sensordistanzen. |

Die vorhandenen Lektionen aus der Doku kannst du wie „Bausteine“ für ein STEAM-Projekt verwenden:

* L1–L3: Kennenlernen von Rover, Rocker-Bogie und Arduino-Grundlagen
* L4–L5: Motorsteuerung und Fahrverhalten
* L6–L8: Hindernisvermeidung und Ultraschallnavigation
* L9–L11: LEDs, Servo, Kamera
* L13: Energiesystem (Solarpanel, Akku)

---

## Konzept: Marsmission GalaxyRVR als STEAM-Projekt

Ziel: Ein **zusammenhängendes Projekt**, bei dem der Rover eine selbst entwickelte Mission fährt (z. B. „Proben sammeln im Krater“), und dabei alle fünf STEAM-Bereiche berührt.

Rahmen (Vorschlag):

* Dauer: ca. **8–12 Einheiten à 60–90 min**
* Setting: Schul-AG, Projektwoche oder Selbstlernprojekt
* Grundstruktur: **4 Phasen**, die auf den SunFounder-Lektionen aufbauen

---

### Phase 1 – Story & System verstehen (S + A + T)

**Lernziele**

* Wissen, was STEAM ist und wie der Rover aufgebaut ist.
* Eine einfache **Mars-Story** für die spätere Mission entwickeln.
* Arduino-IDE öffnen, Beispielsketch hochladen (aus L3).

**Beispielaktivitäten**

* Kurzinput: Was ist STEAM, wozu Rover-Projekte?
* Doku „About this Kit“ lesen und Komponenten identifizieren (R3-Board, Shield, ESP32-CAM, Sensoren, Akku, Solar).
* In Kleingruppen eine **Mission definieren**:

  * Zielgebiet (z. B. Krater, Eishöhle).
  * Was der Rover dort tun soll (hinfahren, Hindernis umgehen, LED-Signal geben, Foto machen).
* Einfache Skizze des Rovers und der Umgebung anfertigen (A = Design / Zeichnen).

**Produkte**

* 1-seitiges Missions-Briefing (Text + Skizze).
* Erster lauffähiger Demo-Sketch (z. B. Blink-LED oder einfache Motorbewegung aus L3/L4).

---

### Phase 2 – Mechanik, Bewegung & Mathematik (E + M + S)

**Lernziele**

* Rocker-Bogie-System und Fahrverhalten verstehen (L2, L4, L5).
* Grundlegende **Weg-Zeit-Geschwindigkeit**-Zusammenhänge anwenden.
* Fahrmanöver planen und testen.

**Beispielaktivitäten**

* Nach Anleitung L2: Rocker-Bogie und Chassis aufbauen, dabei Engineering-Begriffe einführen (Drehpunkt, Hebelarm, Schwerpunkt).
* In L4/L5 die **Motorsteuerung** programmieren (vorwärts, rückwärts, Kurven, Drehen auf der Stelle).
* Mathe-Aufgabe:

  * Rover fährt bei einem bestimmten PWM-Wert z. B. ( v \approx 0{,}2,\mathrm{m/s} ).
  * Mission verlangt ( 3,\mathrm{m} ) Fahrt – Abschätzen der Fahrzeit:
    ( t = \frac{s}{v} = \frac{3,\mathrm{m}}{0{,}2,\mathrm{m/s}} = 15,\mathrm{s} ).
  * Im Versuch stoppen und mit Realität vergleichen.

**Produkte**

* Kurze Tabelle „PWM-Wert ↔ geschätzte Geschwindigkeit ↔ gemessene Zeit“.
* Testparcours mit Markierungen (z. B. 1 m-Abstände), dokumentierte Ergebnisse (Fotos, Notizen).

---

### Phase 3 – Sensorik & autonome Navigation (S + T + E + M)

**Lernziele**

* Funktionsprinzip **Ultraschall** und **IR-Hindernisvermeidung** erklären (L6–L8).
* Distanzwerte interpretieren und Schwellen sinnvoll wählen.
* Einfache autonome Mission programmieren (z. B. Hindernis umgehen, Linie folgen).

**Beispielaktivitäten**

* Doku zu Ultraschallmodul und IR-Modul lesen, einfache Skizze: „Wie misst Ultraschall die Entfernung?“.
* Arduino-Sketch aus L6/L7/L8 übernehmen und **gezielt anpassen**:

  * Abstands-Schwellwert ändern.
  * Verhalten variieren: bei <20 cm anhalten, bei 20–40 cm langsamer werden, sonst Vollgas.
* Mathe/Physik:

  * Sensorwerte loggen (z. B. 10 Messungen auf bekannte Distanzen).
  * Differenz „Sensorwert – realer Abstand“ berechnen → Diskussion von Messfehlern.

**Produkte**

* Kurze Dokumentation (Tabelle + Diagramm) zur Sensor-Genauigkeit.
* Funktionierender Sketch: Rover fährt selbständig durch einfachen Labyrinth-Parcours.

---

### Phase 4 – Medien, Energie & Präsentation (A + S + T + M)

**Lernziele**

* Energiesystem verstehen (Lektion 13: Akku, Solarpanel, Laufzeit).
* Kamera, Servo und RGB-LEDs kreativ einsetzen (L9–L11).
* Ergebnisse ansprechend darstellen (Video, Plakat, Slides).

**Beispielaktivitäten**

* L9/L10/L11:

  * LED-Farben nutzen, um Missionszustände zu signalisieren (z. B. Grün = unterwegs, Blau = Daten senden, Rot = Fehler).
  * Servo + Kamera als „Kopf“: Rover scannt die Umgebung und macht Fotos/Videos.
* L13: einfache **Energiebilanz**:

  * Akku-Daten aus Doku (z. B. ( 2 \times 2000,\mathrm{mAh} ) 18650, Nennspannung) entnehmen.
  * Stromaufnahme in verschiedenen Modi (Standby, Langsamfahrt, Vollgas) messen oder aus Datenblatt schätzen.
  * Überschlägig Laufzeit berechnen und mit Praxis testen.
* Medien/Arts:

  * Missionsvideo aus Sicht der Kamera schneiden.
  * Infoposter oder kurze Präsentation erstellen: Mission, Technik, Daten, Learnings.

**Produkte**

* Präsentation oder Poster mit:

  * Missionsbeschreibung
  * Foto/Video vom Rover im Einsatz
  * Diagramm zur Laufzeit / Energie
  * Hervorhebung aller fünf STEAM-Bereiche
* Optional: Kurzer „Technikbericht“ als Markdown/LaTeX oder Handout.

---

## 4. STEAM-Fokusfragen für den GalaxyRVR

Für jede Phase kannst du dir (oder der Lerngruppe) gezielt STEAM-Fragen stellen:

* **Science:**
  „Welche physikalischen Effekte bestimmen, ob der Rover eine Steigung schafft?“
* **Technology:**
  „Wie kommunizieren R3-Board, Shield und ESP32-CAM miteinander, und was macht die Software genau?“
* **Engineering:**
  „Welche Änderungen am Fahrwerk oder an der Masseverteilung würden das Klettern verbessern?“
* **Arts:**
  „Wie können LEDs, Kameraperspektive und Story den Rover wie eine echte Marsmission wirken lassen?“
* **Mathematics:**
  „Welche Formeln brauche ich, um Geschwindigkeit, Energieverbrauch und Fahrzeit abzuschätzen?“

