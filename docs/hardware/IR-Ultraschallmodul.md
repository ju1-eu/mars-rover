# IR-Hindernisvermeidungsmodul

### 1. Grundprinzip

* Eine **IR-LED** sendet moduliertes Infrarotlicht (ca. 38 kHz) nach vorne.
* Eine **Fotodiode / IR-Empfänger** misst das zurückgeworfene Licht.
* Ein Komparator wertet das Signal gegen eine **einstellbare Schwelle** aus.
* Ergebnis ist ein **digitales Signal**:

  * Kein Hindernis → Ausgang HIGH
  * Hindernis im eingestellten Bereich → Ausgang LOW

Je heller / reflektierender das Objekt und je näher es ist, desto früher fällt das Signal ab.

---

### 2. Kenndaten

| Parameter                 | Wert / Bereich           | Hinweis                                 |
| ------------------------- | ------------------------ | --------------------------------------- |
| Versorgungsspannung       | 3,3 V … 5 V DC           | direkt an 5 V des Shields betreibbar    |
| Stromaufnahme             | wenige 10 mA             | inkl. IR-LED                            |
| Ausgang                   | digital HIGH/LOW         | open-collector/TTL, bereits aufbereitet |
| Distanzbereich            | ca. 2 cm … 40 cm         | abhängig von Reflektivität des Objekts  |
| Erfassungswinkel          | ca. 35°                  | Öffnungswinkel des Sensors              |
| Einstellschwelle          | über 2 Potentiometer     | Empfindlichkeit + Verstärkung           |
| Trägerfrequenz IR         | ~38 kHz                  | passend zum Empfängerfilter             |
| Temperaturbereich Betrieb | −10 °C … +50 °C          |                                         |
| Schnittstelle             | 4-Draht (- / + / S / EN) | GND, VCC, OUT, EN                       |

---

### 3. Pins

| Pin | Bezeichnung | Funktion                                              |
| --- | ----------: | ----------------------------------------------------- |
| 1   |         GND | Masse                                                 |
| 2   |           + | Versorgung 3,3–5 V DC                                 |
| 3   |     OUT / S | Digitaler Ausgang: HIGH = frei, LOW = Hindernis       |
| 4   |          EN | Enable-Pin: GND = dauerhaft aktiv, HIGH = deaktiviert |

---

### 4. Betriebsmodi (aus Sicht des Programms)

* **Immer an (Standard)**
  EN auf GND → Modul arbeitet permanent, OUT zeigt direkt Hindernis an.

* **Software-gesteuert aktivieren**
  EN an einen Mikrocontroller-Pin legen:

  * EN LOW → Modul misst.
  * EN HIGH → IR-LED und Auswertung aus, Energiesparen möglich.

* **Logik im Code**

  * Simple Nutzung: `digitalRead(OUT)` → `true/false` für Hindernis.
  * Für robustes Verhalten: Mehrfach lesen und mitteln/entprellen (z. B. 3–5 Messungen, Mehrheitsentscheid).

Merken für später:

* Du musst **keine Analogsachen** auswerten, nur ein digitales HIGH/LOW.
* Die Reichweite stellst du **mechanisch am Modul** (Potis) ein, nicht im Code.

---

# Ultraschallmodul HC-SR04

### 1. Grundprinzip

* Eine **Ultraschall-Sendeeinheit** sendet einen kurzen Burst (typ. 8 Perioden bei 40 kHz).
* Schall breitet sich mit etwa (c \approx 343,\mathrm{m/s}) aus.
* Die **Empfangseinheit** registriert das Echo eines Hindernisses.
* Der Sensor gibt am **ECHO-Pin einen HIGH-Puls** aus; dessen Dauer entspricht der Hin- und Rücklaufzeit:
  [
  d = \frac{c \cdot t}{2}
  ]
  mit (d) = Distanz, (t) = Pulsdauer.

Der Mikrocontroller triggert die Messung und misst anschließend die Pulsbreite des ECHO-Signals.

---

### 2. Kenndaten

| Parameter               | Wert / Bereich          | Hinweis                                  |
| ----------------------- | ----------------------- | ---------------------------------------- |
| Versorgungsspannung     | 5 V DC                  | stabil, wenig Ripple                     |
| Stromaufnahme           | ≈ 16 mA                 | während Messung                          |
| Arbeitsfrequenz Schall  | 40 kHz                  | Ultraschall                              |
| Messbereich             | ca. 2 cm … 400 cm       | bei typischen Bedingungen                |
| Auflösung / Genauigkeit | ≈ 3 mm                  | abhängig von Temperatur/Luft             |
| Trigger-Eingangssignal  | 10 µs TTL-Puls          | steigende Flanke startet Messung         |
| Echo-Ausgangssignal     | TTL HIGH-Puls (bis 5 V) | Pulsbreite ∝ Entfernung                  |
| Empfohlene Messrate     | ≤ 40 Hz                 | mind. ~20–60 ms Pause zwischen Messungen |
| Steckverbinder          | XH2.54-4P               | passend zum SONAR-Port am Shield         |

---

### 3. Pins

| Pin | Bezeichnung | Funktion                                                      |
| --- | ----------: | ------------------------------------------------------------- |
| 1   |         VCC | +5 V Versorgung                                               |
| 2   |        TRIG | Trigger-Eingang: 10-µs-Puls vom Mikrocontroller               |
| 3   |        ECHO | Echo-Ausgang: HIGH-Puls mit Dauer proportional zur Entfernung |
| 4   |         GND | Masse                                                         |

Hinweis für 3,3-V-Controller: ECHO ist 5-V-TTL; bei reinen 3,3-V-MCUs wäre ein Level-Shifter nötig. Beim 5-V-R3 ist das unkritisch.

---

### 4. Betriebsmodi (aus Sicht des Programms)

Der HC-SR04 selbst hat keine komplexen Power-Modi; die „Modi“ entstehen durch dein Ansteuern:

1. **Idle**

   * TRIG = LOW, ECHO = LOW.
   * Keine Messung aktiv, nur Ruhestrom.

2. **Einzelmessung**

   * Im Code:

     1. Evtl. kurze Ruhezeit abwarten (z. B. 10–20 ms seit der letzten Messung).
     2. TRIG für mind. 10 µs HIGH setzen, dann wieder LOW.
     3. Auf steigende und fallende Flanke von ECHO warten, Pulsdauer (t) in µs messen.
     4. Distanz berechnen:
        [
        d[\text{cm}] \approx \frac{t[\mu s]}{58}
        ]
   * Gut für gezielte Abstandsmessungen (z. B. alle 50 ms).

3. **Periodische Messung („Radar-Mode“)**

   * Der Mikrocontroller triggert in einem festen Intervall (z. B. alle 50–100 ms).
   * Distanzwerte werden fortlaufend geloggt / gefiltert (Moving Average, Median, etc.).

4. **Energiesparen**

   * Nur über Abschalten der **Versorgung** (z. B. mit MOSFET/Transistor oder geschaltetem 5-V-Rail).
   * Danach muss vor der nächsten Messung eine kurze Einschwingzeit berücksichtigt werden.

Merken für später:

* Du programmierst eigentlich nur zwei Aktionen: **kurzen TRIG-Puls erzeugen** und **ECHO-Pulsbreite messen**.
* Die Distanzformel kannst du als Inline-Funktion oder Makro kapseln.
* Für stabile Werte lohnt sich ein **Filter** (z. B. Mittelwert aus 3–5 Messungen).

---

## IR-Hindernisvermeidungsmodul – technische Daten

| Parameter                       | Wert                          | Kommentar                                              |
|---------------------------------|-------------------------------|--------------------------------------------------------|
| Betriebs­spannung               | 3,3 V … 5 V DC                | direkt an 5 V des Galaxy-RVR-Shields betreibbar       |
| Ausgang                         | digital (HIGH/LOW)           | LOW bei erkanntem Hindernis                           |
| Distanzbereich                  | ca. 2 cm … 40 cm             | abhängig von Reflektivität des Objekts                |
| Einstellschwelle                | einstellbar                   | zwei Potentiometer (Empfindlichkeit / Verstärkung)    |
| Trägerfrequenz IR-LED (R5)      | 38 kHz (voreingestellt)       | optimiert für Empfängerfilter                         |
| IR-Tastverhältnis (R6)          | voreingestellt                | bestimmt Puls-Paket / Helligkeit der IR-LED           |
| Effektiver Erfassungswinkel     | ca. 35°                       | Öffnungswinkel Sensor                                 |
| Betriebstemperatur              | −10 °C … +50 °C               | laut Datenblatt                                       |
| I/O-Schnittstelle               | 4-Draht (- / + / S / EN)      | GND, VCC, OUT, EN                                     |
| Abmessungen                     | 45 × 16 × 10 mm               | L × B × H                                             |
| Gewicht                         | 9 g                           |                                                       |

### IR-Hindernisvermeidungsmodul – Pinbelegung

| Pin | Bezeichnung | Funktion                                                            |
|-----|------------:|---------------------------------------------------------------------|
| 1   | GND         | Masse / Bezugspotential                                             |
| 2   | +           | Versorgung 3,3 V … 5 V DC                                          |
| 3   | OUT         | Digitaler Ausgang (standardmäßig HIGH, LOW bei Hindernis)          |
| 4   | EN          | Enable-Eingang; meist auf GND gelegt → Modul dauerhaft aktiv       |

---

## Ultraschallmodul HC-SR04 – technische Daten

| Parameter                  | Wert                          | Kommentar                                              |
|----------------------------|-------------------------------|--------------------------------------------------------|
| Betriebs­spannung          | 5 V DC                        | Versorgung über Shield-5-V                             |
| Betriebsstrom              | ca. 16 mA                     | während Messung                                        |
| Arbeitsfrequenz Schall     | 40 kHz                        | Ultraschall-Trägerfrequenz                             |
| Messbereich                | 2 cm … 400 cm                 | typischer Einsatzbereich                               |
| Genauigkeit / Auflösung    | ≈ 3 mm                        | abhängig von Bedingungen                               |
| Trigger-Eingangssignal     | 10 µs TTL-Puls                | steigende Flanke startet Messung                       |
| Echo-Ausgangssignal        | TTL-Pegel, Pulsbreite ∝ Distanz | HIGH-Pulsdauer wird vom Controller gemessen        |
| Wiederholrate              | bis ca. 40 Hz                 | genügend Pause zwischen Messungen einhalten           |
| Steckverbinder             | XH2.54-4P                     | passend zum SONAR-Port des Galaxy-RVR-Shields         |
| Abmessungen                | 46 × 20,5 × 15 mm             | L × B × H                                              |

### Ultraschallmodul HC-SR04 – Pinbelegung

| Pin | Bezeichnung | Funktion                                      |
|-----|------------:|-----------------------------------------------|
| 1   | VCC         | +5 V Versorgung                               |
| 2   | TRIG        | Trigger-Eingang (10 µs TTL-Puls)              |
| 3   | ECHO        | Echo-Ausgang; Pulsdauer ∝ Entfernung          |
| 4   | GND         | Masse                                         |
