---
title: "Kanban Board"
kanban.board: "Mars Rover"
status: "active"
phase: "2.0"
---

# 📋 Kanban Board

## TODO (Phase 2: Interaktion)

- [ ] **Konzept:** Zustandsautomat (FSM) für "Wall Follower" entwerfen
- [ ] **HAL:** Treiber für IR-Sensoren verfeinern (Digital vs. Analog?)
- [ ] **Logic:** "Bang-Bang"-Regler oder PID für Wandverfolgung implementieren
- [ ] **Safety:** Absturz-Sicherung (Cliff Detection) priorisieren

## DOING

- [ ] *Aktuell keine aktive Task – Bereit für Phase 2*

## DONE (Phase 1: Präzision)

- [x] **Release v1.2.0:** Finalisierung der Basis-Firmware
- [x] **Doku:** Vollständige Doxygen-Dokumentation (inkl. LaTeX-Formeln)
- [x] **Logic:** 180°-Wende mit Trägheitskompensation (Inertia) implementiert
- [x] **Logic:** Rechtsdrall durch Feed-Forward-Bias (`BIAS_TRIM`) behoben
- [x] **Logic:** PID-Regler getunt (Oszillation eliminiert, $K_p=0.6$)
- [x] **Architektur:** Schichtentrennung (HAL / Logic / App) etabliert
- [x] **HAL:** Treiber für Motoren, IMU (MPU6050) und Ultraschall erstellt
