#pragma once
#include "AnimationPlayer.h"
#include "GaitEngine.h"

enum ControllerMode : uint8_t {
  MODE_IDLE,
  MODE_ANIMATION,
  MODE_GAIT
};

// Единая точка входа: не даёт AnimationPlayer и GaitEngine одновременно
// слать команды одним и тем же ногам, переключает режимы.
class RobotDogController {
public:
  RobotDogController(RobotLeg* legs[LEG_COUNT],
                      const LegPose neutralStance[LEG_COUNT],
                      const float legMountXY[LEG_COUNT][2]);

  void update(); // вызывать каждую итерацию loop()

  // Заранее заданная анимация (сесть/встать/помахать лапой и т.п.)
  void playAnimation(const Keyframe* frames, uint8_t count, bool loop = false);

  // Непрерывная ходьба
  void walk(float vx, float vy, float omega, GaitType gait = GAIT_TROT);
  void stopWalking();

  // Остановить всё и встать в нейтральную стойку
  void idle();

  ControllerMode getMode() const { return _mode; }

private:
  RobotLeg** _legs;
  AnimationPlayer _animPlayer;
  GaitEngine _gaitEngine;
  ControllerMode _mode;
  LegPose _neutral[LEG_COUNT];
};
