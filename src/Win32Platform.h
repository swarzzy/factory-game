#pragma once
#include "Platform.h"
#include <windows.h>

#include "Win32CodeLoader.h"

#define DISCRETE_GRAPHICS_DEFAULT
#define ENABLE_CONSOLE
#define DEBUG_OPENGL
constexpr u32 OPENGL_MAJOR_VERSION = 4;
constexpr u32 OPENGL_MINOR_VERSION = 5;

constexpr f64 SECONDS_PER_TICK = 1.0 / 60.0;

const u32 NumOfWorkerThreads = 4;

//#define OPENGL_WORKER_CONTEXTS

// Macros from windowsx.h
#define GET_X_LPARAM(lp) ((int)(short)LOWORD(lp))
#define GET_Y_LPARAM(lp) ((int)(short)HIWORD(lp))

#define KEY_REPEAT_COUNT_FROM_LPARAM(lParam) ((u16)(lParam & 0xffff >> 2))

#define WGL_DRAW_TO_WINDOW_ARB            0x2001
#define WGL_SUPPORT_OPENGL_ARB            0x2010
#define WGL_DOUBLE_BUFFER_ARB             0x2011
#define WGL_PIXEL_TYPE_ARB                0x2013
#define WGL_TYPE_RGBA_ARB                 0x202B
#define WGL_COLOR_BITS_ARB                0x2014
#define WGL_DEPTH_BITS_ARB                0x2022
#define WGL_STENCIL_BITS_ARB              0x2023
#define WGL_ACCELERATION_ARB              0x2003
#define WGL_FULL_ACCELERATION_ARB         0x2027
#define WGL_NO_ACCELERATION_ARB           0x2025

#define WGL_FRAMEBUFFER_SRGB_CAPABLE_ARB 0x20A9

#define WGL_CONTEXT_MAJOR_VERSION_ARB     0x2091
#define WGL_CONTEXT_MINOR_VERSION_ARB     0x2092
#define WGL_CONTEXT_LAYER_PLANE_ARB       0x2093
#define WGL_CONTEXT_FLAGS_ARB             0x2094
#define WGL_CONTEXT_PROFILE_MASK_ARB      0x9126
#define WGL_CONTEXT_CORE_PROFILE_BIT_ARB  0x00000001
#define WGL_CONTEXT_COMPATIBILITY_PROFILE_BIT_ARB 0x00000002
#define WGL_CONTEXT_DEBUG_BIT_ARB         0x0001

#define GL_SHADING_LANGUAGE_VERSION       0x8B8C

const int OpenGLContextAttribs[] = {
    WGL_CONTEXT_MAJOR_VERSION_ARB, OPENGL_MAJOR_VERSION,
    WGL_CONTEXT_MINOR_VERSION_ARB, OPENGL_MINOR_VERSION,
    WGL_CONTEXT_PROFILE_MASK_ARB, WGL_CONTEXT_CORE_PROFILE_BIT_ARB,
#if defined(DEBUG_OPENGL)
    WGL_CONTEXT_FLAGS_ARB, WGL_CONTEXT_DEBUG_BIT_ARB,
#endif
    0
};

extern "C"
{
    typedef const char*(APIENTRY wglGetExtensionsStringARBFn)(HDC);
    typedef BOOL (APIENTRY wglChoosePixelFormatARBFn)(HDC, const int*, const FLOAT*, UINT, int*, UINT*);
    typedef HGLRC (APIENTRY wglCreateContextAttribsARBFn)(HDC, HGLRC, const int*);
    typedef BOOL (APIENTRY wglSwapIntervalEXTFn)(int interval);
    typedef int (APIENTRY wglGetSwapIntervalEXTFn)(void);
}

struct WorkQueueEntry {
    void* data0;
    void* data1;
    void* data2;
    WorkFn* function;
};

struct WorkQueue {
    u32 volatile pendingWorkCount;
    u32 volatile completedWorkCount;
    u32 volatile begin;
    u32 volatile end;
    HANDLE semaphore;
    WorkQueueEntry queue[128];
};

struct Win32ThreadInfo {
    u32 index;
    WorkQueue* lowPriorityQueue;
    WorkQueue* highPriorityQueue;
    HGLRC glrc;
};

struct Win32Context
{
    HANDLE consoleHandle = GetStdHandle(STD_OUTPUT_HANDLE);
    WINDOWPLACEMENT wpPrev;
    b32 fullscreen;
    HWND windowHandle;
    HDC windowDC;
    HGLRC openGLRC;
    PlatformState state;
    TCHAR windowTitle[32];
    TRACKMOUSEEVENT Win32MouseTrackEvent;
    InputMode inputMode;
    LibraryData gameLib;
    WorkQueue lowPriorityQueue;
    WorkQueue highPriorityQueue;
    HGLRC workersGLRC[NumOfWorkerThreads];

    // NOTE: WGL
    wglGetExtensionsStringARBFn* wglGetExtensionsStringARB;
    wglChoosePixelFormatARBFn* wglChoosePixelFormatARB;
    wglCreateContextAttribsARBFn* wglCreateContextAttribsARB;
    wglSwapIntervalEXTFn* wglSwapIntervalEXT;
    wglGetSwapIntervalEXTFn* wglGetSwapIntervalEXT;
    u8 keyTable[InputState::KeyCount];
};

enum ConsoleColor
{
    CONSOLE_COLOR_BLACK = 0, CONSOLE_COLOR_DARKBLUE, CONSOLE_COLOR_DARKGREEN,
    CONSOLE_COLOR_DARKCYAN, CONSOLE_COLOR_DARKRED, CONSOLE_COLOR_DARKPURPLE,
    CONSOLE_COLOR_DARKYELLOW, CONSOLE_COLOR_DARKWHITE, CONSOLE_COLOR_GRAY,
    CONSOLE_COLOR_BLUE, CONSOLE_COLOR_GREEN, CONSOLE_COLOR_CYAN,
    CONSOLE_COLOR_RED, CONSOLE_COLOR_PURPLE, CONSOLE_COLOR_YELLOW,
    CONSOLE_COLOR_WHITE, CONSOLE_COLOR_DEFAULT
};

const ConsoleColor CONSOLE_DEFAULT_TEXT_COLOR = CONSOLE_COLOR_DARKWHITE;
const ConsoleColor CONSOLE_DEFAULT_BACK_COLOR = CONSOLE_COLOR_BLACK;
