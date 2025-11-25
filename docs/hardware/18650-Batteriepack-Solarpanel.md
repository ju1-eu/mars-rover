# 1. 18650-Batteriepack (2s-Pack für Galaxy RVR)

### 1.1 Grundprinzip

- Das Pack besteht aus **zwei 18650-Li-Ion-Zellen in Serie**:
  - $V_\text{sys} \approx 2 \cdot 3{,}7\,\mathrm{V} = 7{,}4\,\mathrm{V}$

- Die Zellen hängen an einem **integrierten Lade-/Schutzboard**:
  - Zellüberwachung (Über-/Unterspannung, Überstrom),
  - Balancing über den **Middle-Pin**,
  - Übergabe der Energie an das Galaxy-RVR-Shield.
- Das Shield übernimmt:
  - Laden über **USB-C ($5\,\mathrm{V} / 2\,\mathrm{A}$)** oder **Solarport**,
  - Wandlung von Akkuspannung ($\approx 6{,}6 \dots 8{,}4\,\mathrm{V}$) auf **$5\,\mathrm{V}$-Logik**,
  - Versorgung von Motoren und Elektronik.

Für dich: Das Pack ist eine „Blackbox“-Energiequelle mit definierten Grenzen – programmierseitig siehst du vor allem die **Batteriespannung** und ggf. Ladezustandsanzeigen.

---

### 1.2 Kenndaten (für Abschätzungen & Dimensionierung)

| Parameter                        | Wert / Bereich                      | Hinweis                            |
|----------------------------------|-------------------------------------|------------------------------------|
| Zelltyp                          | 18650 Li-Ion                        | 2 Zellen in Serie                  |
| Nennspannung Pack                | $7{,}4\,\mathrm{V}$                 | $2 \times 3{,}7\,\mathrm{V}$       |
| Spannungsbereich Pack            | ca. $6{,}6 \dots 8{,}4\,\mathrm{V}$ | leer ↔ voll                        |
| Kapazität (Beispiel SunFounder)  | $2{,}0 \dots 3{,}0\,\mathrm{Ah}$    | je nach verbauten Zellen          |
| Ladespannung (am Shield)         | $5\,\mathrm{V}$ (USB-C)             | über integrierten Lade-IC          |
| Max. Ladestrom                   | $2\,\mathrm{A}$ (USB-C)             | Herstellerangabe                   |
| Ausgang (geregelt)               | $5\,\mathrm{V} / 5\,\mathrm{A}$     | $5$-V-Schiene für Logik & Motoren |
| Akkulaufzeit (Hersteller)        | ca. $90\,\mathrm{min}$              | bei typischer Rover-Nutzung        |
| Ladezeit (USB-C)                 | ca. $130\,\mathrm{min}$             | von leer nach voll                 |

---

### 1.3 Pins (3-poliger Batterie-Stecker)

Am Pack selbst (3-poliger XH-Stecker):

| Pin | Bezeichnung | Funktion                                                                 |
|-----|------------:|--------------------------------------------------------------------------|
| 1   | GND         | Negativer Batterieanschluss                                              |
| 2   | Middle      | Mittelabgriff zwischen den beiden Zellen (Balancing / Schutz)           |
| 3   | VCC         | Positiver Batterieanschluss (oft intern doppelt geführt, um Strom zu verteilen) |

Am **Battery-Port** des Galaxy-RVR-Shields liegt derselbe 3-polige Aufbau (Steckersystem kann abweichen: PH2.0 vs. XH2.54, elektrisch aber identisch).

---

### 1.4 Betriebsmodi (aus System-/Programmier-Sicht)

Elektrisch unterscheidet man drei Zustände:

1. **Entladen (Rover fährt)**
   - Energiefluss: Akku → Shield → $5$-V-Schiene → Motoren & Elektronik.
   - Die **Batteriespannung sinkt** langsam; du kannst sie per ADC messen (z. B. über einen Spannungsteiler an A3).
   - Programmierbar:
     - Spannungs-Schwellen definieren (z. B. „Low Battery“ bei $V_\text{sys} \approx 6{,}8\,\mathrm{V}$),
     - Fahrprofil anpassen (weniger Leistung, Warn-LED).

2. **Laden über USB-C oder Solar**
   - Energiefluss: USB-C / Solarpanel → Lade-IC auf dem Shield → Akku.
   - **Charge-LED (rot)** zeigt Ladevorgang, **Battery-LEDs (orange)** blinken beim Laden.
   - Aus Code-Sicht:
     - Du hast **keinen direkten Zugriff** auf den Lade-IC,
     - du kannst nur die **Batteriespannung beobachten** (steigende Tendenz) und ggf. bei Volladung bestimmte Funktionen aktivieren oder Logging stoppen.

3. **Ruhezustand / Aus**
   - Power-Schalter am Shield auf OFF → R3, ESP32, Motoren, Sensoren spannungslos.
   - Kein Programm läuft, keine Messung möglich.

Merken für später:

- Programmierseitig erfasst du nur **Spannung (und daraus $SOC$)**, keine Lade-/Entladeströme.
- Logging von Spannung vs. Zeit unter verschiedenen Lasten gibt dir reale **Reichweitenabschätzungen**.

---

## 2. Solarpanel (für Galaxy RVR)

### 2.1 Grundprinzip

- Das Panel besteht aus mehreren in Serie/parallel geschalteten Solarzellen, ausgelegt auf ca. **$6\,\mathrm{V}$ Leerlaufspannung**.
- Unter Last senkt sich die Spannung in Richtung **Ladebereich des Akku-/Lade-ICs** ab:
  - hoher Strom bei guter Einstrahlung,
  - deutlich geringerer Strom bei Bewölkung / ungünstigem Winkel.
- Das Panel wird an den **SOLAR-Port** des Shields angeschlossen; der Lade-IC nutzt es wie eine alternative Spannungsquelle zu USB-C.

Für dich: Das Solarpanel ist eine **zusätzliche Ladequelle**, die bei Sonne den Akku langsam nachlädt. Programmierseitig siehst du nur die **indirekte Wirkung** (langsamerer Spannungsabfall bzw. leichter Anstieg).

---

### 2.2 Kenndaten

| Parameter               | Wert / Bereich                | Hinweis                                      |
|------------------------|-------------------------------|---------------------------------------------|
| Nennspannung           | ca. $6\,\mathrm{V}$           | Leerlauf / optimale Last                    |
| Max. Strom             | ca. $660\,\mathrm{mA}$        | bei voller Sonne, optimaler Ausrichtung     |
| Max. Leistung          | ca. $4\,\mathrm{W}$           | grober Richtwert                            |
| Theoretische Ladedauer | ca. $7{,}2\,\mathrm{h}$       | für einen leeren 2s-Pack bei Idealbedingungen |
| Abmessungen            | $170 \times 170\,\mathrm{mm}$ | quadratisches Panel                         |
| Kabellänge             | ca. $200\,\mathrm{mm}$        | $24$ AWG, grau/schwarz                      |
| Anschluss              | XH2.54-2P                     | passend zum SOLAR-Port                      |
| Umgebung               | direkter Sonnenschein         | im Innenraum meist zu schwach               |

---

### 2.3 Pins (2-poliger SOLAR-Port)

| Pin | Bezeichnung | Funktion                                     |
|-----|------------:|----------------------------------------------|
| 1   | +           | Pluspol des Panels (graues Kabel)            |
| 2   | −           | Minuspol / GND des Panels (schwarzes Kabel)  |

Polarität ist wichtig: falscher Anschluss → der Lade-IC kann nicht laden, im schlimmsten Fall Schutzmaßnahmen.

---

### 2.4 Betriebsmodi (praktisch / programmierbezogen)

Aus Sicht des Systems:

1. **Ohne Sonne / zu wenig Licht**
   - Panel liefert nur sehr kleinen Strom → Effekt auf Akku praktisch Null.
   - Lade-LED bleibt aus (wenn USB-C nicht steckt).
   - Programmierseitig:
     - der Akku entlädt sich wie ohne Solarpanel,
     - du kannst die Spannungskurve als „Baseline“ für Vergleichstests nutzen.

2. **Teils Sonne (bewölkt, schräger Winkel)**
   - Panel liefert **wenig, aber nicht Null**:
     - Entladung des Akkus wird verlangsamt,
     - evtl. leicht steigende Spannung im Stillstand (ohne Motorlast).
   - Programmierseitig:
     - Fahrversuch mit/ohne Panel vergleichen (Spannung vs. Zeit),
     - so kannst du den **Netto-Gewinn** abschätzen.

3. **Volle Sonne / gute Ausrichtung**
   - Panel liefert bis zu ca. $660\,\mathrm{mA}$:
     - Akku kann im Stand geladen werden,
     - während langsamer Fahrt kann ein Teil des Motorstroms aus dem Panel kommen.
   - Die Charge-LED am Shield verhält sich wie beim Laden über USB-C.
   - Programmierseitig:
     - Akkuspannung kann trotz Nutzung **steigen oder konstant bleiben**,
     - du kannst das als „Solar-Bonus-Modus“ loggen (z. B. Flag in Telemetrie).

4. **Kombinierte Quellen (USB-C + Solar)**
   - Vorrang-/Summierungslogik hängt vom Lade-IC ab (Shield-Blackbox).
   - Typisch: der höhere Spannungspegel wird bevorzugt; bei $5\,\mathrm{V}$-USB dominiert USB.

Merken für später:

- Es gibt **keinen direkten Sensor „Solarstrom“** – du siehst nur den **Effekt** auf die Akkukurve.
- Für Auswertungen kannst du z. B.:
  - Batterieversuche mit/ohne Panel fahren,
  - Fahrprofil, Startspannung und Dauer loggen,
  - Unterschiede in Spannungsabfall und Laufzeit analysieren.

---

## 18650-Batteriepack – technische Daten

| Parameter               | Wert                               | Kommentar                                           |
|-------------------------|------------------------------------|-----------------------------------------------------|
| Zelltyp                 | 18650 Li-Ion                       | Spezielles SunFounder-Pack mit 2 Zellen in Serie   |
| Nennspannung je Zelle   | $3{,}7\,\mathrm{V}$                | typische Li-Ion-Nennspannung                       |
| Packkonfiguration       | $2 \times 3{,}7\,\mathrm{V}$       | $V_\text{sys} \approx 7{,}4\,\mathrm{V}$ ungepuffert |
| Kapazität               | $2{,}0\,\mathrm{Ah} \times 2$      | Gesamtenergie abhängig von Last und Wirkungsgraden |
| Ladeeingang             | $5\,\mathrm{V} / 2\,\mathrm{A}$    | über USB-C am Galaxy-RVR-Shield                    |
| Batterieausgang         | $5\,\mathrm{V} / 5\,\mathrm{A}$    | vom Shield aufbereitete Ausgangsspannung           |
| Batterielebensdauer     | ca. $90\,\mathrm{min}$             | Herstellerangabe bei typischer Nutzung             |
| Ladezeit                | ca. $130\,\mathrm{min}$            | Herstellerangabe                                   |
| Anschluss               | XH2.54-3P                          | VCC / Middle / GND                                 |

### 18650-Batteriepack – Pinbelegung XH2.54-3P

| Pin   | Bezeichnung | Funktion                                                                 |
|-------|-------------|--------------------------------------------------------------------------|
| 1     | GND         | Negativer Batterieanschluss                                              |
| 2     | Middle      | Mittelabgriff zwischen den beiden Zellen (Spannungsausgleich / Schutz)  |
| 3     | VCC         | Positiver Batterieanschluss (zweifach ausgeführt zur Stromverteilung)   |

---

## Solarpanel – technische Daten

| Parameter                    | Wert                         | Kommentar                                      |
|-----------------------------|------------------------------|-----------------------------------------------|
| Ausgangsspannung            | $6\,\mathrm{V}$              | Leerlauf / Nennspannung                       |
| Ausgangsstrom (max.)        | $660\,\mathrm{mA}$           | bei starker Sonneneinstrahlung                |
| Theoretische Ladedauer Akku | ca. $7{,}2\,\mathrm{h}$      | Angabe für Vollladung des 2s-Packs            |
| Abmessungen                 | $170 \times 170\,\mathrm{mm}$| Quadratpanel                                  |
| Kabellänge                  | $200\,\mathrm{mm}$           | $24$ AWG, Farben: grau und schwarz            |
| Stecker                     | XH2.54-2P (weiß)             | passend zum „SOLAR“-Port des Galaxy-RVR-Shields |
