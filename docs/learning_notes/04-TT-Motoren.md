# Cornell-Notizen: Lektion 4 - Beherrschung des TT-Motors (SunFounder Galaxy RVR)

| **Fragen / Schlüsselbegriffe** | **Notizen & Details** |
| :--- | :--- |
| **Was ist ein TT-Motor?** | \* Ein **Gleichstrommotor (DC-Motor)** mit integriertem **Getriebe**.<br>\* **Funktion:** Wandelt elektrische in mechanische Energie um (elektromagnetische Induktion).<br>\* **Vorteil:** Getriebe erhöht das Drehmoment für schwerere Lasten.<br>\* **Einsatz:** Treibt die Räder des Rovers an. |
| **Warum ein Motor-Treiber?** | \* **Problem:** Arduino-Signalpins liefern zu wenig Strom (\~20 mA) für Motoren.<br>\* **Lösung:** Motor-Treiber fungiert als Brücke/Verstärker zwischen Arduino und Motor.<br>\* **GalaxyRVR-Shield:** Enthält zwei Treiber-Chips.<br> \* *Chip 1:* Steuert 3 Motoren der linken Seite (Pins 2 & 3).<br> \* *Chip 2:* Steuert 3 Motoren der rechten Seite (Pins 4 & 5). |
| **Motorsteuerung (Richtung)** | \* **Prinzip:** Steuerung über zwei Pins pro Treiber-Chip (z.B. Pin 2 & 3).<br>\* **Logik-Tabelle:**<br> \* `LOW` / `LOW` -\> **Standby** (langsames Ausrollen).<br> \* `LOW` / `HIGH` -\> **Uhrzeigersinn** (Vorwärts).<br> \* `HIGH` / `LOW` -\> **Gegen-Uhrzeigersinn** (Rückwärts).<br> \* `HIGH` / `HIGH` -\> **Bremse** (abruptes Stoppen). |
| **Motorsteuerung (Geschwindigkeit)** | \* **Technik:** **PWM** (Pulsweitenmodulation) simuliert variable Spannung durch schnelles Ein-/Ausschalten.<br>\* **Software-Lösung:** Da Pins 2-5 beim R3-Board keine Hardware-PWM unterstützen, wird die **SoftPWM-Bibliothek** genutzt.<br>\* **Wertebereich:** `0` (Stopp) bis `255` (Vollgas). |
| **Code-Befehle (Arduino)** | \* `pinMode(pin, OUTPUT)`: Pin als Ausgang definieren.<br>\* `digitalWrite(pin, HIGH/LOW)`: An/Aus für einfache Richtungssteuerung.<br>\* `#include <SoftPWM.h>`: Bibliothek einbinden.<br>\* `SoftPWMBegin()`: Initialisierung der Bibliothek in `setup()`.<br>\* `SoftPWMSet(pin, speed)`: Geschwindigkeit (0-255) setzen. |
| **Programmablauf (Beispiel)** | \* **Initialisierung:** Pins definieren, Bibliothek starten.<br>\* **Loop (Schleife):** Motoren starten, warten (`delay()`), stoppen oder Richtung ändern.<br>\* **For-Schleife:** Kann genutzt werden, um Geschwindigkeit schrittweise zu erhöhen (Beschleunigung). |

-----

### Zusammenfassung

Lektion 4 vermittelt die Grundlagen der elektromechanischen Fortbewegung. Kernstück ist der **TT-Motor**, der über einen **Motor-Treiber** auf dem GalaxyRVR-Shield angesteuert wird, da der Mikrocontroller selbst zu wenig Strom liefert. Die **Richtung** wird durch digitale Logikpegel (`HIGH`/`LOW`) an den Steuerpins bestimmt (H-Brücken-Prinzip), während die **Geschwindigkeit** mittels **Software-PWM** (`SoftPWM`-Bibliothek) geregelt wird, da die verwendeten Pins keine Hardware-PWM bieten. Dies ermöglicht präzise Manöver wie Beschleunigen, Bremsen und Richtungswechsel durch Code.

---

## PWM - Motordrehzahl

Durch geringere Motordrehzahl (niedrigeren PWM-Wert) kannst du beim TT-Motor messbar Akku sparen – und mit den SunFounder-Daten kann man das halbwegs quantifizieren.

Aus den TT-Motor-Spezifikationen: ([SunFounder Dokumentation][1])

* Übersetzung: 1:120
* vorgeschlagene Spannung: 3–4,5 V DC
* Leerlaufstrom bei 3 V: ca. 130 mA
* Leerlaufdrehzahl bei 3 V: ca. 38 rpm

Angenommen, du betreibst zwei Motoren bei etwa 4 V (dein Pack liegt in der Praxis dort in Lastnähe), dann grob:

* pro Motor Leerlaufleistung ≈ (P_0 \approx U \cdot I_0 \approx 4,\mathrm{V} \cdot 0{,}13,\mathrm{A} \approx 0{,}52,\mathrm{W})
* zwei Motoren → ≈ 1 W schon ohne Last.

Unter Last steigt der Strom deutlich – ein Vielfaches des Leerlaufstroms ist normal (z. B. 300–600 mA pro Motor). Die mittlere Leistung ist dann näherungsweise proportional zum mittleren PWM-Duty-Cycle, solange du nicht in Sättigung/Blockierbereich kommst.

### Was bedeutet 30 % PWM praktisch?

Wenn du in der Diagnose `Config::SpeedMax` z. B. 255 hast und auf 30 % heruntergehst:

```cpp
const int TEST_SPEED = static_cast<int>(Config::SpeedMax * 0.3f);
// ~ 76 bei SpeedMax = 255
```

dann:

* ist der Motor nur 30 % der Zeit „voll bestromt“, 70 % ist er aus → mittlerer Strom ≈ 0,3·Vollstrom
* bei zwei Motoren und z. B. 0,4–0,5 A Vollstrom pro Motor liegen wir grob statt 4–5 W nur noch bei 1,2–1,5 W während des Tests

In deinem Diagnose-Sketch:

* Motor-Test läuft 5 s,
* das ist kurz im Vergleich zu Gesamtfahrzeit, aber:
  – du schonst Akku,
  – du schonst die TT-Getriebe (Plastikzahnräder) und Reifen,
  – die Bewegung bleibt klar erkennbar.

Für den echten Fahrbetrieb gilt:

* Vollgas (100 % PWM) ist energetisch am teuersten und mechanisch am belastendsten.
* Fährst du im „Öko-Modus“ bei z. B. 40–60 % PWM und akzeptierst geringere Endgeschwindigkeit, ist der mittlere Motorstrom deutlich kleiner → längere Laufzeit pro Akkuladung.

Kurz zusammengefasst:

* Ja, niedriger PWM-Wert = geringere elektrische Leistung = weniger Akkuverbrauch.
* Deine Idee, Diagnose und ggf. „Cruise-Speed“ bei ~30–60 % von `SpeedMax` zu fahren, passt sehr gut zu den TT-Motor-Spezifikationen und schont Motor + Akku.

[1]: https://docs.sunfounder.com/projects/galaxy-rvr/de/latest/hardware/cpn_tt_motor_xh.html "TT-Motor — SunFounder GalaxyRVR Kit for Arduino 1.0 Dokumentation"
