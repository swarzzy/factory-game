#pragma once

u32 AtomicCompareExhange(u32 volatile* dest, u32 comp, u32 newValue);
u32 AtomicExchange(u32 volatile* dest, u32 value);
u32 AtomicIncrement(u32 volatile* dest);

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

f32 Ceil(f32 v) {
    return ceilf(v);
}

f32 Round(f32 v) {
    return roundf(v);
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
