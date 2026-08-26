#pragma once
#include "RobotAnimTypes.h"
#include "RobotLeg.h"   // ваш существующий класс с методом moveTo(x,y,z)

// Проигрывает последовательность Keyframe с плавной интерполяцией
// между ними. Не блокирует loop() — update() нужно вызывать каждую итерацию.
class AnimationPlayer {
public:
  explicit AnimationPlayer(RobotLeg* legs[LEG_COUNT]);

  // frames обычно лежит в PROGMEM (см. PredefinedAnimations.cpp)
  void play(const Keyframe* frames, uint8_t count, bool loop = false);
  void stop();
  void update();

  bool isPlaying()  const { return _playing; }
  bool isFinished() const { return _finished; }

  // Нужно вызвать один раз после старта, чтобы первая анимация
  // начиналась из реальной текущей позы, а не из (0,0,0).
  void setCurrentPose(const LegPose pose[LEG_COUNT]);

private:
  RobotLeg** _legs;

  const Keyframe* _frames;
  uint8_t _frameCount;
  uint8_t _frameIndex;
  bool    _loop;
  bool    _playing;
  bool    _finished;

  unsigned long _frameStartMs;
  LegPose _frameStartPose[LEG_COUNT]; // поза в начале текущего перехода
  LegPose _currentPose[LEG_COUNT];    // последняя реально отправленная поза

  void beginTransition();
};
