#include "RobotLeg.h"          // ваш существующий класс с IK
#include "RobotAnimTypes.h"
#include "AnimationPlayer.h"
#include "GaitEngine.h"
#include "RobotDogController.h"
#include "PredefinedAnimations.h"

RobotLeg legFL, legFR, legBL, legBR;
RobotLeg* legs[LEG_COUNT] = { &legFL, &legFR, &legBL, &legBR };

// Нейтральная стойка — точка опоры каждой лапы в её локальных координатах
const LegPose neutralStance[LEG_COUNT] = {
  {60, 0, -90}, {60, 0, -90}, {-60, 0, -90}, {-60, 0, -90}
};

// Координаты крепления лап относительно центра корпуса (мм) —
// нужны GaitEngine для расчёта поворота (omega)
const float legMountXY[LEG_COUNT][2] = {
  { 80,  50}, { 80, -50}, {-80,  50}, {-80, -50}
};

RobotDogController robot(legs, neutralStance, legMountXY);

void setup() {
  Serial.begin(115200);
  // ... здесь инициализация сервоприводов/RobotLeg, если она нужна

  robot.idle(); // встать в нейтральную стойку
}

void loop() {
  robot.update(); // обязательно вызывать каждую итерацию, неблокирующе

  // Команды через Serial для демонстрации
  if (Serial.available()) {
    char c = Serial.read();
    switch (c) {
      case 's': robot.playAnimation(ANIM_SIT, ANIM_SIT_LEN); break;
      case 't': robot.playAnimation(ANIM_STAND, ANIM_STAND_LEN); break;
      case 'r': robot.playAnimation(ANIM_STRETCH, ANIM_STRETCH_LEN); break;
      case 'w': robot.walk(40.0f, 0.0f, 0.0f, GAIT_TROT); break;   // вперёд трусцой
      case 'a': robot.walk(0.0f, 0.0f, 1.0f, GAIT_WALK); break;    // поворот на месте
      case 'i': robot.idle(); break;                               // стоп
    }
  }
}
