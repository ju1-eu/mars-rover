# Checkliste – Energiestudie Galaxy RVR / „Mars-Rover“

Ziel: Systematisch untersuchen, wie sich **Betriebsmodus**, **PWM-Einstellungen** und **Umgebung (Innen/außen, Sonne)** auf den **Energieverbrauch** und die **Laufzeit** auswirken.

---

## 1. Versuchsplanung

- [ ] Versuchsziel klar formulieren
  - [ ] z. B. „Einfluss von Cruise-Speed (≈ 45 % PWM) vs. Vollgas auf SOC-Verlauf“
  - [ ] z. B. „Einfluss von RGB-Dimmung (30 %) auf Akkulaufzeit bei gleicher Fahrstrecke“

- [ ] Versuchsvarianten festlegen
  - [ ] Variante A: Innenraum, **ohne Solarpanel**, Cruise + 30 %-RGB
  - [ ] Variante B: Innenraum, **ohne Solarpanel**, Vollgas + 100 %-RGB
  - [ ] Variante C: Außen, **mit Solarpanel**, Cruise + 30 %-RGB
  - [ ] Variante D: Außen, **mit Solarpanel**, Vollgas + 100 %-RGB

- [ ] Rahmenbedingungen definieren
  - [ ] Fahrprofil: „Dauerfahrt geradeaus“, „Runden auf festem Kurs“, o. Ä.
  - [ ] Testdauer pro Lauf: z. B. \(t = 30\,\mathrm{min}\)
  - [ ] Pausen zwischen Läufen: z. B. \(t_\text{Pause} = 10\,\mathrm{min}\)

---

## 2. Vorbereitung der Hardware

- [ ] Akku vollständig laden
  - [ ] Ladekabel, LED-Status am Battery-Pack prüfen
  - [ ] Startspannung messen: \(U_\text{Bat, Start}\)
  - [ ] Start-SOC aus Code (`batteryGetPercentage()`): \(SOC_\text{Start}\,\% \)

- [ ] Solarpanel vorbereiten (für Außenversuche)
  - [ ] Mechanisch am Rover befestigt
  - [ ] Kabelverbindung zum Power-Modul geprüft
  - [ ] Neigungswinkel und Ausrichtung zur Sonne notiert

- [ ] Konfiguration dokumentieren
  - [ ] `Config::SpeedMax = ...`
  - [ ] Cruise-Faktor: \(f_\text{Cruise} \approx 0{,}45\) → `TEST_SPEED ≈ 0.45 * SpeedMax`
  - [ ] RGB-Brightness: \(\approx 30\,\%\) (SoftPWM-Wert)
  - [ ] Sonstige Verbraucher: Kamera an/aus, Servo-Test an/aus

---

## 3. Messplan pro Versuchslauf

Für **jeden Lauf** (A, B, C, D) folgende Schritte:

- [ ] Lauf-ID und Rahmenbedingungen notieren

  | Feld               | Wert                          |
  |--------------------|-------------------------------|
  | Lauf-ID            | A / B / C / D                 |
  | Ort                | Innen / Außen                 |
  | Licht/Sonne        | Bewölkt / Sonne / Schatten    |
  | Modus Motor        | Cruise / Vollgas              |
  | Modus RGB          | 30 % / 100 %                  |
  | Kamera             | an / aus                      |
  | Dauer              | z. B. 30 min                  |

- [ ] **Vor Start** messen/loggen
  - [ ] \(t_0\): Zeitstempel (Start)
  - [ ] \(U_\text{Bat}(t_0)\) – Batteriespannung
  - [ ] \(SOC(t_0)\,\%\) – Ladezustand aus `batteryGetPercentage()`

- [ ] **Während des Laufs** (z. B. alle 5 min)
  - [ ] Zeitstempel \(t_i\)
  - [ ] \(SOC(t_i)\,\%\)
  - [ ] optionale Zusatzinfos: Motorgeräusch, Temperaturgefühl, Verhalten in der Sonne

  Beispiel-Tabelle:

  | Zeit \(t\) [min] | \(SOC(t)\) [%] | Bemerkung                 |
  |------------------|----------------|---------------------------|
  | 0                |                | Start                     |
  | 5                |                |                           |
  | 10               |                |                           |
  | 15               |                |                           |
  | 20               |                |                           |
  | 25               |                |                           |
  | 30               |                | Ende Lauf                 |

- [ ] **Nach Ende**
  - [ ] \(U_\text{Bat}(t_\text{End})\)
  - [ ] \(SOC(t_\text{End})\,\%\)
  - [ ] Subjektive Beobachtungen (z. B. „Rover deutlich langsamer“, „Panel im Schatten“)

---

## 4. Vergleichs- und Auswertungsschritte

Für jede Variante (A–D):

- [ ] \(\Delta SOC = SOC(t_0) - SOC(t_\text{End})\) berechnen
- [ ] SOC-Verbrauch pro Zeiteinheit:

  \[
  \dot{SOC} = \frac{\Delta SOC}{t_\text{Lauf}}\;[\%/\mathrm{min}]
  \]

- [ ] Zwischen den Varianten vergleichen
  - [ ] Unterschied Cruise vs. Vollgas (bei gleicher Umgebung)
  - [ ] Unterschied mit vs. ohne Solarpanel (bei gleicher Fahrweise)
  - [ ] Effekt der RGB-Dimmung (30 % vs. 100 %)

Beispiel-Vergleichstabelle:

| Lauf | Ort   | Panel | Modus Motor | Modus RGB | \(\Delta SOC\) [%] | \(t\) [min] | \(\dot{SOC}\) [%/min] |
|------|-------|-------|-------------|-----------|--------------------|-------------|------------------------|
| A    | Innen | aus   | Cruise      | 30 %      |                    |             |                        |
| B    | Innen | aus   | Vollgas     | 100 %     |                    |             |                        |
| C    | Außen | an    | Cruise      | 30 %      |                    |             |                        |
| D    | Außen | an    | Vollgas     | 100 %     |                    |             |                        |

---

## 5. Dokumentation & Fazit

- [ ] Kurzprotokoll schreiben
  - [ ] Welche Konfiguration ist „energetisch sinnvoll“ für **lange Laufzeit**?
  - [ ] Wie stark kompensiert das Solarpanel den Verbrauch im Cruise-Modus?
  - [ ] Ab welcher Last (Vollgas + volle RGB + Kamera) „verheizt“ der Rover die Energie deutlich schneller?

- [ ] To-Do-Liste aus der Studie ableiten
  - [ ] „Eco-Modus“ im Code (Cruise + 30 %-RGB + Kamera-Limit)
  - [ ] „Performance-Modus“ mit Warnung, wenn SOC < \(x\,\%\)
  - [ ] Optional: automatische Geschwindigkeitsreduzierung, wenn SOC unter Schwelle fällt

