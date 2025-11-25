# Engineering (Ingenieurwissenschaften)
Diese Einheit behandelt das Thema: **"Systemarchitektur & Hardware-Abstraktion (HAL)"**.

-----

# Lektion 3: Ingenieurskunst – Modularität & Hardware-Design

**Zielgruppe:** Informatik / Technik-Grundkurs (Oberstufe).
**Lernziel:** Verstehen, warum man Code in Schichten (Layers) unterteilt und wie man Hardware-Einschränkungen (Pin-Mangel) durch Software-Engineering löst.
**Bezug zur Firmware:** `src/hal/` vs. `src/logic/` und `include/Pins.h`.

-----

## 1\. Das Konzept: Separation of Concerns (Trennung der Zuständigkeiten)

Ein Anfänger schreibt alles in eine Datei. Ein Ingenieur baut modulare Systeme. Das GalaxyRVR Profi-Projekt nutzt eine **3-Schichten-Architektur**, die das System wartbar und austauschbar macht.

### Die Analogie: Das Auto

Erklären Sie den Schülern die Architektur anhand eines Autos:

1.  **Application Layer (`src/main.cpp`): Der Fahrer.**

      * Entscheidet: "Ich will nach München fahren" oder "Bremsen, Hindernis\!".
      * Kennt keine Details über die Einspritzpumpe.
      * Nutzt den Zustandsautomaten (FSM) für Entscheidungen.

2.  **Logic Layer (`src/logic/`): Der Bordcomputer (Fahrassistenz).**

      * Berechnet: "Um die Kurve mit Radius $r$ zu fahren, muss das linke Rad 80% und das rechte 100% Geschwindigkeit haben" ($v_\text{in} = v \cdot (1.0 - r)$).
      * Kennt keine Pin-Nummern. Es ist reine Mathematik.
      * *Vorteil:* Dieser Code funktioniert auf einem Arduino genauso wie auf einem PC oder ESP32.

3.  **HAL - Hardware Abstraction Layer (`src/hal/`): Der Motor & Mechaniker.**

      * Führt aus: Setzt elektrische Spannungen an Pin 5 oder Pin 6.
      * Weiß genau, welcher Chip verbaut ist (ATmega328P oder ESP32).
      * Kapselt die "schmutzigen" Hardware-Details weg.

-----

## 2\. Engineering Challenge: Das PWM-Problem

Ein zentraler Teil des Engineerings ist das Lösen von Restriktionen (Einschränkungen). Der GalaxyRVR stellt uns vor ein konkretes Problem, das in der `README.md` dokumentiert ist.

### Das Problem: Zu wenig Hardware-Ressourcen

Der Mikrocontroller (Arduino Uno / ATmega328P) hat nur **6 Pins**, die "echtes" Hardware-PWM (Pulsweitenmodulation zur Geschwindigkeitssteuerung) können. Das Shield des Galaxy RVR ist jedoch so verdrahtet, dass nicht alle Motoreingänge auf diesen Pins liegen.

**Pin-Analyse (aus der Dokumentation):**

| Motor-Funktion | Pin | Typ | Problem |
| :--- | :--- | :--- | :--- |
| Links Vorwärts | 2 | **Digital** | Kann von Haus aus nur AN/AUS (kein Dimmen\!) |
| Links Rückwärts | 3 | PWM | OK (Hardware regelt das) |
| Rechts Rückwärts | 4 | **Digital** | Kann von Haus aus nur AN/AUS |
| Rechts Vorwärts | 5 | PWM | OK (Hardware regelt das) |

### Die Ingenieurs-Lösung: Hybrides Design

Da wir die Hardware (das Shield) nicht ändern können, müssen wir eine Software-Lösung "engineeren".

  * **Lösung:** Einsatz einer **SoftPWM**-Klasse im HAL.
  * **Funktionsweise:** Wir simulieren PWM auf Pin 2 und 4, indem wir den Pin per Software extrem schnell an- und ausschalten.
  * **Trade-off (Abwägung):** Software-PWM verbraucht Rechenleistung (CPU-Zyklen), spart aber Hardware-Kosten. Hardware-PWM läuft im Hintergrund ohne CPU-Last.
  * **Lernziel:** Ingenieure müssen oft Kompromisse zwischen Hardware-Kosten und Software-Komplexität finden.

-----

## 3\. Workflow für Erweiterungen

Wie arbeitet ein Ingenieur, wenn er das System erweitern will? Die Dokumentation gibt einen strikten Workflow vor, den Schüler simulieren können.

**Aufgabe: Einbau eines "Scheinwerfers" (LED)**

1.  **Konfiguration (`include/Pins.h`):**
    Niemals "Magische Nummern" im Code nutzen\!

    ```cpp
    // Falsch:
    digitalWrite(13, HIGH);

    // Richtig (Engineering-Style):
    constexpr uint8_t PIN_HEADLIGHT = 13;
    ```

    *Warum?* Ändert sich der Pin, muss man es nur an einer Stelle ändern. Das spart Wartungskosten.

2.  **Treiber (`src/hal/LightSystem.cpp`):**
    Erstellen einer Klasse, die `on()`, `off()` oder `blink()` anbietet. Die `main.cpp` darf niemals `digitalWrite` aufrufen, sondern nur `lightSystem.on()`.

3.  **Integration (`src/main.cpp`):**
    Im Zustand `CRUISE` wird das Licht automatisch eingeschaltet.

-----

## Zusammenfassung der Lektion

In dieser Einheit lernen Schüler, dass Programmieren mehr ist als Code-Syntax.

  * **Architektur** entscheidet, ob ein Projekt langfristig überlebt oder im Chaos endet.
  * **Abstraktion** erlaubt es, Software auf neue Hardware (z.B. vom Arduino zum ESP32) zu portieren, indem man nur den HAL austauscht (`src/logic/` bleibt identisch).
  * **Problemlösung:** Technische Limits (wie fehlende PWM-Pins) werden durch kreative Software-Lösungen umgangen.
