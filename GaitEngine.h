#pragma once
#include "RobotAnimTypes.h"
#include "RobotLeg.h"

enum GaitType : uint8_t {
  GAIT_TROT,   // диагональные пары синхронны — быстрая, динамически устойчивая
  GAIT_WALK,   // волновая походка, всегда 3 опорные ноги — самая устойчивая
  GAIT_PACE,   // синхронны лапы одной стороны
  GAIT_BOUND   // синхронны передняя/задняя пары
};

// Процедурно генерирует траекторию каждой лапы по фазе цикла шага.
// В отличие от AnimationPlayer работает не с заранее заданными кадрами,
// а вычисляет позицию "на лету" исходя из скорости и типа походки —
// это то, что нужно для непрерывной ходьбы/поворота.
class GaitEngine {
public:
  // neutralStance  — точка опоры лапы в нейтральной стойке (локальные коорд. ноги)
  // legMountXY     — координаты крепления каждой лапы относительно центра корпуса,
  //                  нужны для расчёта поворота (omega)
  GaitEngine(RobotLeg* legs[LEG_COUNT],
             const LegPose neutralStance[LEG_COUNT],
             const float legMountXY[LEG_COUNT][2]);

  void setGait(GaitType type);
  void setVelocity(float vx, float vy, float omega); // vx/vy — мм/с вперёд/вбок, omega — рад/с
  void setStepHeight(float h);       // высота подъёма лапы в фазе переноса
  void setCycleTime(uint16_t ms);    // длительность полного цикла шага
  void setSwingRatio(float ratio);   // доля цикла на фазу переноса (0..1)

  void start();
  void stop();                       // останавливает и возвращает лапы в нейтральную стойку
  bool isActive() const { return _active; }

  void update(); // вызывать каждую итерацию loop()

private:
  RobotLeg** _legs;
  LegPose _neutral[LEG_COUNT];
  float   _mountXY[LEG_COUNT][2];
  float   _phaseOffset[LEG_COUNT];

  GaitType _gait;
  float _vx, _vy, _omega;
  float _stepHeight;
  uint16_t _cycleTimeMs;
  float _swingRatio;

  bool _active;
  unsigned long _startMs;

  void applyGaitOffsets(GaitType type);
  LegPose computeFootPosition(uint8_t legIdx, float phase01);
};
