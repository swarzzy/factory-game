#pragma once
#include <stdint.h>
#include <stdio.h>
#include <float.h>
// NOTE: offsetof
#include <stddef.h>
#include <stdarg.h>
#include <math.h>

#if defined(_MSC_VER)
#define COMPILER_MSVC
#elif defined(__clang__)
#define COMPILER_CLANG
#else
#error Unsupported compiler
#endif

#if defined(PLATFORM_WINDOWS)
#define debug_break() __debugbreak()
#elif defined(PLATFORM_LINUX)
#define debug_break() __builtin_debugtrap()
#endif

// TODO: Define these on other platforms
#include <intrin.h>
#define WriteFence() (_WriteBarrier(), _mm_sfence())
#define ReadFence() (_ReadBarrier(), _mm_lfence())

#define assert(expr, ...) do { if (!(expr)) {LogAssert(__FILE__, __func__, __LINE__, #expr, __VA_ARGS__); debug_break();}} while(false)
// NOTE: Defined always
#define panic(expr, ...) do { if (!(expr)) {LogAssert(__FILE__, __func__, __LINE__, #expr, __VA_ARGS__); debug_break();}} while(false)

#define array_count(arr) ((uint)(sizeof(arr) / sizeof(arr[0])))
#define typedecl(type, member) (((type*)0)->member)
#define invalid_default() default: { debug_break(); } break
#define unreachable() debug_break()

// NOTE: Jonathan Blow defer implementation. Reference: https://pastebin.com/SX3mSC9n
#define concat_internal(x,y) x##y
#define concat(x,y) concat_internal(x,y)

template<typename T>
struct ExitScope
{
    T lambda;
    ExitScope(T lambda) : lambda(lambda) {}
    ~ExitScope() { lambda(); }
    ExitScope(const ExitScope&);
  private:
    ExitScope& operator =(const ExitScope&);
};

class ExitScopeHelp
{
  public:
    template<typename T>
    ExitScope<T> operator+(T t) { return t; }
};

#define defer const auto& concat(defer__, __LINE__) = ExitScopeHelp() + [&]()

typedef uint8_t byte;
typedef unsigned char uchar;

typedef int8_t i8;
typedef int16_t i16;
typedef int32_t i32;
typedef int64_t i64;

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

typedef uintptr_t uptr;

typedef u32 b32;
typedef byte b8;

typedef float f32;
typedef double f64;

typedef u32 u32x;
typedef i32 i32x;
typedef u32 uint;

namespace Uptr {
    constexpr uptr Max = UINTPTR_MAX;
}

namespace F32 {
    constexpr f32 Pi = 3.14159265358979323846f;
    constexpr f32 Eps = 0.000001f;
    constexpr f32 Nan = NAN;
    constexpr f32 Max = FLT_MAX;
    constexpr f32 Min = FLT_MIN;
};

namespace I32 {
    constexpr i32 Max = INT32_MAX;
    constexpr i32 Min = INT32_MIN;
}

namespace U32 {
    constexpr u32 Max = 0xffffffff;
}

// NOTE: Allocator API
typedef void*(AllocateFn)(uptr size, uptr alignment, void* allocatorData);
typedef void(DeallocateFn)(void* ptr, void* allocatorData);

inline void LogAssertV(const char* file, const char* func, u32 line, const char* assertStr, const char* fmt = nullptr, va_list* args = nullptr)
{
    printf("[Assertion failed] Expression (%s) result is false\nFile: %s, function: %s, line: %d.\n", assertStr, file, func, (int)line);
    if (fmt && args)
    {
        printf("Message: ");
        vprintf(fmt, *args);
    }
}

inline void LogAssert(const char* file, const char* func, u32 line, const char* assertStr)
{
    LogAssertV(file, func, line, assertStr, nullptr, nullptr);
}


inline void LogAssert(const char* file, const char* func, u32 line, const char* assertStr, const char* fmt, ...)
{
    if (fmt)
    {
        va_list args;
        va_start(args, fmt);
        LogAssertV(file, func, line, assertStr, fmt, &args);
        va_end(args);
    }
    else
    {
        LogAssertV(file, func, line, assertStr, nullptr, nullptr);
    }
}
