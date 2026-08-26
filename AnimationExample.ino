#include "RobotLeg.h"
#include "AnimationController.h"

// ---- лапы: создание объектов оставляю как у вас в проекте ----
RobotLeg legFL(/* ваши параметры/пины */);
RobotLeg legFR(/* ... */);
RobotLeg legBL(/* ... */);
RobotLeg legBR(/* ... */);

RobotLeg* legs[4] = { &legFL, &legFR, &legBL, &legBR };
AnimationController anim(legs);

// Порядок лап во ВСЕХ кадрах ниже: FL, FR, BL, BR — как в массиве legs[]

// ===================== Анимация "стоять" (idle) =====================
// static const — обязательно, иначе указатель на массив станет
// невалидным после выхода из setup()
static const Keyframe kfIdle[] = {
  { { {0,0,-100}, {0,0,-100}, {0,0,-100}, {0,0,-100} }, 400 }
};
Animation animIdle(kfIdle, 1, /*loop=*/true, "idle");

// ===================== Анимация "шаг" (упрощённый trot) =====================
// 4 фазы: по диагонали пары лап поочерёдно отрываются от земли.
// Координаты — ЗАГЛУШКИ, замените на реально достижимые точки вашей ИК.
static const Keyframe kfWalk[] = {
  // фаза 0: FL+BR в воздухе (вынос вперёд), FR+BL толкают
  { { {20,0,-80}, {-20,0,-100}, {-20,0,-100}, {20,0,-80} }, 150 },
  // фаза 1: все на земле, корпус смещается вперёд
  { { {0,0,-100}, {0,0,-100}, {0,0,-100}, {0,0,-100} }, 100 },
  // фаза 2: FR+BL в воздухе, FL+BR толкают
  { { {-20,0,-100}, {20,0,-80}, {20,0,-80}, {-20,0,-100} }, 150 },
  // фаза 3: все на земле
  { { {0,0,-100}, {0,0,-100}, {0,0,-100}, {0,0,-100} }, 100 },
};
Animation animWalk(kfWalk, 4, /*loop=*/true, "walk");

// ===================== Анимация "сесть" (одноразовая) =====================
static const Keyframe kfSit[] = {
  { { {0,0,-100}, {0,0,-100}, {0,0,-100}, {0,0,-100} }, 300 },
  { { {0,20,-60}, {0,20,-60}, {0,-10,-100}, {0,-10,-100} }, 500 },
};
Animation animSit(kfSit, 2, /*loop=*/false, "sit");

void onAnimFinished(const Animation* a) {
  Serial.print("Анимация завершена: ");
  Serial.println(a->name());
}

void setup() {
  Serial.begin(115200);

  // Если знаете реальную стартовую позу лап — задайте её явно,
  // иначе первый переход посчитается от (0,0,0).
  // LegPos startPose[4] = { {0,0,-100}, {0,0,-100}, {0,0,-100}, {0,0,-100} };
  // anim.setCurrentPose(startPose);

  anim.setEasing(Easing::EASE_IN_OUT);
  anim.setOnFinished(onAnimFinished);

  anim.play(&animIdle, 500); // плавно встать в стойку за 500 мс
}

void loop() {
  anim.update(); // обязательно вызывать каждый цикл

  if (Serial.available()) {
    char c = Serial.read();
    switch (c) {
      case 'w': anim.play(&animWalk, 250); break; // плавный переход в ходьбу за 250 мс
      case 's': anim.play(&animIdle, 250); break; // вернуться в стойку
      case 'd': anim.play(&animSit,  400); break; // сесть (одноразово)
      case '+': anim.setSpeedMultiplier(1.5f); break; // ускорить анимации
      case '-': anim.setSpeedMultiplier(0.7f); break; // замедлить
    }
  }
}
