# 🤝 Leitfaden für Code-Beiträge (CONTRIBUTING)

Willkommen beim GalaxyRVR-Profi-Projekt!

Dieses Projekt verfolgt einen hohen Standard in Bezug auf Software-Architektur und Dokumentationsqualität.
Unsere Philosophie ist die **doppelte Legitimation**: Der Code muss technisch exzellent (effizient, sicher) sein und gleichzeitig seine Funktionsweise didaktisch klar vermitteln.

Bitte lesen Sie diesen Leitfaden sorgfältig durch, bevor Sie einen Pull Request (PR) einreichen.

---

## 1. Architektur-Regeln (Separation of Concerns)

Das Projekt nutzt eine strikte **3-Schichten-Architektur**.
Jeder Beitrag muss eindeutig einer dieser Ebenen zugeordnet werden.
Verletzungen der Abhängigkeitsrichtung führen zur Ablehnung des PRs.

### 1.1 Schichtenübersicht

| Schicht      | Verzeichnis     | Zweck & Verantwortung                              | Erlaubte Abhängigkeiten      |
|--------------|-----------------|----------------------------------------------------|------------------------------|
| 1. Application | `src/main.cpp` | Was passiert wann? (FSM, Setup, Not-Aus).          | Kennt Logic & HAL.           |
| 2. Logic     | `src/logic/`    | Wie wird geregelt? (PID, Kinematik, Filter).       | Kennt nur HAL. Keine direkten Hardware-Pins. |
| 3. HAL       | `src/hal/`      | Womit wird gearbeitet? (Treiber, Register, I²C).   | Kennt niemanden über sich. Keine Logic-Header einbinden. |

### 1.2 Goldene Regel der Abhängigkeit

Code darf nur Module importieren (`#include`), die **architektonisch unter ihm** stehen:

```text
Application → Logic → HAL
```

---

## 2. Coding Guidelines

Wir schreiben Embedded-C++, das deterministisch und ressourcenschonend ist.

### 2.1 C++-Standards & Typisierung

* **Versionierung:**

  * AVR (Uno): C++17

    * Keine STL, kein `std::vector`, kein `new`/`delete` zur Laufzeit.
  * ESP32 (XIAO): C++20

    * STL erlaubt, `std::vector` erlaubt.

* **Strenge Typen:**

  * Nutzen Sie `enum class` für Zustände.
  * Vermeiden Sie `#define`-Makros für Konstanten; nutzen Sie stattdessen `constexpr`.

Beispiele:

Schlecht:

```cpp
#define SPEED 100
```

Gut:

```cpp
constexpr std::uint8_t CRUISE_SPEED = 100;
```

### 2.2 Zeitverhalten (Non-Blocking I/O)

Die Nutzung von `delay()` ist innerhalb der `loop()` und in Logik-Modulen **streng verboten**.
Das System muss jederzeit reaktionsfähig bleiben (z. B. für Not-Aus).

Zeitsteuerung erfolgt ausschließlich über Delta-Messung:

$$
t_{\text{now}} - t_{\text{last}} \ge \Delta t
$$

Richtiges Pattern:

```cpp
unsigned long now = millis();
if (now - lastExecution >= INTERVAL_MS) {
    lastExecution = now;
    // ... Aktion ...
}
```

### 2.3 Dokumentation (Doxygen)

Code ohne Dokumentation existiert nicht.
Wir nutzen Doxygen mit spezifischen Custom-Tags, um technische und sicherheitsrelevante Aspekte hervorzuheben.

**Pflicht für alle Header-Files (`.h`):**

* `@brief` – Kurze, prägnante Beschreibung.
* `@details` – Erklärung der Algorithmen (gerne mit LaTeX-Formeln für Mathe).
* `@param` / `@return` – Beschreibung der Ein- und Ausgabewerte.

**Projekt-spezifische Tags (wichtig):**

Nutzen Sie diese Tags, um Kontext zu schaffen:

* `@safety` – Beschreibt Sicherheitsmechanismen
  z. B. „Stoppt Motoren bei Verbindungsverlust“.
* `@hardware` – Listet physische Abhängigkeiten
  z. B. „Benötigt Timer1“.
* `@unit` – Gibt die physikalische Einheit an
  z. B. `m/s`, `PWM (0–255)`, `°/s`.

---

## 3. Workflow & Git

### 3.1 Commit-Messages

Wir folgen der **Conventional-Commits**-Konvention, um die Git-Historie lesbar zu halten:

* `feat(scope): ...`
  für neue Funktionen
  z. B. `feat(hal): add support for QMC6310 compass`
* `fix(scope): ...`
  für Fehlerbehebungen
  z. B. `fix(logic): correct PID windup`
* `docs: ...`
  für reine Dokumentationsänderungen
* `refactor: ...`
  für Code-Umbau ohne Funktionsänderung

### 3.2 Pull-Request-Checkliste

Bevor Sie einen PR öffnen, prüfen Sie:

* [ ] **Build:** Kompiliert der Code fehlerfrei für beide Environments?

  ```bash
  platformio run -e uno -e xiao_esp32s3
  ```

* [ ] **Architektur:** Wurde die Schichtentrennung eingehalten?

* [ ] **Doku:** Haben neue Funktionen entsprechende Doxygen-Kommentare inkl. `@safety` / `@unit`-Tags?

* [ ] **Format:** Wurde der Code formatiert (Clang-Format / CppStyle)?

---

Vielen Dank, dass Sie dazu beitragen, den GalaxyRVR besser zu machen! 🚀
