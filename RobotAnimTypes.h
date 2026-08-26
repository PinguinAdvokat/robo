#pragma once
#include <Arduino.h>

// Индексы лап. Порядок и mount-координаты (см. legMountXY в примере)
// должны соответствовать реальному расположению лап на корпусе.
enum LegIndex : uint8_t {
  LEG_FRONT_LEFT  = 0,
  LEG_FRONT_RIGHT = 1,
  LEG_BACK_LEFT   = 2,
  LEG_BACK_RIGHT  = 3,
  LEG_COUNT       = 4
};

// Целевая точка стопы в локальных координатах ноги — те же (x,y,z),
// что принимает RobotLeg::moveTo(x, y, z).
struct LegPose {
  float x, y, z;
};

enum EasingType : uint8_t {
  EASE_LINEAR,
  EASE_IN_OUT_QUAD,
  EASE_IN_OUT_SINE
};

// Кадр анимации: целевая поза всех 4 лап + время перехода к ней.
struct Keyframe {
  LegPose  legs[LEG_COUNT];
  uint16_t duration_ms;
  EasingType easing;
};

// t из [0..1] -> сглаженное значение [0..1]
inline float applyEasing(float t, EasingType type) {
  switch (type) {
    case EASE_IN_OUT_QUAD: {
      float f = -2.0f * t + 2.0f;
      return t < 0.5f ? 2.0f * t * t : 1.0f - (f * f) / 2.0f;
    }
    case EASE_IN_OUT_SINE:
      return -(cos(PI * t) - 1.0f) / 2.0f;
    case EASE_LINEAR:
    default:
      return t;
  }
}
