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
#define GAME_CODE_ENTRY __declspec(dllexport)
#elif defined(PLATFORM_LINUX)
#define GAME_CODE_ENTRY
#else
#error Unsupported OS
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

template<typename T>
T* Init(T* ptr) {
    return new(ptr) T();
}

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

namespace U32 {
    constexpr u32 Max = 0xffffffff;
}

#include "flux_intrinsics.h"
#include "flux_math.h"
#include "flux_opengl.h"

enum struct GameInvoke : u32
{
    Init, Reload, Update, Render
};

struct DateTime
{
    static constexpr u32 StringSize = 9;
    u16 year;
    u16 month;
    u16 dayOfWeek;
    u16 day;
    u16 hour;
    u16 minute;
    u16 seconds;
    u16 milliseconds;
};

struct DirectoryContents
{
    b32 scannedSuccesfully;
    u32 count;
    wchar_t** filenames;
};

enum struct LogLevel
{
    Fatal = 0, Error, Warn, Info
};

struct MemoryArena;

// NOTE: On unix API this should be defined as int
typedef uptr FileHandle;
const FileHandle InvalidFileHandle = Uptr::Max;

typedef u32(DebugGetFileSizeFn)(const wchar_t* filename);
typedef u32(DebugReadFileFn)(void* buffer, u32 bufferSize, const wchar_t* filename);
typedef u32(DebugReadTextFileFn)(void* buffer, u32 bufferSize, const wchar_t* filename);
typedef bool(DebugWriteFileFn)(const wchar_t* filename, void* data, u32 dataSize);
typedef b32(DebugCopyFileFn)(const wchar_t* source, const wchar_t* dest, bool overwrite);

typedef FileHandle(DebugOpenFileFn)(const wchar_t* filename);
typedef bool(DebugCloseFileFn)(FileHandle handle);
typedef u32(DebugWriteToOpenedFileFn)(FileHandle handle, void* data, u32 size);

typedef f64(GetTimeStampFn)();

typedef DirectoryContents(EnumerateFilesInDirectoryFn)(const wchar_t* dirName, MemoryArena* tempArena);

typedef void(GLDebugCallbackFn)(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, const GLvoid* userParam);

enum struct InputMode
{
    FreeCursor,
    CaptureCursor
};

typedef void(SetInputModeFn)(InputMode);

typedef void*(AllocateFn)(uptr size);
typedef void(DeallocateFn)(void* ptr);
typedef void*(ReallocateFn)(void* ptr, uptr newSize);

// Work queue API
struct WorkQueue;
typedef void(WorkFn)(void* data, u32 threadIndex);
typedef b32(PushWorkFn)(WorkQueue* queue, void* data, WorkFn* fn);
typedef void(CompleteAllWorkFn)(WorkQueue* queue);

typedef void(SleepFn)(u32 ms);

struct Mesh {
    char name[32];
    void* base;
    Mesh* head;
    Mesh* next;
    u32 vertexCount;
    u32 indexCount;
    v3* vertices;
    v3* normals;
    v2* uvs;
    v3* tangents;
    v3* bitangents;
    v3* colors;
    u32* indices;
    BBoxAligned aabb;
    u32 gpuVertexBufferHandle;
    u32 gpuIndexBufferHandle;
};

static_assert(sizeof(Mesh) % 8 == 0);

enum struct DynamicRange : u32 {
    LDR, HDR
};

const char* ToString(DynamicRange value) {
    switch (value) {
    case DynamicRange::LDR: { return "LDR"; } break;
    case DynamicRange::HDR: { return "HDR"; } break;
        invalid_default();
    }
    return "";
}

struct LoadedImage {
    void* base;
    void* bits;
    char name[32];
    u32 width;
    u32 height;
    u32 channels;
    DynamicRange range;
};

static_assert(sizeof(LoadedImage) % 4 == 0);

typedef LoadedImage*(__cdecl ResourceLoaderLoadImageFn)(const char* filename, DynamicRange range, b32 flipY, u32 forceBPP, AllocateFn* allocator);

struct ImageInfo {
    b32 valid;
    u32 width;
    u32 height;
    u32 channelCount;
};

typedef ImageInfo(__cdecl ResourceLoaderValidateImageFileFn)(const char* filename);

struct PlatformCalls
{
    DebugGetFileSizeFn* DebugGetFileSize;
    DebugReadFileFn* DebugReadFile;
    DebugReadTextFileFn* DebugReadTextFile;
    DebugWriteFileFn* DebugWriteFile;
    DebugOpenFileFn* DebugOpenFile;
    DebugCloseFileFn* DebugCloseFile;
    DebugCopyFileFn* DebugCopyFile;
    DebugWriteToOpenedFileFn* DebugWriteToOpenedFile;
    SetInputModeFn* SetInputMode;

    AllocateFn* Allocate;
    DeallocateFn* Deallocate;
    ReallocateFn* Reallocate;

    // Work queue
    PushWorkFn* PushWork;
    CompleteAllWorkFn* CompleteAllWork;
    SleepFn* Sleep;

    GetTimeStampFn* GetTimeStamp;

    ResourceLoaderLoadImageFn* ResourceLoaderLoadImage;
    ResourceLoaderValidateImageFileFn* ResourceLoaderValidateImageFile;

    EnumerateFilesInDirectoryFn* EnumerateFilesInDirectory;
};

struct KeyState
{
    // TODO: Compress
    b32 pressedNow;
    b32 wasPressed;
};

struct MouseButtonState
{
    // TODO: Compress
    b32 pressedNow;
    b32 wasPressed;
};

enum struct MouseButton : u8
{
    Left = 0, Right, Middle, XButton1, XButton2
};

enum struct Key : u8
{
    Invalid = 0x00,
    // NOTE: currently works only ctrl for both left and right keys
    // right ctrl and super key doesn`t work on linux.
    Ctrl, Space, Apostrophe, Comma,
    Minus, Period, Slash, _0 = 0x30,
    _1, _2, _3, _4, _5, _6, _7, _8,
    _9, Semicolon, Equal, A = 0x41,
    B, C, D, E, F, G, H,
    I, J, K, L, M, N, O,
    P, Q, R, S, T, U, V,
    W, X, Y, Z, LeftBracket,
    BackSlash, RightBracket, Tilde,
    Escape, Enter, Tab, Backspace,
    Insert, Delete, Right, Left,
    Down, Up, PageUp, PageDown,
    Home, End, CapsLock, ScrollLock,
    NumLock, PrintScreen, Pause,
    Return = Enter, F1 = 114,
    F2, F3, F4, F5, F6,
    F7, F8, F9, F10, F11,
    F12, F13, F14, F15, F16,
    F17, F18, F19, F20, F21,
    F22, F23, F24, Num0, Num1,
    Num2, Num3, Num4, Num5, Num6,
    Num7, Num8, Num9, NumDecimal,
    NumDivide, NumMultiply, NumSubtract,
    NumAdd, NumEnter = Enter,
    LeftShift = 153, LeftCtrl, Alt,
    LeftSuper, Menu, RightShift,
    RightCtrl, RightSuper, Clear,
    Shift
};


struct InputState
{
    static const u32 KeyCount = 256;
    static const u32 MouseButtonCount = 5;
    KeyState keys[KeyCount];
    MouseButtonState mouseButtons[MouseButtonCount];
    b32 mouseInWindow;
    b32 activeApp;
    // NOTE: All mouse position values are normalized
    f32 mouseX;
    f32 mouseY;
    f32 mouseFrameOffsetX;
    f32 mouseFrameOffsetY;
    // NOTE: Not normalized
    i32 scrollOffset;
    i32 scrollFrameOffset;
};

struct ImGuiContext;

struct PlatformState
{
    PlatformCalls functions;
    OpenGL* gl;
    volatile b32 supportsAsyncGPUTransfer;
    WorkQueue* workQueue;
    ImGuiContext* imguiContext;
    InputState input;
    u64 tickCount;
    i32 fps;
    i32 ups;
    f32 gameSpeed;
    f32 absDeltaTime;
    f32 gameDeltaTime;
    u32 windowWidth;
    u32 windowHeight;
    DateTime localTime;
    GLDebugCallbackFn* glDebugCallback;
};

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
