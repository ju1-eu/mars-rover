#include <Arduino.h>

void setup() {
    // Serielle Schnittstelle mit 115200 Baud starten
    Serial.begin(115200);

    // kurze Pause, damit sich der Monitor verbinden kann
    delay(1000);

    // Einmalige Ausgabe
    Serial.println("Hallo Welt aus PlatformIO");
}

void loop() {
    // nichts tun
}
