#pragma once
#include <Arduino.h>

namespace HAL::Sensor {
void init();
void ultrasonicTest();
void rgbTest();

bool irLeftBlocked();
bool irRightBlocked();
void irTest();

void setCameraAngle(int angleDeg);
void servoTest();

float getBatteryVoltage();
uint8_t getBatteryPercentage();
bool isBatteryCharging();

} // namespace HAL::Sensor
