#pragma once
#include <Arduino.h>
#include "RobotLeg.h"   // подключите ваш существующий файл с классом RobotLeg (метод moveTo(x,y,z))

// =====================================================================
//  Позиция одной лапы в системе координат обратной кинематики
// =====================================================================
struct LegPos {
  float x, y, z;
};

// =====================================================================
//  Один кадр анимации.
//  legs[4]      — целевые точки для всех 4 лап (порядок как в массиве
//                 RobotLeg*, который вы передаёте в AnimationController)
//  duration_ms  — время движения ИЗ ПРЕДЫДУЩЕГО кадра В ЭТОТ.
//                 Для kf(0) это поле используется только при зацикливании
//                 (переход из последнего кадра обратно в первый).
//                 При первом входе в анимацию используется transitionMs
//                 из play(), а не duration_ms кадра 0.
// =====================================================================
struct Keyframe {
  LegPos legs[4];
  uint16_t duration_ms;
};

enum class Easing : uint8_t {
  LINEAR,       // постоянная скорость
  EASE_IN_OUT,  // плавный разгон и торможение (по умолчанию)
  EASE_OUT      // резкий старт, мягкая остановка
};

// =====================================================================
//  Описание анимации: набор кадров + флаг зацикливания.
//  Массив keyframes должен жить всё время использования анимации —
//  объявляйте его как global или как "static const" внутри setup(),
//  иначе указатель станет невалидным после выхода из setup().
// =====================================================================
class Animation {
public:
  Animation(const Keyframe* keyframes, uint8_t count, bool loop = true, const char* name = "")
    : _kf(keyframes), _count(count), _loop(loop), _name(name) {}

  const Keyframe& kf(uint8_t i) const { return _kf[i]; }
  uint8_t count() const { return _count; }
  bool loop() const { return _loop; }
  const char* name() const { return _name; }

private:
  const Keyframe* _kf;
  uint8_t _count;
  bool _loop;
  const char* _name;
};

// =====================================================================
//  Контроллер анимаций.
//  Вызывайте update() в каждом loop().
// =====================================================================
class AnimationController {
public:
  explicit AnimationController(RobotLeg* legs[4]) {
    for (uint8_t i = 0; i < 4; i++) _legs[i] = legs[i];
  }

  // Задать реальную текущую позу лап (например, сразу после калибровки),
  // чтобы самый первый play() строил переход не от нулевых координат.
  void setCurrentPose(const LegPos pose[4]) {
    for (uint8_t i = 0; i < 4; i++) _currentPose[i] = pose[i];
  }

  void setEasing(Easing e) { _easing = e; }

  // Множитель скорости воспроизведения: 2.0 — вдвое быстрее, 0.5 — вдвое медленнее.
  void setSpeedMultiplier(float m) { _speed = (m > 0.01f) ? m : 0.01f; }

  // Необязательный колбэк — вызывается один раз, когда незацикленная
  // анимация доиграла до конца (удобно для связки "sit -> потом idle").
  void setOnFinished(void (*cb)(const Animation*)) { _onFinished = cb; }

  // Запустить анимацию.
  // transitionMs — длительность плавного перехода от ТЕКУЩЕЙ позы лап
  // к первому кадру новой анимации. Можно вызывать в любой момент,
  // в том числе поверх уже играющей анимации — переход всегда строится
  // от актуальной интерполированной позы, поэтому рывков не будет.
  void play(const Animation* anim, uint16_t transitionMs = 300) {
    if (!anim || anim->count() == 0) return;
    if (_anim == anim && !_finished) return; // эта анимация уже играет — ничего не делаем

    _anim = anim;
    _entering = true;
    _finished = false;
    for (uint8_t i = 0; i < 4; i++) _segFrom[i] = _currentPose[i];
    _segTo = &anim->kf(0);
    _segDuration = transitionMs;
    _segStart = millis();
  }

  void stop() {
    _anim = nullptr;
    _finished = true;
  }

  bool isPlaying() const { return _anim && !_finished; }
  bool isFinished() const { return _finished; }
  const Animation* current() const { return _anim; }

  // Вызывать каждый loop().
  void update() {
    if (!_anim || _finished) return;

    unsigned long now = millis();
    float t;
    if (_segDuration == 0) {
      t = 1.0f;
    } else {
      t = (float)(now - _segStart) / ((float)_segDuration / _speed);
      t = constrain(t, 0.0f, 1.0f);
    }

    float te = ease(t);

    for (uint8_t i = 0; i < 4; i++) {
      LegPos p = lerp(_segFrom[i], _segTo->legs[i], te);
      _currentPose[i] = p;
      _legs[i]->moveTo(p.x, p.y, p.z);
    }

    if (t >= 1.0f) advanceSegment(now);
  }

private:
  RobotLeg* _legs[4];
  const Animation* _anim = nullptr;

  LegPos _currentPose[4] = { {0,0,0},{0,0,0},{0,0,0},{0,0,0} };
  LegPos _segFrom[4];
  const Keyframe* _segTo = nullptr;

  uint8_t _targetIdx = 0;
  bool _entering = false;
  bool _finished = true;

  unsigned long _segStart = 0;
  uint16_t _segDuration = 0;

  Easing _easing = Easing::EASE_IN_OUT;
  float _speed = 1.0f;

  void (*_onFinished)(const Animation*) = nullptr;

  void advanceSegment(unsigned long now) {
    uint8_t n = _anim->count();

    if (_entering) {
      // только что доехали до kf(0)
      _entering = false;
      if (n <= 1) {
        _finished = true;
        if (_onFinished) _onFinished(_anim);
        return;
      }
      _targetIdx = 1;
    } else {
      uint8_t next = (_targetIdx + 1) % n;
      if (next == 0 && !_anim->loop()) {
        _finished = true; // остаёмся в последней позе
        if (_onFinished) _onFinished(_anim);
        return;
      }
      _targetIdx = next;
    }

    for (uint8_t i = 0; i < 4; i++) _segFrom[i] = _currentPose[i];
    _segTo = &_anim->kf(_targetIdx);
    _segDuration = _segTo->duration_ms;
    _segStart = now;
  }

  float ease(float t) const {
    switch (_easing) {
      case Easing::LINEAR:   return t;
      case Easing::EASE_OUT: return 1.0f - (1.0f - t) * (1.0f - t);
      default:                return t * t * (3.0f - 2.0f * t); // EASE_IN_OUT (smoothstep)
    }
  }

  static LegPos lerp(const LegPos& a, const LegPos& b, float t) {
    return LegPos{ a.x + (b.x - a.x) * t, a.y + (b.y - a.y) * t, a.z + (b.z - a.z) * t };
  }
};
