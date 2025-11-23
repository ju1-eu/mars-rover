# Cornell-Notizen: Lektion 4 - Motoren & Hardware-Abstraktion

| **Fragen / Schlüsselbegriffe** | **Notizen & Details** |
| :--- | :--- |
| **Was ist ein TT-Motor?** | \* **Typ:** Gleichstrommotor (DC) mit Getriebe (gelb).<br>\* **Funktion:** Elektromagnetische Induktion wandelt Strom in Rotation.<br>\* **Besonderheit:** Hohes Drehmoment durch Untersetzung, ideal für Rover-Räder. |
| **Hardware-Setup (Shield)** | \* **Problem:** ATmega328P liefert nur \~20mA pro Pin (zu wenig für Motoren).<br>\* **Lösung:** Motor-Treiber auf dem Shield verstärken das Signal.<br>\* **Gruppen:**<br> \* *Links:* Pins 2 & 3.<br> \* *Rechts:* Pins 4 & 5. |
| **PWM-Problem & Lösung** | \* **Hardware-Limitierung:** ATmega328P hat nur an Pins 3, 5, 6, 9, 10, 11 echtes PWM.<br>\* **Konflikt:** Galaxy RVR nutzt Pin 2 & 4 für Motoren (kein Hardware-PWM).<br>\* **Lösung:** Nutzung der **`SoftPWM`**-Bibliothek für Pin 2 & 4, um PWM per Software zu simulieren. |
| **Software-Architektur (HAL)** | \* **Konzept:** Hardware Abstraction Layer (HAL) trennt Logik von Hardware.<br>\* **Ziel:** `main.cpp` kennt keine Pin-Nummern oder `SoftPWMSet`.<br>\* **Struktur:**<br> 1. `include/Pins.h`: Pin-Definitionen (`constexpr`).<br> 2. `src/hal/Motor.cpp`: Kapselt `SoftPWM` und `analogWrite`.<br> 3. `src/hal/Motor.h`: Öffentliche Schnittstelle (`setSpeed`). |
| **Profi-Code (C++17)** | \* **Keine Magic Numbers:** Konstanten in `Config.h`.<br>\* **Non-Blocking:** Kein `delay()`, sondern `millis()`-Timer für Testabläufe.<br>\* **Zustandsautomat:** `enum class TestState` steuert den Testablauf (Vorwärts -\> Pause -\> Rückwärts). |

-----

### Zusammenfassung

Lektion 4 behandelt die elektromechanischen Grundlagen und deren professionelle softwareseitige Umsetzung. Aufgrund der Hardware-Limitierung des ATmega328P (fehlendes Hardware-PWM an Pin 2/4) wird eine **Software-PWM-Lösung** (`SoftPWM`) benötigt. Im Gegensatz zum einfachen Beispielcode wird hier eine **HAL (Hardware Abstraction Layer)** implementiert, die diese Komplexität kapselt. Die Anwendungslogik steuert die Motoren über abstrakte Befehle (`HAL::Motor::setSpeed`), nutzt **C++17 Features** und verzichtet auf blockierende `delay()`-Aufrufe zugunsten eines **nicht-blockierenden Zustandsautomaten**.
