#include "GaitEngine.h"

GaitEngine::GaitEngine(RobotLeg* legs[LEG_COUNT],
                        const LegPose neutralStance[LEG_COUNT],
                        const float legMountXY[LEG_COUNT][2]) {
  _legs = legs;
  for (uint8_t i = 0; i < LEG_COUNT; i++) {
    _neutral[i]    = neutralStance[i];
    _mountXY[i][0] = legMountXY[i][0];
    _mountXY[i][1] = legMountXY[i][1];
  }
  _vx = _vy = _omega = 0.0f;
  _stepHeight  = 30.0f;
  _cycleTimeMs = 600;
  _swingRatio  = 0.5f;
  _active = false;
  setGait(GAIT_TROT);
}

void GaitEngine::applyGaitOffsets(GaitType type) {
  switch (type) {
    case GAIT_PACE:
      _phaseOffset[LEG_FRONT_LEFT]  = 0.0f;
      _phaseOffset[LEG_BACK_LEFT]   = 0.0f;
      _phaseOffset[LEG_FRONT_RIGHT] = 0.5f;
      _phaseOffset[LEG_BACK_RIGHT]  = 0.5f;
      _swingRatio = 0.5f;
      break;

    case GAIT_BOUND:
      _phaseOffset[LEG_FRONT_LEFT]  = 0.0f;
      _phaseOffset[LEG_FRONT_RIGHT] = 0.0f;
      _phaseOffset[LEG_BACK_LEFT]   = 0.5f;
      _phaseOffset[LEG_BACK_RIGHT]  = 0.5f;
      _swingRatio = 0.5f;
      break;

    case GAIT_WALK:
      // волновая последовательность: в каждый момент минимум 3 лапы на земле
      _phaseOffset[LEG_BACK_LEFT]   = 0.00f;
      _phaseOffset[LEG_FRONT_LEFT]  = 0.25f;
      _phaseOffset[LEG_BACK_RIGHT]  = 0.50f;
      _phaseOffset[LEG_FRONT_RIGHT] = 0.75f;
      _swingRatio = 0.25f;
      break;

    case GAIT_TROT:
    default:
      _phaseOffset[LEG_FRONT_LEFT]  = 0.0f;
      _phaseOffset[LEG_BACK_RIGHT]  = 0.0f;
      _phaseOffset[LEG_FRONT_RIGHT] = 0.5f;
      _phaseOffset[LEG_BACK_LEFT]   = 0.5f;
      _swingRatio = 0.5f;
      break;
  }
}

void GaitEngine::setGait(GaitType type) {
  _gait = type;
  applyGaitOffsets(type);
}

void GaitEngine::setVelocity(float vx, float vy, float omega) {
  _vx = vx; _vy = vy; _omega = omega;
}

void GaitEngine::setStepHeight(float h)    { _stepHeight = h; }
void GaitEngine::setCycleTime(uint16_t ms) { _cycleTimeMs = ms; }
void GaitEngine::setSwingRatio(float r)    { _swingRatio = r; }

void GaitEngine::start() {
  _active = true;
  _startMs = millis();
}

void GaitEngine::stop() {
  _active = false;
  for (uint8_t i = 0; i < LEG_COUNT; i++) {
    _legs[i]->moveTo(_neutral[i].x, _neutral[i].y, _neutral[i].z);
  }
}

LegPose GaitEngine::computeFootPosition(uint8_t i, float phase01) {
  // Требуемая скорость точки стопы с учётом вращения корпуса:
  // v_point = v_body + omega x r, где r — плечо от центра до крепления лапы.
  float dirX = _vx - _omega * _mountXY[i][1];
  float dirY = _vy + _omega * _mountXY[i][0];

  float mag = sqrt(dirX * dirX + dirY * dirY);
  float nx = (mag > 0.0001f) ? dirX / mag : 0.0f;
  float ny = (mag > 0.0001f) ? dirY / mag : 0.0f;

  float stanceTimeSec = (_cycleTimeMs * (1.0f - _swingRatio)) / 1000.0f;
  float stepLength = mag * stanceTimeSec;
  float halfStep = stepLength / 2.0f;

  LegPose p = _neutral[i];

  if (phase01 < _swingRatio) {
    // Фаза переноса: лапа в воздухе, идёт вперёд по дуге (подъём — синус)
    float u = phase01 / _swingRatio;
    float along = -halfStep + halfStep * 2.0f * u;
    p.x = _neutral[i].x + nx * along;
    p.y = _neutral[i].y + ny * along;
    // Знак/ось для "вверх" зависит от конвенции координат вашей IK —
    // при необходимости замените + на - или поменяйте ось.
    p.z = _neutral[i].z + _stepHeight * sin(PI * u);
  } else {
    // Опорная фаза: лапа на земле, едет назад относительно корпуса —
    // это и есть шаг, толкающий корпус вперёд.
    float u = (phase01 - _swingRatio) / (1.0f - _swingRatio);
    float along = halfStep - halfStep * 2.0f * u;
    p.x = _neutral[i].x + nx * along;
    p.y = _neutral[i].y + ny * along;
    p.z = _neutral[i].z;
  }

  return p;
}

void GaitEngine::update() {
  if (!_active) return;

  unsigned long elapsed = millis() - _startMs;
  float globalPhase = fmod((float)elapsed / (float)_cycleTimeMs, 1.0f);

  for (uint8_t i = 0; i < LEG_COUNT; i++) {
    float phase = fmod(globalPhase + _phaseOffset[i], 1.0f);
    LegPose target = computeFootPosition(i, phase);
    // Если точка недостижима, moveTo вернёт false — в проде стоит
    // залогировать это (может означать, что шаг/скорость выбраны
    // слишком агрессивно для геометрии ноги).
    _legs[i]->moveTo(target.x, target.y, target.z);
  }
}
