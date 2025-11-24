# Briefing – Phase 4 „Medien, Energie & Präsentation“ (GalaxyRVR × STEAM)

## 1. Ziel der Phase

Die Teilnehmenden sollen

* das Energiesystem des GalaxyRVR (Akku, ggf. Solarpanel) grob verstehen,
* eine einfache Laufzeitabschätzung für ihre Mission vornehmen,
* LEDs, Servo und ggf. Kamera gezielt als „Feedback“ und „Story-Elemente“ einsetzen,
* ihre Mission mit dem Rover einmal komplett durchspielen und dokumentieren,
* eine kurze Abschlusspräsentation vorbereiten (Poster, Slides oder Video).

---

## 2. Rahmen

* Empfohlene Dauer: **60–90 Minuten**
* Gruppengröße: **2–4 Personen pro Rover**
* Vorkenntnisse: Phasen 1–3 abgeschlossen (Mission definiert, Bewegung + Hindernisvermeidung vorhanden)

---

## 3. Materialien

* Voll montierter GalaxyRVR mit Akku (geladen), ggf. Solarpanel
* USB-Kabel, Rechner mit Arduino-IDE
* Zugriff auf die GalaxyRVR-Dokumentation (Angaben zum Akku / Stromverbrauch)
* Testparcours aus den vorigen Phasen (Missionenumgebung)
* Kamera (ESP32-CAM oder externe Kamera/Smartphone für Videoaufnahmen)
* Material für Präsentation: Papier, Stifte, ggf. PC/Beamer für Slides

---

## 4. Zielkompetenzen (STEAM-Perspektive)

* **Science:**

  * Grundidee „Energieinhalt des Akkus“ und Energieverbrauch des Rovers.
* **Technology:**

  * LEDs, Servo, Kamera als Ausgabemedien und Statusanzeigen im Programm nutzen.
* **Engineering:**

  * Mission zuverlässig und reproduzierbar ablaufen lassen; Lastfälle bedenken (Viele Starts, Dauerbetrieb, Hindernisse).
* **Arts:**

  * Rover-Mission verständlich und ansprechend visualisieren (Licht, Bild, Video, Story).
* **Mathematics:**

  * Sehr grobe Energiebilanz/Laufzeitabschätzung mit einfachen Zahlen.

---

## 5. Ablaufstruktur (Schritte)

### Schritt 1 – Energiesystem verstehen (10–15 min)

Ziele:

* Klarheit, welche Energiequelle(n) vorhanden sind.
* Begriff „Energie ist begrenzt“ auf das Projekt anwenden.

Aktivitäten:

* Kurzer Blick in die Doku:

  * Akkutyp (z. B. 2×18650 mit bestimmter Kapazität in mAh),
  * Hinweis auf Solarpanel (falls genutzt).
* Gemeinsame Fragen:

  * „Wie lange soll der Rover am Stück ungefähr fahren?“
  * „Was braucht mehr Energie – Standby oder dauerhafte Fahrt?“

Output:

* 3–4 Stichpunkte zum Energiesystem: Akku-Aufbau, ungefähre Kapazität, Rolle des Solarpanels (Laden, nicht „unendlich Energie“).

---

### Schritt 2 – Laufzeit grob abschätzen (Mathematik) (15–20 min)

Ziele:

* Sehr einfache Überschlagsrechnung zur Missionslaufzeit.
* Bewusstsein: Zahlen ergeben nur eine grobe Orientierung.

Aktivitäten:

* Annahmen treffen (vereinfachen!):

  * „Unser Rover zieht im Fahrmodus ungefähr I_Fahr (z. B. 0,5–1 A)“.
  * „Unsere Mission dauert pro Durchlauf vielleicht 2–3 Minuten.“
* Rechnungsidee:

  * Kapazität in mAh in mögliche Betriebszeit umrechnen (z. B. „Wenn 2000 mAh und wir ziehen in etwa 1000 mA, dann ca. 2 h bei Dauerlast – aber unsere Mission ist viel kürzer und die Last schwankt.“).
* Ergebnis notieren:

  * „Wir schätzen, dass wir X–Y Missionsläufe machen können, bevor der Akku deutlich leer ist.“

Output:

* Kurzer Abschnitt im Protokoll: Annahmen + geschätzte Anzahl Missionsläufe.

---

### Schritt 3 – Medien- und Feedback-Design (20–25 min)

Ziele:

* LEDs, Servo und Kamera sinnvoll in die Story einbauen.
* Klar erkennbare Zustände definieren.

Aktivitäten:

1. **Zustände definieren (Beispiel):**

   * Zustand 1: „Startbereit im Basislager“ → LED z. B. blau.
   * Zustand 2: „Unterwegs, alles ok“ → LED grün.
   * Zustand 3: „Hindernis erkannt / langsam“ → LED gelb.
   * Zustand 4: „Ziel erreicht“ → LED pulsierend oder Farbwechsel.

2. **Servo + Kamera (optional):**

   * Beim Start: Servo „scannt“ den Horizont (kurze links-rechts-Bewegung).
   * Am Ziel: Kamera-Foto oder kurzer Videoclip des Zielbereichs.

3. **Programm-Anpassung:**

   * In den vorhandenen Bewegungs-/Sensor-Sketch aus Phase 3 LED- und Servo-Befehle an passenden Stellen einbauen.

Output:

* Klar definierte Zustände mit zugehörigen Medienaktionen.
* Aktualisierter Sketch, der Bewegung, Sensorik und Medien kombiniert.

---

### Schritt 4 – Missions-Durchlauf und Dokumentation (15–20 min)

Ziele:

* Mission einmal „am Stück“ laufen lassen.
* Beobachten, ob Story und Technik zusammenpassen.

Aktivitäten:

* Parcours wie in Phase 1 definiert („Eisfeld“, Krater etc.) aufbauen.
* Rover starten und kompletten Missionslauf durchführen:

  * Start → Fahrt → Hindernisvermeidung → Ziel → LED-/Kamera-Aktion.
* Mit Smartphone oder Kamera den Lauf filmen.
* Kurz notieren:

  * „Was hat gut funktioniert?“
  * „Wo gab es Probleme (z. B. falsche Schwellwerte, Wackelkontakt, Rutschuntergrund)?“

Output:

* 1 Missionsvideo (oder Foto-Serie) pro Gruppe.
* 4–5 Stichworte zu Beobachtungen und Verbesserungsmöglichkeiten.

---

### Schritt 5 – Abschlusspräsentation vorbereiten (10–15 min)

Ziele:

* Ergebnisse und Lernwege sichtbar machen.
* STEAM-Bezug der Mission verdeutlichen.

Aktivitäten:

* Für jede Gruppe:

  * Kurzes **Poster** oder **2–3 Folien** erstellen:

    * Missionstitel + Ziel
    * Bild/Foto vom Rover im Parcours
    * 1–2 Stichworte zum Energiesystem + grobe Laufzeitschätzung
    * 3–5 Stichworte zu Erkenntnissen / Schwierigkeiten
    * 1 Satz zu jedem STEAM-Bereich (Wie wurde er in der Mission sichtbar?)
* Optional: 1–2 Minuten mündliche Präsentation vor der Gruppe.

Output:

* Fertiges Poster oder Folien-Set, das in einer kurzen Präsentation nutzbar ist.

---

## 6. Artefakte (Ergebnisse der Phase)

Pro Gruppe sollten am Ende vorliegen:

1. **Kurzprotokoll Energiesystem & Laufzeitabschätzung**

   * Annahmen + sehr grobe Berechnung.

2. **Aktualisierter Missions-Sketch**

   * Bewegung + Hindernisvermeidung + LED/Servo/Kamera-Feedback.

3. **Missionsdokumentation**

   * Video oder Fotos vom vollständigen Missionslauf.

4. **Präsentationsmaterial**

   * Poster oder Folien mit Mission, Technik, Energie, Learnings und STEAM-Bezug.

---

## 7. Sozialform und Rollen

* Weiterarbeit in denselben Kleingruppen; Rollen können spezifiziert werden:

  * „Energy & Numbers“: kümmert sich um Abschätzung und schreibt Zahlen auf.
  * „Coder/in“: integriert LED/Servo/Kamera in den Sketch.
  * „Media & Story“: filmt, achtet auf Story und visuelle Wirkung.
  * „Speaker“: übernimmt 1–2 Minuten Präsentation.

---

## 8. Erfolgskriterien (Checkliste)

Phase 4 ist erfolgreich, wenn:

* [ ] Jede Gruppe kann in 1–2 Sätzen erklären, wie ihr Rover mit Energie versorgt wird.
* [ ] Es existiert eine nachvollziehbare, wenn auch grobe Laufzeitabschätzung.
* [ ] Bewegung, Sensorik und Medien (mindestens LEDs) sind im Missions-Sketch kombiniert.
* [ ] Die Mission wurde mindestens einmal komplett im Parcours gefahren und dokumentiert.
* [ ] Jede Gruppe hat ein einfaches Präsentationsmedium (Poster/Folien) mit klarem STEAM-Bezug erstellt.

