# Arts (Kunst & Gestaltung)

Diese Einheit behandelt das Thema: **"Code als Handwerk: Struktur, Stil & Dokumentation"**.

-----

# Lektion 4: Die Kunst des sauberen Codes (Clean Code)

**Zielgruppe:** Informatik-Kurse, Projektmanagement-Einführung.
**Lernziel:** Verstehen, dass Code primär für Menschen (Entwickler) geschrieben wird, nicht nur für Maschinen. Erlernen von Standards für Lesbarkeit und Wartbarkeit.
**Bezug zur Firmware:** `README.md` (Frontmatter, Coding Guidelines), `include/Config.h`, `include/Pins.h`.

-----

## 1\. Code-Ästhetik: Lesbarkeit statt "Hieroglyphen"

Programmieren ist eine sprachliche Kunstform. Ein "hässlicher" Code funktioniert vielleicht, ist aber unwartbar. Das Projekt GalaxyRVR Profi setzt hier hohe Standards.

### A. Das Verbot der "Magischen Zahlen"

In Anfänger-Codes findet man oft Zeilen wie: `digitalWrite(4, HIGH);`.

  * **Warum ist das "hässlich"?** Die Zahl `4` hat keine Bedeutung. Ein Leser weiß nicht, ob das ein Motor, eine LED oder ein Sensor ist.
  * **Die Kunst (Refactoring):** In der `include/Pins.h` werden diese Zahlen "getauft".
      * Das Projekt nutzt `constexpr` für Konstanten.
      * Statt `4` steht dort `PIN_MOTOR_R_BACK`.
  * **Lerneffekt:** Wir geben abstrakten Zahlen semantische Bedeutung. Das ist Design.

### B. Starke Typisierung (`enum class`)

Die Guidelines fordern die Verwendung von `enum class` für Zustände.

  * *Schlecht (C-Style):* `int zustand = 1;` (Was bedeutet 1? Fahren? Fehler?)
  * *Schön (C++ Style):* `State current = State::CRUISE;`
  * Dies macht den Code selbsterklärend und verhindert logische Fehler, da man Zustände nicht versehentlich mit Zahlen addieren kann.

-----

## 2\. Informations-Design: Die Struktur der Dokumentation

Dokumentation ist das "User Interface" für Entwickler. Die `README.md` dieses Projekts ist nicht einfach nur Text, sondern folgt einem visuellen Design-System.

### A. Frontmatter & Metadaten

Der Kopf der Datei enthält strukturierte Metadaten:

```yaml
title: "GalaxyRVR Profi – Firmware-Dokumentation"
author: "Jan Unger"
status: "active"
kanban.board: "Mars Rover"
```

  * **Analyse:** Dies erinnert an ein Buch-Cover. Es gibt sofort Kontext (Wer? Was? Status?).
  * **Kanban-Board:** Der Verweis auf `kanban.board: "Mars Rover"` zeigt, dass hier visuelles Projektmanagement (Spalten wie "To Do", "In Progress", "Done") genutzt wird, um den kreativen Prozess zu ordnen.

### B. Die Ordner-Hierarchie als Landkarte

Die Ordnerstruktur ist visuell so gestaltet, dass man sich sofort zurechtfindet:

  * `include/`: Die "Speisekarte" (Was kann das System?).
  * `src/`: Die "Küche" (Wie wird es gemacht?).
  * `docs/`: Die "Bedienungsanleitung".

Dies folgt dem Prinzip der **kognitiven Entlastung**. Ein guter Architekt gestaltet Räume so, dass man weiß, wo man ist. Ein guter Software-Architekt gestaltet Ordner genauso.

-----

## 3\. Stil & Konventionen (Styleguides)

So wie in der Kunst oder Musik gibt es auch im Code "Stilrichtungen". Das Projekt definiert seinen Stil explizit in den **Coding Guidelines**:

1.  **Sprache:** Aktive Formulierungen in der Dokumentation ("Aktive Sprache, konkrete Aussagen").
2.  **Ressourcen-Bewusstsein:** Der sparsame Umgang mit RAM (z.B. Nutzung von `F("...")` Strings) ist eine Form von Minimalismus – eine ästhetische Entscheidung für Effizienz.
3.  **Konsistenz:** Die strikte Trennung von `release` und `debug` Umgebungen in der `platformio.ini` schafft klare Verhältnisse.

-----

## 4\. Praktische Übung für Schüler: "Code Review"

**Aufgabe:**
Geben Sie den Schülern einen "hässlichen" (aber funktionierenden) Code-Schnipsel und lassen Sie ihn nach den Guidelines des GalaxyRVR Profi "verschönern".

**Beispiel (Hässlich):**

```cpp
void loop() {
  digitalWrite(2, 1);
  analogWrite(3, 200);
  delay(1000);
}
```

**Lösung (Kunstvoll / Clean Code):**

```cpp
// Anwendung der Projekt-Guidelines:
// 1. Keine Magic Numbers (Pins benannt)
// 2. Keine delay() (Non-blocking)
// 3. Sprechende Methodennamen

void loop() {
    if (timer.check()) {
       motorLeft.setSpeed(Speed::FAST); // 200 wird zu einer Konstante
       lightSystem.on();
    }
}
```

-----

## Zusammenfassung der Lektion

Im Bereich **Arts** lernen Schüler, dass Softwareentwicklung ein kreativer und gestalterischer Prozess ist. Guter Code zeichnet sich durch **Klarheit, Struktur und Eleganz** aus. Das Projekt GalaxyRVR Profi dient hier als Galerie für professionelles Code-Design.
