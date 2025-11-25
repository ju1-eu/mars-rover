/**
 * @file       main.cpp
 * @brief      Hardware- und Toolchain-Selbsttest für Seeed XIAO ESP32S3.
 *
 * @details
 * Dieses Beispielprogramm validiert grundlegende MCU-Funktionen
 * (GPIO, Timing, serielle Schnittstelle) und demonstriert – sofern
 * verfügbar – moderne C++-Features wie `std::vector` und
 * konstante Referenzen.
 *
 * Der Code ist bewusst so gestaltet, dass er sowohl auf ESP32-
 * Architekturen als auch auf klassischen AVR-Boards (z. B. Uno,
 * ATmega328P) übersetzbar ist. Plattformspezifische Teile werden
 * über `#if`/`#else`-Blöcke gekapselt.
 *
 * @author     Jan Unger
 * @version    1.0.2  (Fehlerbereinigung AVR)
 * @date       2025-11-22
 *
 * @note
 *   - Plattform: Dual-Plattform-kompatibel (ESP32S3 / ATmega328P)
 *   - Toolchain: GCC 8.4 (C++17/20-kompatibel)
 */

#include <Arduino.h>

// ----------------------------------------------------------------------
// BEDINGTE KOMPILIERUNG: STL-FEATURES
// ----------------------------------------------------------------------
// `#include <vector>` und STL-Code nur für ESP32-Architekturen einbinden.
#if defined(ARDUINO_ARCH_ESP32) || defined(ARDUINO_ARCH_ESP32S3)
#include <vector>
#define USE_STL_FEATURES
#endif

// ----------------------------------------------------------------------
// GLOBALE KONFIGURATION & HARDWARE-DEFINITIONEN
// ----------------------------------------------------------------------

/**
 * @brief Pin-Nummer der Onboard-User-LED.
 *
 * @note
 * Wird als `constexpr` definiert, damit der Wert im Programmspeicher
 * abgelegt und kein RAM verbraucht wird.
 *
 * @hardware ESP32S3: GPIO 21
 */
constexpr int kLedPin = 21;

/**
 * @struct StatusConfig
 * @brief  Beispielhafte Konfigurationsstruktur für ein Peripherie-Modul.
 *
 * Dient in diesem Beispiel zur Demonstration von Initialisierung
 * (z. B. mittels Designated Initializers) und zur Ausgabe über
 * die serielle Schnittstelle.
 */
struct StatusConfig {
    const char *moduleName; ///< Klartext-Bezeichnung des Moduls.
    int id;                 ///< Interne ID oder Adressierung des Moduls.
    bool active;            ///< Aktiv-Status (true = aktiv/verwenden).
};

// --- Globale Laufzeit-Variablen ---

/// Zeitstempel der letzten Status-Aktualisierung (ms seit Programmstart).
unsigned long lastUpdate = 0;

/// Aktueller LED-Zustand (true = ein, false = aus).
bool ledState = false;

// ----------------------------------------------------------------------
// FUNKTIONEN (plattformspezifisch)
// ----------------------------------------------------------------------

#ifdef USE_STL_FEATURES
/**
 * @brief  Verarbeitet exemplarische Daten aus einem STL-Vektor (ESP32).
 *
 * @details
 * Die Funktion demonstriert:
 *  - die Nutzung von `std::vector<int>` auf ESP32-Plattformen,
 *  - die effiziente Übergabe per `const`-Referenz,
 *  - eine einfache Ausgabe der Elemente und der Vektorgröße via Serial.
 *
 * @param daten Konstante Referenz auf den zu verarbeitenden Vektor.
 */
void verarbeiteDaten(const std::vector<int> &daten) {
    Serial.print("-> Vektor Check: [ ");
    for (const auto &wert : daten) {
        Serial.print(wert);
        Serial.print(" ");
    }
    Serial.print("] | Größe: ");
    Serial.println(daten.size());
}
#else
/**
 * @brief  Dummy-Implementierung für ressourcenarme Plattformen (z. B. AVR).
 *
 * @details
 * Auf klassischen AVR-Controllern steht die C++-Standardbibliothek
 * in der Regel nicht zur Verfügung. Diese Funktion dient als
 * Platzhalter, um den Aufruf im restlichen Code zu erhalten,
 * ohne zusätzlichen Flash- oder RAM-Verbrauch zu erzeugen.
 */
void verarbeiteDaten_AVR_Dummy() {
    // Absichtlich leer: Verhindert lediglich Kompilierungsfehler.
}
#endif // USE_STL_FEATURES

// ----------------------------------------------------------------------
// HAUPTPROGRAMM
// ----------------------------------------------------------------------

/**
 * @brief Einmalige Initialisierung von Hardware und
 * Kommunikationsschnittstellen.
 *
 * @details
 * - Initialisiert die serielle Schnittstelle (115200 Baud).
 * - Konfiguriert den LED-Pin als Ausgang.
 * - Wartet kurz (2 s), um den seriellen Monitor zu verbinden.
 * - Legt eine beispielhafte Status-Konfiguration an und gibt diese
 *   plattformspezifisch (ESP32 vs. AVR) aus.
 *
 * @post
 *  Die LED-Pinrichtung ist gesetzt, die serielle Schnittstelle bereit
 *  und der Benutzer hat eine erste Statusmeldung im Terminal.
 */
void setup() {
    Serial.begin(115200);

    pinMode(kLedPin, OUTPUT);

    // BLOCKING: Wartezeit (2s), z. B. für das Öffnen des Serial Monitors.
    delay(2000);

    Serial.println("\n=== Hardware Test (Dual-Platform) ===");

    StatusConfig sensorConf = {"IMU-Sensor", 42, true};

    // --- Korrekte Ausgabe für unterschiedliche Plattformen ---
#if defined(ARDUINO_ARCH_ESP32) || defined(ARDUINO_ARCH_ESP32S3)
    // ESP32 besitzt Serial.printf()
    Serial.printf("Konfiguriert: %s (ID: %d)\n", sensorConf.moduleName,
                  sensorConf.id);
#else
    // AVR: Serial.printf() nicht verfügbar -> klassische Ausgabe.
    Serial.print("Konfiguriert: ");
    Serial.print(sensorConf.moduleName);
    Serial.print(" (ID: ");
    Serial.print(sensorConf.id);
    Serial.println(")");
#endif
}

/**
 * @brief Hauptschleife des Programms (Control Loop).
 *
 * @details
 * Die Schleife arbeitet vollständig non-blocking über einen
 * Zeitvergleich (Polling-Ansatz):
 *  - Alle 500 ms wird der LED-Zustand getoggelt.
 *  - Auf ESP32-Systemen wird zusätzlich ein Test-Vektor erzeugt
 *    und an @c verarbeiteDaten() übergeben.
 *  - Auf AVR-Systemen wird die Dummy-Funktion @c verarbeiteDaten_AVR_Dummy()
 *    aufgerufen.
 *
 * @note
 * Der Einsatz von `millis()` vermeidet Blockierungen durch `delay()` und
 * eignet sich als Vorlage für einfache zeitgesteuerte Aufgaben.
 */
void loop() {
    unsigned long now = millis();

    // --- TIMING / POLLING (Non-Blocking) ---
    if (now - lastUpdate >= 500) {
        lastUpdate = now;

        // LED toggeln (ein/aus)
        ledState = !ledState;
        digitalWrite(kLedPin, ledState ? LOW : HIGH);

        // Aufruf der jeweils passenden Testfunktion
#ifdef USE_STL_FEATURES
        // Vektor erstellen und Funktion aufrufen (NUR ESP32)
        std::vector<int> testDaten = {10, 20,
                                      static_cast<int>(random(100, 999))};
        verarbeiteDaten(testDaten);
#else
        // Platzhalter-Funktion für AVR (NUR UNO/ATmega)
        verarbeiteDaten_AVR_Dummy();
#endif
    }
}
