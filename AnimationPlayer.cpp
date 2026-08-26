#include "AnimationPlayer.h"

AnimationPlayer::AnimationPlayer(RobotLeg* legs[LEG_COUNT]) {
  _legs = legs;
  _frames = nullptr;
  _frameCount = 0;
  _frameIndex = 0;
  _loop = false;
  _playing = false;
  _finished = true;
  for (uint8_t i = 0; i < LEG_COUNT; i++) {
    _currentPose[i] = {0, 0, 0};
  }
}

void AnimationPlayer::setCurrentPose(const LegPose pose[LEG_COUNT]) {
  for (uint8_t i = 0; i < LEG_COUNT; i++) {
    _currentPose[i] = pose[i];
  }
}

void AnimationPlayer::play(const Keyframe* frames, uint8_t count, bool loop) {
  if (count == 0) return;
  _frames = frames;
  _frameCount = count;
  _loop = loop;
  _frameIndex = 0;
  _playing = true;
  _finished = false;
  beginTransition();
}

void AnimationPlayer::stop() {
  _playing = false;
}

void AnimationPlayer::beginTransition() {
  // Переход всегда стартует из последней реально достигнутой позы,
  // поэтому смена анимации на лету не даёт рывка.
  for (uint8_t i = 0; i < LEG_COUNT; i++) {
    _frameStartPose[i] = _currentPose[i];
  }
  _frameStartMs = millis();
}

void AnimationPlayer::update() {
  if (!_playing) return;

  Keyframe kf;
  memcpy_P(&kf, &_frames[_frameIndex], sizeof(Keyframe));

  unsigned long elapsed = millis() - _frameStartMs;
  float t = (kf.duration_ms == 0)
              ? 1.0f
              : (float)elapsed / (float)kf.duration_ms;
  if (t > 1.0f) t = 1.0f;
  float eased = applyEasing(t, kf.easing);

  for (uint8_t i = 0; i < LEG_COUNT; i++) {
    LegPose start  = _frameStartPose[i];
    LegPose target = kf.legs[i];

    LegPose p;
    p.x = start.x + (target.x - start.x) * eased;
    p.y = start.y + (target.y - start.y) * eased;
    p.z = start.z + (target.z - start.z) * eased;

    // Если точка недостижима, moveTo вернёт false — считаем,
    // что нога осталась в предыдущей позиции, и не обновляем кэш.
    if (_legs[i]->moveTo(p.x, p.y, p.z)) {
      _currentPose[i] = p;
    }
  }

  if (t >= 1.0f) {
    _frameIndex++;
    if (_frameIndex >= _frameCount) {
      if (_loop) {
        _frameIndex = 0;
      } else {
        _playing = false;
        _finished = true;
        return;
      }
    }
    beginTransition();
  }
}
