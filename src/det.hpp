#pragma once

namespace det {

constexpr float PI_F      = 3.14159265358979323846f;
constexpr float TWO_PI_F  = 6.28318530717958647692f;
constexpr float HALF_PI_F = 1.57079632679489661923f;
constexpr float INV_TWO_PI_F = 0.15915494309189533577f;

constexpr float FIXED_DT = 1.0f / 120.0f;

inline float Sin(float x) {
    double xd = (double)x;
    double q  = xd * 0.15915494309189533577;
    double k  = (double)(long long)(q >= 0.0 ? q + 0.5 : q - 0.5);
    x = (float)(xd - k * 6.283185307179586476925286766559);

    const float S1 = -1.66666666e-01f;
    const float S2 =  8.33333310e-03f;
    const float S3 = -1.98408747e-04f;
    const float S4 =  2.75255578e-06f;
    const float S5 = -2.38898590e-08f;

    float x2 = x * x;
    return x * (1.0f + x2 * (S1 + x2 * (S2 + x2 * (S3 + x2 * (S4 + x2 * S5)))));
}

inline float Cos(float x) {
    return Sin(x + HALF_PI_F);
}

inline float Atan(float z) {
    const float A1 =  9.99866000e-01f;
    const float A2 = -3.30299500e-01f;
    const float A3 =  1.80141000e-01f;
    const float A4 = -8.51330000e-02f;
    const float A5 =  2.08351000e-02f;
    float z2 = z * z;
    return z * (A1 + z2 * (A2 + z2 * (A3 + z2 * (A4 + z2 * A5))));
}

inline float Atan2(float y, float x) {
    if (x == 0.0f && y == 0.0f) return 0.0f;
    float ax = x < 0.0f ? -x : x;
    float ay = y < 0.0f ? -y : y;
    float a, r;
    if (ax >= ay) {
        a = Atan(ay / ax);
    } else {
        a = HALF_PI_F - Atan(ax / ay);
    }
    r = (x < 0.0f) ? (PI_F - a) : a;
    return (y < 0.0f) ? -r : r;
}

inline float Fmod(float x, float m) {
    if (m == 0.0f) return 0.0f;
    float q = x / m;
    float k = (float)(int)q;
    return x - k * m;
}

}
