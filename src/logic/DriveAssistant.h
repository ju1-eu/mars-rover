#pragma once
#include <stdint.h>

namespace Logic::DriveAssistant {

/** * @brief Setzt interne Parameter zurück.
 */
void init();

/**
 * @brief Fährt geradeaus und korrigiert Abweichungen (Gieren) aktiv.
 * * @param baseSpeed Die gewünschte Geschwindigkeit (PWM).
 * @return true = Alles OK, false = Not-Aus (z.B. Kippwinkel zu groß).
 */
bool update(uint8_t baseSpeed);

} // namespace Logic::DriveAssistant
