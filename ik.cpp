#include <iostream>
#include <cmath>

struct LegAngles {
    float q1;   // hip yaw
    float q2;   // hip pitch
    float q3;   // knee
    bool valid;
};

float clamp(float value, float minValue, float maxValue)
{
    if (value < minValue) return minValue;
    if (value > maxValue) return maxValue;
    return value;
}

LegAngles inverseKinematics(
    float x, float y, float z,
    float L1, float L2,
    bool kneeForward = true
) {
    LegAngles result;
    result.valid = false;

    // 1. Поворот первого сустава
    float q1 = atan2(y, x);

    // Расстояние до точки в вертикальной плоскости
    float r = sqrt(x * x + y * y);

    // 2. Закон косинусов
    float D = (r * r + z * z - L1 * L1 - L2 * L2)
            / (2.0f * L1 * L2);

    // Точка находится вне рабочей области
    if (D < -1.0f || D > 1.0f) {
        return result;
    }

    // Защита от ошибок float
    D = clamp(D, -1.0f, 1.0f);

    float sinQ3 = sqrt(1.0f - D * D);

    if (!kneeForward)
        sinQ3 = -sinQ3;

    // 3. Угол колена
    float q3 = atan2(sinQ3, D);

    // 4. Угол бедра
    float q2 = atan2(z, r)
             - atan2(
                 L2 * sin(q3),
                 L1 + L2 * cos(q3)
               );

    result.q1 = q1;
    result.q2 = q2;
    result.q3 = q3;
    result.valid = true;

    return result;
}

float radToDeg(float rad) {
    return rad * 180.0f /  3.14159265359;
}

int main() {
    LegAngles a = inverseKinematics(
        0,   // x, м
        2,   // y, м
        0,   // z, м
        1,   // L1, м
        1    // L2, м
    );

    if (a.valid) {
        std::cout << radToDeg(a.q1) << std::endl;

        std::cout << radToDeg(a.q2) << std::endl;

        std::cout << radToDeg(a.q3) << std::endl;
    } else {
        std::cout << "not valid" << std::endl;
    }

    return 0;
}
