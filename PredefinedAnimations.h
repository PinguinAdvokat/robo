#pragma once
#include "RobotAnimTypes.h"

// Данные хранятся в PROGMEM (экономия SRAM). Значения координат
// иллюстративные — подставьте те, что подходят геометрии вашего робота.

extern const Keyframe ANIM_STAND[] PROGMEM;
extern const uint8_t  ANIM_STAND_LEN;

extern const Keyframe ANIM_SIT[] PROGMEM;
extern const uint8_t  ANIM_SIT_LEN;

extern const Keyframe ANIM_STRETCH[] PROGMEM;
extern const uint8_t  ANIM_STRETCH_LEN;
