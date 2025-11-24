# Briefing – Phase 3 „Sensorik & autonome Navigation“ (GalaxyRVR × STEAM)

## 1. Ziel der Phase

Die Teilnehmenden sollen

* Funktionsprinzip von Ultraschallsensor (und ggf. IR-Sensor) verstehen,
* Distanzmesswerte auslesen, interpretieren und einfache Schwellen definieren,
* eine einfache Hindernisvermeidung programmieren („fahr, bis Hindernis, dann ausweichen“),
* erste Erfahrungen mit Zustandslogik (IF/ELSE, einfache Zustandsmaschine) sammeln.

---

## 2. Rahmen

* Empfohlene Dauer: **60–90 Minuten**
* Gruppengröße: **2–4 Personen pro Rover**
* Vorkenntnisse: Phase 1 und 2 abgeschlossen (Rover-Bewegung & einfache Routinen bekannt)

---

## 3. Materialien

* Voll montierter GalaxyRVR mit angeschlossenem Ultraschallsensor (und IR-Sensor, falls verfügbar)
* USB-Kabel, Rechner mit Arduino-IDE und GalaxyRVR-Beispielsketchen
* Kleine Hindernisse (z. B. Kartons, Holzklötze, Bücher)
* Teststrecke / Parcours (gerader Abschnitt mit 1–2 Hindernissen)
* Maßband (zur Kalibrierung der Sensorwerte)
* Papier/Arbeitsblatt für Mess-Tabellen und Pseudocode

---

## 4. Zielkompetenzen (STEAM-Perspektive)

* **Science:**

  * Grundidee Schalllaufzeit (Ultraschall) und Reflexion / Abstandsmessung.
* **Technology:**

  * Sensorwerte per Programm auslesen (z. B. `analogRead()` oder Bibliotheksaufrufe), seriellen Monitor verwenden.
* **Engineering:**

  * Einfache Navigationslogik entwerfen und im Parcours testen; Verhalten beobachten und verbessern.
* **Mathematics:**

  * Distanzwerte prüfen, einfache Tabellen führen, sinnvolle Schwellwerte festlegen.
* **Arts (optional):**

  * LED-Farbcodes für Abstand (z. B. Grün = frei, Gelb = Vorsicht, Rot = Stop).

---

## 5. Ablaufstruktur (Schritte)

### Schritt 1 – Einstieg & Wiederholung (5–10 min)

Ziele:

* Phase 2 kurz rekapitulieren (Bewegung, Geschwindigkeit).
* Motivation: „Jetzt soll der Rover selbst merken, wann ein Hindernis kommt.“

Aktivitäten:

* Kurzfrage an die Gruppe:

  * „Was konnte der Rover bisher?“
  * „Was müsste er zusätzlich können, um alleine durch ein Hindernisfeld zu fahren?“
* Ziel formulieren:

  * „Heute bringen wir dem Rover bei, Hindernisse zu erkennen und selbständig auszuweichen.“

---

### Schritt 2 – Sensor verstehen und testen (15–20 min)

Ziele:

* Grundprinzip des Ultraschalls erläutern.
* Sensorwerte live anzeigen.

Aktivitäten:

1. **Funktionsprinzip (einfach, bildhaft):**

   * Ultraschall-Sensor sendet einen Schallimpuls aus.
   * Schall trifft auf Hindernis und kommt zurück.
   * Aus der Zeitdifferenz berechnet das Modul die Entfernung.

2. **Test-Sketch laden:**

   * Beispielsketch für Ultraschallsensor öffnen (aus GalaxyRVR-Doku/Kurs).
   * Sensorwerte im seriellen Monitor anzeigen lassen.

3. **Beobachtung:**

   * Hindernis (Hand, Buch) verschieden weit vor den Sensor halten.
   * Veränderung der Messwerte beobachten.

Output:

* Kurze Notiz: „Wenn das Hindernis näher kommt, dann … (Messwerte werden kleiner/…?)“.

---

### Schritt 3 – Kalibrierung: Sensorwert vs. Abstand (15–20 min)

Ziele:

* Sensorwerte nicht nur „fühlen“, sondern quantifizieren.
* Einen sinnvollen Schwellwert für „zu nah“ bestimmen.

Aktivitäten:

1. **Messpunkte festlegen:**

   * z. B. 10 cm, 20 cm, 30 cm, 40 cm Abstand.

2. **Mit Maßband und seriellen Werten messen:**

   * Rover stehen lassen, Hindernis auf jeweilige Distanz positionieren.
   * Sensorwert vom seriellen Monitor ablesen (jeweils 2–3 Messungen pro Distanz).

3. **Tabelle ausfüllen:**

   | Distanz „real“ [cm] | Sensorwert 1 | Sensorwert 2 | Mittelwert |
   | ------------------- | ------------ | ------------ | ---------- |
   | 10                  |              |              |            |
   | 20                  |              |              |            |
   | 30                  |              |              |            |
   | 40                  |              |              |            |

4. **Schwellwert wählen:**

   * z. B. „Wenn der Mittelwert für 20 cm bei ≈ X liegt, setzen wir die Grenze bei X: alles darunter = Hindernis zu nah.“

Output:

* Ausgefüllte Tabelle + gewählter Schwellwert (z. B. „Stop bei <= 20 cm“).

---

### Schritt 4 – Hindernis-Logik entwerfen (10–15 min)

Ziele:

* Klare Entscheidungslogik in Alltagssprache formulieren, bevor Code geschrieben wird.
* Einfacher Einstieg in Zustandslogik.

Aktivitäten:

* Pseudocode / Flussdiagramm erstellen, z. B.:

  ```text
  Wiederhole:
    Miss Abstand mit Ultraschall
    WENN Abstand > Schwelle:
        Fahre vorwärts
    SONST:
        Stoppe
        Drehe nach links (oder rechts) für kurze Zeit
        Fahre wieder vorwärts
  ```

* Diskussion:

  * „Was passiert, wenn direkt nach der Drehung wieder ein Hindernis ist?“
  * „Welche Variante ist sicherer: kurze Drehung oder mehrfache Versuche?“

Output:

* Pseudocode oder einfache Skizze des Entscheidungsbaums.

---

### Schritt 5 – Programmierung & Parcours-Test (20–25 min)

Ziele:

* Hindernisvermeidung in Code umsetzen.
* Rover im einfachen Parcours testen und Verhalten verbessern.

Aktivitäten:

1. **Code schreiben/anpassen:**

   * Auf Basis des Pseudocodes in Arduino umsetzen:

     * Sensorwert lesen.
     * IF-Bedingung mit Schwellwert.
     * Vorwärtsfahrt / Stopp / Drehung programmieren.

2. **Testparcours:**

   * Gerade Strecke mit 1–2 Hindernissen (z. B. Kartons).
   * Rover starten und beobachten:

     * Fährt er los?
     * Stoppt er vor dem Hindernis?
     * Weicht er erkennbar aus?

3. **Korrektur & Feintuning:**

   * Schwellwert anpassen, wenn der Rover zu spät/zu früh reagiert.
   * Drehdauer oder -richtung anpassen, falls der Rover „hängen bleibt“.

Optional:

* LED-Farben einbauen:

  * Grün = frei,
  * Gelb = Vorsicht (Abstand knapp > Schwelle),
  * Rot = Stop.

Output:

* Funktionsfähiger Hindernisvermeidungssket ch (auch wenn noch nicht perfekt).

---

## 6. Artefakte (Ergebnisse der Phase)

Pro Gruppe sollten am Ende vorliegen:

1. **Sensor-Messtabelle**

   * Distanz (real) vs. Sensorwert mit einfachem Mittelwert.

2. **Dokumentierter Schwellwert**

   * z. B. „Stop bei ≤ 20 cm, gemessen als …“.

3. **Pseudocode / Flussdiagramm**

   * Klarer Entscheidungsablauf für Hindernisvermeidung.

4. **Hindernisvermeidungs-Sketch**

   * Programm, das den Rover im Testparcours auf Hindernisse reagieren lässt.

5. **Kurze Reflexion (2–3 Sätze)**

   * z. B. „Unser Sensor war bei 10 cm sehr zuverlässig, bei 40 cm schwankten die Werte stärker. Wir haben gelernt, dass der Schwellwert ein Kompromiss ist.“

---

## 7. Sozialform und Rollen

* Weiterarbeit in denselben Kleingruppen.
* Rollen können erneut vergeben werden:

  * „Sensor-Operator“: positioniert Hindernisse, misst Abstände.
  * „Programmierer/in“: setzt Pseudocode in Arduino-Code um.
  * „Beobachter/in“: achtet beim Testlauf auf Verhalten und notiert Auffälligkeiten.
  * „Dokumentation“: pflegt Tabelle, Pseudocode und Reflexion.

---

## 8. Erfolgskriterien (Checkliste)

Phase 3 ist erfolgreich, wenn:

* [ ] Jede Gruppe kann das Prinzip des Ultraschallsensors in 1–2 Sätzen erklären.
* [ ] Es existiert eine Tabelle mit Distanz- und Sensorwerten und ein begründeter Schwellwert.
* [ ] Jede Gruppe hat einen funktionierenden Sketch, der Hindernisse erkennt und darauf reagiert.
* [ ] Mindestens ein Testlauf im Parcours wurde dokumentiert (Stichworte, Foto oder Video).
* [ ] Eine kurze Reflexion zum Verhalten und zu Grenzen der Messung wurde formuliert.

