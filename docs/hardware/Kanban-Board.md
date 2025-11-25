---
title: "Mars Rover – Kanban Board"
kanban.board: "Mars Rover"
---

## TODO – Lektionen

- [ ] Lektion 11: Erforschung des visuellen Systems - Kamera und Echtzeitsteuerung
- [ ] Lektion 12: Steuerung des Rovers mit der App

## TODO – Hardware

- [ ] ESP32 CAM
- [ ] Kamera-Adapterplatine (**HINWEIS**: QMC6310 – 3-Achsen-Magnetfeldsensor nicht verbaut im Mars-Rover)

## TODO Autonomen Rover

- [ ] Die IR-Sensoren nutzen, um auch seitlichen Hindernissen auszuweichen (nicht nur Frontal-Crashs).

- [ ] Die Kamera (ESP32-CAM) integrieren, um das Bild zu streamen.

- [ ] Eine Fernsteuerung (Bluetooth/WLAN) bauen, bei der der Assistent trotzdem hilft ("Fly-By-Wire").

- [ ] ESP32-CAM flashen

- [ ] Web-Dashboard

- [ ] Kommunikation ESP32 - Uno verstehen

## DOING

- [ ] (hierhin verschieben, was du gerade aktiv bearbeitest)

## DONE

- [x] Lektion 1: Enthüllung des Mars-Rovers
- [x] Lektion 2: Verständnis und Bau des Rocker-Bogie-Systems
- [x] Lektion 3: Einstieg in die Welt von Arduino und Programmierung
- [x] Lektion 4: Beherrschung des TT-Motors
- [x] Lektion 5: Entfesselung der Beweglichkeit des Mars Rovers
- [x] Lektion 6: Erkundung des Hindernisvermeidungsmoduls
- [x] Lektion 7: Verbesserung der Rover-Navigation mit Ultraschallmodul
- [x] Lektion 8: Fortgeschrittene Hindernisvermeidung und intelligentes Folgesystem
- [x] Lektion 9: Den Weg mit RGB-LED-Streifen beleuchten
- [x] Lektion 10: Erkundung des visuellen Systems - Servo und Kippmechanismus
- [x] Lektion 13: Untersuchung des Energiesystems des Mars-Rovers

- [x] SunFounder R3 Board
- [x] GalaxyRVR Shield
- [x] Ultraschallmodul
- [x] IR-Hindernisvermeidungsmodul
- [x] 4 RGB-LED-Streifen
- [x] Servo
- [x] TT-Motor
- [x] Solarpanel
- [x] 18650 Batterie

- Rover fährt autonom, stabilisiert sich selbst (Gierrate ~0.0) und weicht Hindernissen mit einer präzisen 180°-Wende aus. Das ist ein voll funktionsfähiges Robotik-System.
  - [x] Single Source of Truth (SSOT)
  - [x] Hardware: Alle Sensoren (IMU, Ultraschall, IR, Batterie) kalibriert und entstört.
  - [x] Architektur: Eine professionelle HAL-Struktur (Hardware Abstraction Layer), die leicht erweiterbar ist.
  - [x] Logik: Ein intelligenter Spurhalte-Assistent (P-Regler) und eine Manöver-Logik (Wende).
