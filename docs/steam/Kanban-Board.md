---
title: "Kanban Board"
kanban.board: "Mars Rover"
status: "active"
phase: "2.0"
---

# 📋 Kanban Board

## TODO (Phase 2: Interaktion)

- [ ] **Konzept:** Zustandsautomat (FSM) für "Wall Follower" entwerfen
- [ ] **Logic:** "Bang-Bang"-Regler für Wandverfolgung implementieren
- [ ] **Safety:** Absturz-Sicherung (Cliff Detection) priorisieren

## DOING

- [ ] **HAL:** IR-Sensoren kalibrieren (Potentiometer auf 10-15cm einstellen)

## DONE (Phase 1: Präzision)

- [x] **Release v1.4.0:** Finalisierung der Basis-Firmware (Gold Master)
- [x] **Feature:** Odometrie (Koppelnavigation) mit Spannungskompensation
- [x] **Doku:** Vollständige Doxygen-Dokumentation (inkl. LaTeX-Formeln)
- [x] **Logic:** 180°-Wende mit Trägheitskompensation (Inertia) implementiert
- [x] **Logic:** Rechtsdrall durch Feed-Forward-Bias (`BIAS_TRIM`) behoben
- [x] **Logic:** PID-Regler getunt (Oszillation eliminiert, $K_p=0.6$)
- [x] **Architektur:** Schichtentrennung (HAL / Logic / App) etabliert
- [x] **HAL:** Treiber für Motoren, IMU (MPU6050) und Ultraschall erstellt
