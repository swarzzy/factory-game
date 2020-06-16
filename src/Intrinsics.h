#pragma once

u32 AtomicCompareExchange(u32 volatile* dest, u32 comp, u32 newValue);
u32 AtomicExchange(u32 volatile* dest, u32 value);
u32 AtomicIncrement(u32 volatile* dest);

u32 ThreadSleep(u32 ms);

// NOTE: https://graphics.stanford.edu/~seander/bithacks.html#RoundUpPowerOf2
constexpr u32 NextPowerOfTwo(u32 v) {
    v--;
    v |= v >> 1;
    v |= v >> 2;
    v |= v >> 4;
    v |= v >> 8;
    v |= v >> 16;
    v++;
    return v;
}

constexpr bool IsPowerOfTwo(u32 n) {
    bool result = ((n & (n - 1)) == 0);
    return result;
}

bool IsNegative(f32 value) {
    bool result;
    auto sign = signbit(value);
    if (sign) {
        result = true;
    } else {
        result = false;
    }
    return result;
}

f32 Pow(f32 base, f32 exp) {
    return powf(base, exp);
}

template<typename T>
constexpr T Min(T a, T b) {
    return a < b ? a : b;
}

template<typename T>
constexpr T Max(T a, T b) {
    return a > b ? a : b;
}

template<typename T>
constexpr T Clamp(T x, T min, T max) {
    return Min(Max(x, min), max);
}

template<typename T>
constexpr T Saturate(T x) {
    return Min(Max(x, static_cast<T>(0)), static_cast<T>(1));
}

template<typename T>
constexpr T Abs(T val) {
    return val >= static_cast<T>(0) ? val : -val;
}

template<typename T>
constexpr f32 Lerp(T a, T b, f32 t) {
    return (1.0f - t) * a + t * b;
}

f32 SmoothStep(f32 e0, f32 e1, f32 x) {
    f32 t = Saturate((x - e0) / (e1 - e0));
    f32 result = t * t * (3.0f - 2.0f * t);
    return result;
}

constexpr f32 ToDeg(f32 rad) {
    return 180.0f / F32::Pi * rad;
}

constexpr f32 ToRad(f32 deg) {
    return F32::Pi / 180.0f * deg;
}

f32 Sin(f32 rad) {
    return sinf(rad);
}

f32 Cos(f32 rad) {
    return cosf(rad);
}

f32 Tan(f32 rad) {
    return tanf(rad);
}

f32 Acos(f32 v) {
    return acosf(v);
}

f32 Atan2(f32 n, f32 d) {
    return atan2f(n, d);
}

f32 Sqrt(f32 v) {
    return sqrtf(v);
}

f32 Floor(f32 v) {
    return floorf(v);
}

f64 Floor(f64 v) {
    return floor(v);
}

f32 Ceil(f32 v) {
    return ceilf(v);
}

f32 Round(f32 v) {
    return roundf(v);
}

constexpr f32 SafeRatioN(f32 num, f32 div, f32 n) {
    f32 result = n;
    if (div != 0.0f) {
        result = num / div;
    }
    return result;
}

constexpr f32 SafeRatio0(f32 num, f32 div) {
    f32 result = SafeRatioN(num, div, 0.0f);
    return result;
}


constexpr u32 StrLength(const char* string) {
    u32 result = 0;
    while (*string) {
        result++;
        string++;
    }
    return result;
}

constexpr u32 StrSize(const char* string) {
    u32 result = 0;
    while (*string) {
        result++;
        string++;
    }
    result++;
    result *= sizeof(char);
    return result;
}
