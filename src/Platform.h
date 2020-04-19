#pragma once

#include "Common.h"

#if defined(PLATFORM_WINDOWS)
#define GAME_CODE_ENTRY __declspec(dllexport)
#elif defined(PLATFORM_LINUX)
#define GAME_CODE_ENTRY
#else
#error Unsupported OS
#endif

#include "Intrinsics.h"
#include "Math.h"
#include "OpenGL.h"

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

typedef void*(ReallocateFn)(void* ptr, uptr newSize);

typedef MemoryArena*(AllocateArenaFn)(uptr size);
typedef void(FreeArenaFn)(MemoryArena* arena);

// Work queue API
struct WorkQueue;
typedef void(WorkFn)(void* data0, void* data1, void* data2, u32 threadIndex);
typedef b32(PushWorkFn)(WorkQueue* queue, WorkFn* fn, void* data0, void* data1, void* data2);
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

typedef LoadedImage*(__cdecl ResourceLoaderLoadImageFn)(const char* filename, DynamicRange range, b32 flipY, u32 forceBPP, AllocateFn* allocator, LoggerFn* logger, void* loggerData);

struct ImageInfo {
    b32 valid;
    u32 width;
    u32 height;
    u32 channelCount;
};

typedef ImageInfo(__cdecl ResourceLoaderValidateImageFileFn)(const char* filename, LoggerFn* logger, void* loggerData);

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

    // Default allocator
    AllocateFn* Allocate;
    DeallocateFn* Deallocate;
    ReallocateFn* Reallocate;

    AllocateArenaFn* AllocateArena;
    FreeArenaFn* FreeArena;

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
    WorkQueue* lowPriorityQueue;
    WorkQueue* highPriorityQueue;
    ImGuiContext* imguiContext;
    InputMode inputMode;
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
