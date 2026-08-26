#include "PredefinedAnimations.h"

// ВНИМАНИЕ: координаты ниже — заглушки для демонстрации структуры.
// Замените на реальные значения под геометрию вашей робособаки
// (длины звеньев, высота нейтральной стойки и т.д.)

const Keyframe ANIM_STAND[] PROGMEM = {
  {
    { {60,0,-90}, {60,0,-90}, {-60,0,-90}, {-60,0,-90} },
    500, EASE_IN_OUT_SINE
  }
};
const uint8_t ANIM_STAND_LEN = sizeof(ANIM_STAND) / sizeof(Keyframe);

const Keyframe ANIM_SIT[] PROGMEM = {
  // Задние лапы подгибаются, корпус садится назад
  {
    { {60,0,-90}, {60,0,-90}, {-30,0,-50}, {-30,0,-50} },
    600, EASE_IN_OUT_SINE
  },
  // Фиксация позы
  {
    { {60,0,-90}, {60,0,-90}, {-20,0,-40}, {-20,0,-40} },
    300, EASE_LINEAR
  }
};
const uint8_t ANIM_SIT_LEN = sizeof(ANIM_SIT) / sizeof(Keyframe);

const Keyframe ANIM_STRETCH[] PROGMEM = {
  // Классическая "собачья потяжка": перед вниз, зад вверх
  {
    { {90,0,-70}, {90,0,-70}, {-40,0,-100}, {-40,0,-100} },
    700, EASE_IN_OUT_QUAD
  },
  // Возврат в нейтральную стойку
  {
    { {60,0,-90}, {60,0,-90}, {-60,0,-90}, {-60,0,-90} },
    500, EASE_IN_OUT_QUAD
  }
};
const uint8_t ANIM_STRETCH_LEN = sizeof(ANIM_STRETCH) / sizeof(Keyframe);
