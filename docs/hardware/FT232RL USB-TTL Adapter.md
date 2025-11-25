# FT232RL USB-TTL Adapter – technische Daten

AZ-Delivery-FT232RL-Platine

| Parameter          | Wert                           | Kommentar                                             |
|--------------------|--------------------------------|-------------------------------------------------------|
| Produktname        | AZ-Delivery FT232RL USB-TTL    | USB-zu-UART-Adapter                                   |
| USB-Schnittstelle  | USB-C                          | Verbindung zum PC / Laptop                            |
| Bridge-Chip        | FTDI FT232RL                   | erstellt virtuellen COM-Port                          |
| Datenschnittstelle | UART (TTL)                     | serielle Kommunikation zu externer Hardware           |
| Logikpegel         | 3,3 V oder 5 V (per Jumper)    | wählbar je nach Ziel-Hardware                         |
| Typische Nutzung   | Programmierung / Debugging     | z. B. AVR-MCUs, ESP-Module, GPS-Modul, Sensorboards   |
| Anschlussform      | Stiftleiste, Male Pins         | direkte Steckverbindung auf Breadboard oder Kabel     |

### FT232RL USB-TTL Adapter – Pinbelegung

| Pin | Bezeichnung | Funktion                                      |
|-----|------------:|-----------------------------------------------|
| 1   | DTR         | Data Terminal Ready (Handshake / Reset)       |
| 2   | RX          | UART-Empfang (Daten vom Ziel zum PC)          |
| 3   | TX          | UART-Sendung (Daten vom PC zum Ziel)          |
| 4   | VCC         | Versorgungsausgang 3,3 V oder 5 V (je nach Jumper) |
| 5   | CTS         | Clear To Send (Handshake)                     |
| 6   | GND         | Masse / Bezugspotential                       |

