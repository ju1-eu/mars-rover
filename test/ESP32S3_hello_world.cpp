/**
 * @file       main.cpp
 * @brief      "Hallo Welt" & Hardware-Selbsttest für Seeed XIAO ESP32S3.
 * @author     Jan Unger
 * @version    1.1.0 (ESP32 Focus)
 * @date       2025-11-26
 *
 * @details
 * Dieser Sketch dient als "Proof of Concept" für die moderne C++-Entwicklung
 * auf dem ESP32. Er demonstriert:
 * 1. **Modern C++ (C++20):** Nutzung von `std::vector` und Range-based Loops.
 * 2. **Non-Blocking I/O:** Zeitsteuerung ohne `delay()`.
 * 3. **Hardware-Check:** Blinken der User-LED (GPIO 21).
 *
 * @note
 * Um Konflikte mit den AVR-Treibern (HAL) zu vermeiden, nutzt dieser Sketch
 * keine Module aus `src/hal/` oder `src/logic/`, sondern ist standalone.
 *
 * @platform Seeed Studio XIAO ESP32S3
 */

#include <Arduino.h>
#include <vector> // STL-Container (nur auf ESP32/ARM möglich, nicht auf AVR)

// ----------------------------------------------------------------------
// 1. HARDWARE-KONFIGURATION
// ----------------------------------------------------------------------

/**
 * @brief   Pin-Nummer der Onboard-User-LED.
 * @hardware XIAO ESP32S3: GPIO 21 (Low-Active? Zu prüfen).
 * @note    `constexpr` sorgt dafür, dass der Wert zur Compile-Zeit feststeht
 * und keinen RAM verbraucht.
 */
constexpr int PIN_USER_LED = 21;

/**
 * @struct  SystemStatus
 * @brief   Datenstruktur für Telemetrie-Simulation.
 */
struct SystemStatus {
    const char *moduleName; ///< Name des Subsystems
    uint32_t uptime;        ///< Laufzeit in ms
    bool active;            ///< Status-Flag
};

// ----------------------------------------------------------------------
// 2. FUNKTIONEN
// ----------------------------------------------------------------------

/**
 * @brief   Verarbeitet Daten mittels C++ Standard Template Library (STL).
 *
 * @details
 * Zeigt, wie `std::vector` effizient per `const reference` übergeben wird,
 * um unnötige Datenkopien im RAM zu vermeiden.
 *
 * @param   data  Konstante Referenz auf den Vektor.
 */
void processData(const std::vector<int> &data) {
    Serial.printf("[STL Check] Vektor Größe: %d | Elemente: ", data.size());

    // Modern C++: Range-based for loop
    for (const int &val : data) {
        Serial.printf("%d ", val);
    }
    Serial.println();
}

// ----------------------------------------------------------------------
// 3. MAIN LOOP & SETUP
// ----------------------------------------------------------------------

/**
 * @brief   Initialisierung der Hardware.
 * @pre     USB-Serial muss verbunden sein.
 */
void setup() {
    // 1. Serielle Schnittstelle starten
    Serial.begin(115200);

    // 2. GPIO konfigurieren
    pinMode(PIN_USER_LED, OUTPUT);

    // 3. Warten auf USB (wichtig bei Native USB des S3)
    // Wir warten max. 2 Sekunden, damit der Boot nicht ewig hängt,
    // falls kein PC angeschlossen ist.
    unsigned long startWait = millis();
    while (!Serial && (millis() - startWait < 2000)) {
        delay(10);
    }

    Serial.println("\n=== GALAXY RVR: ESP32 CORE TEST ===");
    Serial.printf("CPU Frequenz: %d MHz\n", ESP.getCpuFreqMHz());
    Serial.printf("Flash Größe: %d MB\n",
                  ESP.getFlashChipSize() / (1024 * 1024));
}

/**
 * @brief   Hauptschleife (Nicht-blockierend).
 *
 * @details
 * Implementiert einen einfachen Scheduler, der alle 500ms:
 * 1. Die LED umschaltet (Heartbeat).
 * 2. Einen dynamischen Vektor erzeugt und verarbeitet.
 */
void loop() {
    static unsigned long lastUpdate = 0;
    static bool ledState = false;

    unsigned long now = millis();

    // Delta-Time Check (500ms Intervall)
    if (now - lastUpdate >= 500) {
        lastUpdate = now;

        // A. LED Toggeln
        ledState = !ledState;
        // Hinweis: Viele Onboard-LEDs sind "Low Active" (LOW = An).
        // Wir schreiben hier den logischen Zustand.
        digitalWrite(PIN_USER_LED, ledState ? LOW : HIGH);

        // B. Modern C++ Features testen
        // Erstelle dynamisch einen Vektor mit Zufallszahlen
        std::vector<int> sensorValues = {
            10, 20, static_cast<int>(random(100, 999)) // Zufallswert
        };

        // Verarbeite die Daten
        processData(sensorValues);
    }
}
