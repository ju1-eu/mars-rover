# PWM - Helligkeit von RGB-Strip am Galaxy RVR

### 1. Grundprinzip: weniger Licht = weniger Strom

Beim Galaxy-RVR-RGB-Strip (klassische 3-Kanal-LEDs, kein „intelligenter“ WS2812) gilt:

* Jede Farbe ist eine LED-Reihe mit (vereinfacht) fast **konstantem Spannungsabfall**.
* Der Strom wird über Widerstand/Treiber begrenzt.
* Wenn du mit PWM dimmst (z. B. 50 % Duty-Cycle), ist die LED nur **50 % der Zeit an** ⇒ mittlerer Strom ≈ 50 %, mittlere Leistung ≈ 50 %.

Es ist also nicht nur optisch dunkler, sondern die elektrische Energie pro Zeit sinkt fast proportional zur mittleren Helligkeit.

### 2. Grobe Überschlagsrechnung

Nehmen wir beispielhaft an (typische Größenordnung):

* Pro Farbe ca. **20–60 mA** bei 5 V (laut ähnlicher LED-Strips).
* Worst Case Weiß (R+G+B) ⇒ z. B. 3×60 mA = **180 mA**.
* Leistung:
  ( P = U \cdot I = 5,\mathrm{V} \cdot 0{,}18,\mathrm{A} \approx 0{,}9,\mathrm{W} ).

Jetzt dimmst du auf 30 % über PWM:

* Effektiver Strom ≈ 0,3·0,18 A = **54 mA**
* Leistung ≈ 5 V·0,054 A = **0,27 W**
  → rund **70 % weniger** Verbrauch als bei Vollgas.

Bei den mAh deines Akku-Packs ist das nicht der größte Verbraucher (Motoren und Logik ziehen deutlich mehr), aber es summiert sich, vor allem wenn der Strip dauernd an ist.

### 3. Praktische Empfehlung für dich

1. **Statt digitalWrite** besser SoftPWM/analogWrite nutzen:

   * Beispiel: `SoftPWMSet(Pin::RGB_R, 64);` statt an/aus (0–255).
2. Für „Diagnose-LED“ reicht oft:

   * 20–30 % Helligkeit (Wertebereich 50–80 von 255),
   * du siehst die Farben noch gut, aber verbrauchst deutlich weniger.
3. Wenn der Strip nur Status anzeigen soll, kannst du:

   * Helligkeit reduzieren und
   * die Einschaltdauer kurz halten (z. B. nur alle paar Sekunden kurz „blinken“), dann ist der mittlere Verbrauch extrem niedrig.

### 4. Kurzantwort

* Ja, **dimmen per PWM spart Akku**, weil der mittlere Strom durch den LED-Strip proportional zur Einschaltdauer sinkt.
* Für Diagnose-Zwecke kannst du die Helligkeit ohne Probleme stark reduzieren und so etwas Laufzeit herausholen, vor allem wenn der Strip ständig an wäre.
