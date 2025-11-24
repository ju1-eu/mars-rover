# Briefing – Phase 2 „Mechanik, Bewegung & Mathematik“ (GalaxyRVR × STEAM)

## 1. Ziel der Phase

Die Teilnehmenden sollen

* das Rocker-Bogie-Fahrwerk und das Fahrverhalten des GalaxyRVR funktional verstehen,
* einfache Bewegungsabläufe (geradeaus, Kurve, Stopp) planen und programmieren,
* Weg–Zeit–Geschwindigkeit in einfachen Versuchen anwenden (Abschätzen und Messen),
* erste Zusammenhänge zwischen PWM-Wert, Geschwindigkeit und Fahrstrecke herstellen.

---

## 2. Rahmen

* Empfohlene Dauer: **60–90 Minuten**
* Gruppengröße: **2–4 Personen pro Rover**
* Vorkenntnisse: Phase 1 abgeschlossen (Rover bekannt, Beispiel-Sketch wurde schon geladen)

---

## 3. Materialien

* Voll montierter GalaxyRVR mit Akku
* USB-Kabel, Rechner mit Arduino-IDE und GalaxyRVR-Beispielen
* Parcoursfläche (glatter Boden, z. B. Flur, Tisch mit Begrenzung)
* Maßband oder Zollstock (mind. 2–3 m)
* Klebeband oder Markierungen (Startlinie, 1-m-Abstände, Zielbereich)
* Stoppuhr (Smartphone genügt)
* Papier/Arbeitsblatt für Messwerte

---

## 4. Zielkompetenzen (STEAM-Perspektive)

* **Science:**

  * Grundidee, dass Reibung, Untergrund und Masse das Fahrverhalten beeinflussen.
* **Technology:**

  * Motorsteuerung über Code (z. B. Funktionen für „vorwärts“, „stopp“, „Kurve“).
* **Engineering:**

  * Roverfahrten planen (Strecke, Dauer), sichere Testaufbauten bauen.
* **Mathematics:**

  * Einfache Berechnungen mit ( s ), ( t ), ( v = \frac{s}{t} ) durchführen; Messwerte tabellarisch erfassen.

---

## 5. Ablaufstruktur (Schritte)

### Schritt 1 – Mechanik und Fahrwerk beobachten (10–15 min)

Ziele:

* Rocker-Bogie-Prinzip bewusst machen.
* Verständnis: Wie verteilt sich das Gewicht? Warum bleibt der Rover stabil?

Aktivitäten:

* Rover aufbocken oder langsam rollen lassen und gezielt beobachten:

  * Wie bewegen sich die einzelnen Radarme?
  * Was passiert beim Überfahren eines kleinen Hindernisses (Holzleiste o. Ä.)?
* Kurzfragen:

  * Welche Teile drehen sich?
  * Wo sind Lager, Achsen, Hebel?

Output:

* 3–5 Stichworte, warum dieses Fahrwerk für unebenen Boden geeignet ist (z. B. „immer bodenkontakt“, „Gewichtsverteilung“, „sanftes Klettern“).

---

### Schritt 2 – Basisbewegungen programmieren (15–20 min)

Ziele:

* Rover gezielt vorwärts/rückwärts fahren und stoppen lassen.
* Kurven ansteuern (z. B. Links-/Rechtsdrehung).

Aktivitäten:

* Beispielsketch für Motorsteuerung öffnen (aus dem GalaxyRVR-Kurs zu TT-Motoren / Rover-Bewegung).
* Gemeinsam identifizieren:

  * Welche Funktion oder welcher Abschnitt setzt die Richtung?
  * Wo wird die Geschwindigkeit (PWM) eingestellt?
* Mini-Aufträge:

  * Bewegung 1: Rover fährt 1 s vorwärts und stoppt.
  * Bewegung 2: Rover dreht sich an Ort und Stelle (links/rechts) und stoppt.
  * Bewegung 3: Sequenz aus Vorwärtsfahrt – Drehung – Vorwärtsfahrt.

Output:

* Kurzer Ablaufplan in Alltagssprache („Pseudocode“):

  * z. B. „Starte, fahre 1 s vorwärts, drehe 0,5 s nach links, fahre 1 s vorwärts, stoppe.“

---

### Schritt 3 – Messversuch: Strecke und Zeit (20–25 min)

Ziele:

* Zusammenhang ( s \leftrightarrow t \leftrightarrow v ) konkret erleben.
* Abschätzen lernen und mit Messwerten vergleichen.

Aufbau:

* Gerader Korridor mit Startlinie und z. B. Markierungen bei 0,5 m, 1 m, 1,5 m, 2 m.
* Rover startet immer von der gleichen Linie.

Aktivitäten:

1. **Schätzung vor dem Versuch**

   * Festen PWM-Wert wählen (z. B. „mittlere Geschwindigkeit“).
   * Frage: „Wie lange braucht der Rover ungefähr für 1 m?“
   * Schätzung notieren (z. B. 3 s).

2. **Messung**

   * Sketch so anpassen, dass der Rover mit gewähltem PWM-Wert z. B. 2 s fährt und dann stoppt.
   * Mehrfach messen (mind. 3 Läufe), Strecke mit Maßband bestimmen.
   * Tabelle führen:

     | Versuch | Fahrdauer ( t ) [s] | Strecke ( s ) [m] | berechnete Geschwindigkeit ( v = s/t ) [m/s] |
     | ------- | ------------------- | ----------------- | -------------------------------------------- |

3. **Auswertung**

   * Durchschnittsgeschwindigkeit bestimmen.
   * Vergleichen: Weicht das stark von der ursprünglichen Schätzung ab?
   * Kurze Diskussion: Wie könnten Untergrund, Akku-Ladestand usw. das Ergebnis verändern?

Output:

* Ausgefüllte Tabelle mit mind. 3 Messungen und einem Durchschnittswert.

---

### Schritt 4 – Kleine „Route“ planen (15–20 min)

Ziele:

* Bewegungen kombinieren (Engineering).
* Mathematische Abschätzung in ein Bewegungsprogramm übersetzen.

Aktivitäten:

* Einfacher Miniparcours z. B.:

  * 1 m geradeaus,
  * 90°-Drehung nach rechts,
  * 0,5 m weiter,
  * stop + evtl. LED-Signal.
* Aus Messung aus Schritt 3 abschätzen:

  * Fahrzeit für 1 m bei gegebener ( v ).
  * Fahrzeit für 0,5 m.
* Sketch so anpassen, dass der Rover diese Route in etwa fährt (mit `delay()`- oder Zeitsteuerung).
* Test und Feinkorrektur: wenn zu kurz/lang → Dauer anpassen.

Output:

* Funktionsfähiger Parcours-Sketch (auch wenn noch nicht perfekt exakt), der die geplante Route erkennbar fährt.

---

## 6. Artefakte (Ergebnisse der Phase)

Pro Gruppe sollten am Ende vorliegen:

1. **Kurzprotokoll Mechanik**

   * 3–5 Stichworte zur Funktionsweise des Rocker-Bogie-Fahrwerks.

2. **Mess-Tabelle**

   * Mehrere Versuche mit Strecke, Zeit und berechneter Geschwindigkeit.

3. **Bewegungs-Sketch**

   * Code, der eine einfache Route (geradeaus + Drehung + weiter) ausführt.

4. **Kurze Reflexion (2–3 Sätze)**

   * z. B. „Unsere Schätzung war …, die gemessene Geschwindigkeit war …; wir haben gelernt, dass …“

---

## 7. Sozialform und Rollen

* Weiterarbeit in denselben Kleingruppen wie Phase 1.
* Rollen können bleiben oder neu verteilt werden:

  * „Mechanik/Hardware“: achtet auf sichere Tests, Parcours-Aufbau.
  * „Programmierer/in“: ändert Sketch, spielt auf Rover.
  * „Zeitnehmer/in & Messung“: führt Stoppuhr, misst Strecken.
  * „Dokumentation“: schreibt Messwerte und Erkenntnisse auf.

---

## 8. Erfolgskriterien (Checkliste)

Phase 2 ist erfolgreich, wenn:

* [ ] Jede Gruppe kann erklären, wie das Fahrwerk beim Überfahren eines Hindernisses reagiert.
* [ ] Der Rover kann kontrolliert vorwärts fahren und stoppen.
* [ ] Mindestens eine einfache Route (gerade + Drehung + weiter) ist programmierbar und demonstrierbar.
* [ ] Es existiert eine Mess-Tabelle mit Zeit-, Strecken- und Geschwindigkeitseinträgen.
* [ ] Mindestens ein Beispiel für „Schätzung vs. Messung“ wurde reflektiert.
