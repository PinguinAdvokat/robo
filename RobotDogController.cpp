#include "RobotDogController.h"

RobotDogController::RobotDogController(RobotLeg* legs[LEG_COUNT],
                                         const LegPose neutralStance[LEG_COUNT],
                                         const float legMountXY[LEG_COUNT][2])
  : _animPlayer(legs), _gaitEngine(legs, neutralStance, legMountXY) {
  _legs = legs;
  _mode = MODE_IDLE;
  for (uint8_t i = 0; i < LEG_COUNT; i++) {
    _neutral[i] = neutralStance[i];
  }
  // Чтобы первая анимация не дёрнула ноги из (0,0,0),
  // считаем стартовой позой нейтральную стойку.
  _animPlayer.setCurrentPose(_neutral);
}

void RobotDogController::playAnimation(const Keyframe* frames, uint8_t count, bool loop) {
  _gaitEngine.stop();
  _mode = MODE_ANIMATION;
  _animPlayer.play(frames, count, loop);
}

void RobotDogController::walk(float vx, float vy, float omega, GaitType gait) {
  _animPlayer.stop();
  _gaitEngine.setGait(gait);
  _gaitEngine.setVelocity(vx, vy, omega);
  if (_mode != MODE_GAIT) {
    _gaitEngine.start();
  }
  _mode = MODE_GAIT;
}

void RobotDogController::stopWalking() {
  _gaitEngine.stop();
  _mode = MODE_IDLE;
}

void RobotDogController::idle() {
  _animPlayer.stop();
  _gaitEngine.stop();
  _mode = MODE_IDLE;
}

void RobotDogController::update() {
  switch (_mode) {
    case MODE_ANIMATION:
      _animPlayer.update();
      if (_animPlayer.isFinished()) {
        _mode = MODE_IDLE;
      }
      break;
    case MODE_GAIT:
      _gaitEngine.update();
      break;
    default:
      break;
  }
}
