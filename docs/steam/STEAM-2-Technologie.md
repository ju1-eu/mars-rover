# Technology (Technologie)

Diese Einheit behandelt das Thema: **"Echtzeit-Verarbeitung & Zustandsautomaten (FSM)"**.

-----

# Lektion 2: Vom Skript zur Architektur – Multitasking auf dem Mikrocontroller

**Zielgruppe:** Informatik-Oberstufe / Berufsschule.
**Lernziel:** Überwindung der linearen Programmierung ("Spaghetti-Code"). Verstehen, wie ein Single-Core-Prozessor scheinbar mehrere Dinge gleichzeitig tut (Non-Blocking I/O) und wie man komplexes Verhalten strukturiert.
**Bezug zur Firmware:** `src/main.cpp` (Application Layer) und `platformio.ini`.

-----

## 1\. Das Problem: Die `delay()`-Falle

In typischen Einsteiger-Tutorials lernen Schüler:
`Schalte Motor an -> Warte 2 Sekunden -> Stopp.`

```cpp
// Der klassische Anfängerfehler
digitalWrite(MOTOR, HIGH);
delay(2000); // Der Prozessor schläft hier!
digitalWrite(MOTOR, LOW);
```

**Das Szenario für die Schüler:**
Der Rover fährt geradeaus. Plötzlich springt eine Katze (oder ein anderes Hindernis) in den Weg.

  * **Problem:** Während der `delay(2000)`-Zeile ist der Roboter **blind und taub**. Er kann Sensoren nicht abfragen und nicht bremsen. Er "schläft" mit Vollgas.

-----

## 2\. Die Lösung: Non-Blocking I/O (Asynchrone Verarbeitung)

Die GalaxyRVR Profi-Firmware verbietet `delay()` im Hauptloop strikt. Stattdessen nutzen wir das Konzept des **Pollings** basierend auf der Systemzeit (`millis()`).

### Das Konzept der Stoppuhr

Statt zu schlafen, schaut der Prozessor extrem schnell (tausende Male pro Sekunde) auf die Uhr.

**Die mathematische Bedingung:**
$$t_\text{now} - t_\text{last} > \Delta t$$

Dies ermöglicht **Pseudo-Multitasking** auf dem ATmega328P:

1.  Prüfe Ultraschall (alle 50ms)
2.  Berechne PID-Regler für Motor (alle 10ms)
3.  Blinke LED (alle 500ms)

Wenn keine Zeit abgelaufen ist, macht der Prozessor einfach *nichts* und springt sofort zur nächsten Aufgabe. Das System bleibt **reaktiv**.

-----

## 3\. Architektur: Der Endliche Automat (Finite State Machine)

Wenn wir kein lineares Skript mehr haben, wer entscheidet dann, was der Roboter tut? Hier kommt die **Application Layer** in `src/main.cpp` ins Spiel.

Wir definieren den Roboter nicht als Abfolge von Befehlen, sondern als Menge von **Zuständen (States)** und **Übergängen (Transitions)**.

### Die Zustände des GalaxyRVR

Die Firmware definiert klare Zustände via `enum class` (Starke Typisierung für Sicherheit):

1.  **CRUISE:** Standardfahrt. Der `DriveAssistant` hält die Spur.
2.  **OBSTACLE\_AVOIDANCE:** Hindernis erkannt (\< 15 cm). Motoren stopp, Logik entscheidet Fluchtweg.
3.  **SMART\_TURN:** Aktive Drehung um 180°, überwacht durch das Gyroskop (nicht durch Zeit\!).
4.  **SAFETY:** Not-Aus, wenn die Neigung \> 45° ist (Kippgefahr).

**Lernaufgabe für Schüler:**
Zeichnen Sie ein Zustandsdiagramm.

  * *Pfeil von CRUISE nach OBSTACLE:* Welche Bedingung muss wahr sein? (Antwort: `distance < 15`)
  * *Pfeil von SMART\_TURN nach CRUISE:* Wann ist die Drehung fertig? (Antwort: Wenn das Integral der Gyro-Werte \> 175° ist, nicht nach 2 Sekunden\!)

-----

## 4\. Modernes Tooling: Warum PlatformIO & C++17?

Der "Profi"-Teil des Projekts liegt auch in der Werkzeugwahl. Wir nutzen nicht die Arduino IDE, sondern eine professionelle IDE (VS Code + PlatformIO).

### A. C++ Standards (Compiler-Flags)

Wir nutzen `-std=gnu++17` für den Arduino Uno und `-std=gnu++2a` (C++20) für den ESP32.

  * **Warum?** Um modernen, sicheren Code zu schreiben.
  * **Beispiel `constexpr`:**
    In `include/Pins.h` werden Pins als `constexpr` definiert.
      * *Effekt:* Der Wert wird zur **Kompilierzeit** fest in den Maschinencode geschrieben. Er belegt keinen RAM-Speicher zur Laufzeit. Auf einem Arduino mit nur 2KB RAM ist das ein entscheidender Vorteil gegenüber `int pin = 2;`.

### B. Environment-Management

In der `platformio.ini` sehen die Schüler zwei Umgebungen:

1.  `env:uno` (Release): Optimiert für Geschwindigkeit, keine unnötigen Ausgaben.
2.  `env:uno-debug` (Debug): Aktiviert Zusatz-Logs (`-DDEBUG`), damit wir am PC sehen, was der Rover "denkt".

Dies lehrt den Unterschied zwischen **Entwicklung** (Fehlersuche) und **Produktion** (Leistung).

-----

## Zusammenfassung der Lektion

Technologie ist mehr als nur Hardware. Die Schüler lernen:

1.  **Reaktivität:** Ein Roboter muss immer "wach" sein (kein `delay()`).
2.  **Struktur:** Komplexes Verhalten wird in Zustände (FSM) zerlegt.
3.  **Professionalität:** Werkzeuge wie Compiler-Flags und Build-Systeme sind keine Schikane, sondern notwendig für robuste Systeme.

