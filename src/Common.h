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

#define array_count(arr) ((uint)(sizeof(arr) / sizeof(arr[0])))
#define typedecl(type, member) (((type*)0)->member)
#define offset_of(type, member) ((uptr)(&(((type*)0)->member)))
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

#define foreach(collection) auto concat(iterator__, __LINE__) =  (collection).GetIterator(); for (auto it = concat(iterator__, __LINE__) .Begin(); ! concat(iterator__, __LINE__) .End(); concat(iterator__, __LINE__) .Advance(), it = concat(iterator__, __LINE__). Get())

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

typedef u32 usize;

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

namespace U64 {
    constexpr u64 Max = UINT64_MAX;
}

// NOTE: Allocator API
typedef void*(AllocateFn)(uptr size, uptr alignment, void* allocatorData);
typedef void(DeallocateFn)(void* ptr, void* allocatorData);

struct Allocator {
    AllocateFn* allocate;
    DeallocateFn* deallocate;
    void* data;

    inline void* Alloc(uptr size, uptr alignment) { return allocate(size, alignment, data); }
    inline void Dealloc(void* ptr) { deallocate(ptr, data); }
};

inline Allocator MakeAllocator(AllocateFn* allocate, DeallocateFn* deallocate, void* data) {
    Allocator allocator;
    allocator.allocate = allocate;
    allocator.deallocate = deallocate;
    allocator.data = data;
    return allocator;
}

// NOTE: Logger API
typedef void(LoggerFn)(void* loggerData, const char* fmt, va_list* args);
typedef void(AssertHandlerFn)(void* userData, const char* file, const char* func, u32 line, const char* exprString, const char* fmt, va_list* args);

extern LoggerFn* GlobalLogger;
extern void* GlobalLoggerData;

extern AssertHandlerFn* GlobalAssertHandler;
extern void* GlobalAssertHandlerData;

#define log_print(fmt, ...) _GlobalLoggerWithArgs(GlobalLoggerData, fmt, __VA_ARGS__)
#define assert(expr, ...) do { if (!(expr)) {_GlobalAssertHandler(GlobalAssertHandlerData, __FILE__, __func__, __LINE__, #expr, __VA_ARGS__);}} while(false)
// NOTE: Defined always
#define panic(expr, ...) do { if (!(expr)) {_GlobalAssertHandler(GlobalAssertHandlerData, __FILE__, __func__, __LINE__, #expr, __VA_ARGS__);}} while(false)

inline void _GlobalLoggerWithArgs(void* data, const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);
    GlobalLogger(data, fmt,  &args);
    va_end(args);
}

inline void _GlobalAssertHandler(void* userData, const char* file, const char* func, u32 line, const char* exprString) {
    GlobalAssertHandler(userData, file, func, line, exprString, nullptr, nullptr);
}

inline void _GlobalAssertHandler(void* userData, const char* file, const char* func, u32 line, const char* exprString, const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);
    GlobalAssertHandler(userData, file, func, line, exprString, fmt, &args);
    va_end(args);
}
