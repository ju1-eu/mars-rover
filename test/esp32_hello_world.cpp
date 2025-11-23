/**
 * @file       main.cpp
 * @brief      Hardware- und Toolchain-Test für Seeed XIAO ESP32S3.
 * @details    Validiert grundlegende MCU-Funktionen (GPIO, Timing, Serial)
 * und demonstriert moderne C++-Features (std::vector, Designated Initializers).
 * * @platform   Dual-Plattform-kompatibel (ESP32S3 / ATmega328P)
 * @toolchain  GCC 8.4 (C++17/20 kompatibel)
 * @author     Jan Unger
 * @version    1.0.2 (Fehlerbereinigung AVR)
 * @date       2025-11-22
 */

#include <Arduino.h>

// ----------------------------------------------------------------------
// BEDINGTE KOMPILIERUNG: STL-FEATURES
// ----------------------------------------------------------------------
// #include <vector> und STL-Code nur für ESP32-Architekturen
#if defined(ARDUINO_ARCH_ESP32) || defined(ARDUINO_ARCH_ESP32S3)
#include <vector>
#define USE_STL_FEATURES
#endif

// ----------------------------------------------------------------------
// GLOBALE KONFIGURATION & HARDWARE DEFINITIONEN
// ----------------------------------------------------------------------

/**
 * @brief Pin-Nummer der Onboard-User-LED.
 * @note  Wird als 'const' definiert, um RAM-Verbrauch zu minimieren.
 * @hardware  ESP32S3 GPIO 21
 */
constexpr int kLedPin = 21;

/**
 * @struct StatusConfig
 * @brief Beispiel-Konfiguration für ein Peripherie-Modul.
 */
struct StatusConfig {
    const char *moduleName;
    int id;
    bool active;
};

// --- Globale Laufzeit-Variablen ---
unsigned long lastUpdate = 0;
bool ledState = false;

// ----------------------------------------------------------------------
// FUNKTIONEN (Plattformspezifisch)
// ----------------------------------------------------------------------

#ifdef USE_STL_FEATURES
/**
 * @brief  Verarbeitet exemplarische Daten aus einem STL-Vektor
 * (ESP32-Funktion).
 * @details Demonstriert die effiziente Übergabe mittels const Reference.
 * @param daten  Konstante Referenz auf den zu verarbeitenden Vektor (Input).
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
 * @brief  Platzhalter für ressourcenarme Plattformen (z.B. Uno).
 * @details Auf dem Uno wird diese Funktion verwendet, um Fehler zu vermeiden.
 */
void verarbeiteDaten_AVR_Dummy() {
    // Führt keine Aktion aus, vermeidet aber Kompilierungsfehler.
}
#endif // USE_STL_FEATURES

// ----------------------------------------------------------------------
// HAUPTPROGRAMM
// ----------------------------------------------------------------------

/**
 * @brief Einmalige Initialisierung der Hardware und
 * Kommunikationsschnittstellen.
 */
void setup() {
    Serial.begin(115200);

    pinMode(kLedPin, OUTPUT);

    // BLOCKING: Wartezeit (2s)
    delay(2000);

    Serial.println("\n=== Hardware Test (Dual-Platform) ===");

    StatusConfig sensorConf = {"IMU-Sensor", 42, true};

// --- KORREKTUR FÜR Serial.printf() ---
#if defined(ARDUINO_ARCH_ESP32) || defined(ARDUINO_ARCH_ESP32S3)
    // Verwende Serial.printf() auf ESP32
    Serial.printf("Konfiguriert: %s (ID: %d)\n", sensorConf.moduleName,
                  sensorConf.id);
#else
    // Verwende das Standard-AVR-Format (Serial.print()) auf Uno
    Serial.print("Konfiguriert: ");
    Serial.print(sensorConf.moduleName);
    Serial.print(" (ID: ");
    Serial.print(sensorConf.id);
    Serial.println(")");
#endif
}

/**
 * @brief Hauptschleife (Control Loop).
 */
void loop() {
    unsigned long now = millis();

    // --- TIMING / POLLING (Non-Blocking) ---
    if (now - lastUpdate >= 500) {
        lastUpdate = now;

        ledState = !ledState;
        digitalWrite(kLedPin, ledState ? LOW : HIGH);

// AUFRUF AN DIE JEWEILIGE FUNKTION
#ifdef USE_STL_FEATURES
        // Vektor erstellen und Funktion aufrufen (NUR ESP32)
        std::vector<int> testDaten = {10, 20, (int)random(100, 999)};
        verarbeiteDaten(testDaten);
#else
        // Platzhalter-Funktion für AVR aufrufen (NUR UNO)
        verarbeiteDaten_AVR_Dummy();
#endif
    }
}
