// NOTE: Enforcing unicode
// Engine always supports unicode
#if !defined(UNICODE)
#define UNICODE
#endif
#if !defined(_UNICODE)
#define _UNICODE
#endif

#include "Win32Platform.h"
#include "Memory.h"

#include <intrin.h>
#include <timeapi.h>
#include <tchar.h>
#include <stdlib.h>

#include "../ext/imgui/imgui.h"
#include "../ext/imgui/imgui_impl_win32.h"

#if defined (DISCRETE_GRAPHICS_DEFAULT)
extern "C" { __declspec(dllexport) DWORD NvOptimusEnablement = 0x00000001; }
extern "C" { __declspec(dllexport) DWORD AmdPowerXpressRequestHighPerformance = 0x01; }
#endif

static Win32Context GlobalContext = {};
static bool GlobalRunning = true;
static LARGE_INTEGER GlobalPerformanceFrequency = {};
static void* GlobalGameData = 0;

// TODO: Logger
void Logger(void* data, const char* fmt, va_list* args) {
    vprintf(fmt, *args);
}

LoggerFn* GlobalLogger = Logger;
void* GlobalLoggerData = nullptr;

inline void AssertHandler(void* data, const char* file, const char* func, u32 line, const char* assertStr, const char* fmt, va_list* args) {
    log_print("[Assertion failed] Expression (%s) result is false\nFile: %s, function: %s, line: %d.\n", assertStr, file, func, (int)line);
    if (args) {
        GlobalLogger(GlobalLoggerData, fmt, args);
    }
    debug_break();
}

AssertHandlerFn* GlobalAssertHandler = AssertHandler;
void* GlobalAssertHandlerData = nullptr;

struct OpenGLLoadResult
{
    OpenGL* context;
    b32 success;
};

void* OpenGLGetProcAddress(const char* name)
{
    auto result = wglGetProcAddress(name);
    if (result == 0 ||
        result == (void*)0x1 ||
        result == (void*)0x2 ||
        result == (void*)0x3 ||
        result == (void*)-1)
    {
        // NOTE: Failed
        result = 0;
    }
    return (void*)result;
}

OpenGLLoadResult LoadOpenGL()
{
    OpenGL* context = (OpenGL*)VirtualAlloc(0, sizeof(OpenGL), MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
    panic(context);

    log_print("[Info] Loading OpenGL functions...\n");
    log_print("[Info] Functions defined: %d\n", (int)OpenGL::FunctionCount);
    log_print("[Info] Loading functions...");

    b32 success = true;
    HMODULE glLibHandle = {};
    for (u32 i = 0; i < OpenGL::FunctionCount; i++)
    {
        context->functions.raw[i] = OpenGLGetProcAddress(OpenGL::FunctionNames[i]);
        if (!context->functions.raw[i])
        {
            if (!glLibHandle)
            {
                glLibHandle = LoadLibrary(TEXT("opengl32.dll"));
            }
            if (glLibHandle)
            {
                context->functions.raw[i] = (void*)GetProcAddress(glLibHandle, OpenGL::FunctionNames[i]);
            }
            else
            {
                context->functions.raw[i] = 0;
                log_print("\n[Error]: Failed to load OpenGL procedure: %s", OpenGL::FunctionNames[i]);
                success = false;
            }
        }
    }

    if (success) {
        log_print("   Done\n");
    } else {
        panic("Failed to load OpenGL functions");
    }

    // NOTE: Querying extensions
    log_print("[Info] Loading OpenGL extensions...");
    GLint numExtensions;
    context->functions.fn.glGetIntegerv(GL_NUM_EXTENSIONS, &numExtensions);
    for (i32x i = 0; i < numExtensions; i++)
    {
        const GLubyte* extensionString;
        extensionString = context->functions.fn.glGetStringi(GL_EXTENSIONS, i);
        if (strcmp((const char*)extensionString, "GL_EXT_texture_filter_anisotropic") == 0)
        {
            context->extensions.EXT_texture_filter_anisotropic = true;
        }
        if (strcmp((const char*)extensionString, "GL_ARB_texture_filter_anisotropic") == 0)
        {
            context->extensions.ARB_texture_filter_anisotropic = true;
        }
        if (strcmp((const char*)extensionString, "GL_ARB_gl_spirv") == 0)
        {
            context->extensions.ARB_gl_spirv.glSpecializeShaderARB = (PFNGLSPECIALIZESHADERARBPROC)OpenGLGetProcAddress("glSpecializeShaderARB");
            if (context->extensions.ARB_gl_spirv.glSpecializeShaderARB)
            {
                context->extensions.ARB_gl_spirv.supported = true;
            }
        }
        if (strcmp((const char*)extensionString, "GL_ARB_spirv_extensions") == 0)
        {
            context->extensions.ARB_spirv_extensions = true;
        }
        if (strcmp((const char*)extensionString, "GL_ARB_framebuffer_sRGB") == 0)
        {
            context->extensions.ARB_framebuffer_sRGB = true;
        }
    }

    if (!context->extensions.ARB_texture_filter_anisotropic && !context->extensions.EXT_texture_filter_anisotropic)
    {
        log_print("[Info] GL_texture_filter_anisotropic is not supported\n");
    }
    if (!context->extensions.ARB_gl_spirv.supported)
    {
        log_print("[Info] ARB_gl_spirv is not supported\n");
    }
    if (!context->extensions.ARB_spirv_extensions)
    {
        log_print("[Info] ARB_spirv_extensions is not supported\n");
    }
    if (!context->extensions.ARB_framebuffer_sRGB)
    {
        log_print("[Info] ARB_framebuffer_sRGB is not supported\n");
    }

    log_print("   Done.\n");

    if (success)
    {
        // TODO: Do this in renderer
        u32 globalVAO;
        context->functions.fn.glGenVertexArrays(1, &globalVAO);
        context->functions.fn.glBindVertexArray(globalVAO);
        context->functions.fn.glEnable(GL_DEPTH_TEST);
        context->functions.fn.glDepthFunc(GL_LESS);
        context->functions.fn.glEnable(GL_CULL_FACE);
        context->functions.fn.glEnable(GL_MULTISAMPLE);
        context->functions.fn.glCullFace(GL_BACK);
        context->functions.fn.glFrontFace(GL_CCW);
        context->functions.fn.glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);

        if (context->extensions.ARB_framebuffer_sRGB) {
            // NOTE: Always enables if supported
            //context->functions.fn.glEnable(GL_FRAMEBUFFER_SRGB);
        }

        //context->functions._glEnable(GL_BLEND);
        //context->functions._glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        //context->functions._glBlendEquation(GL_FUNC_ADD);
    }

    return {context, success};
}

u32 DebugGetFileSize(const wchar_t* filename)
{
    u32 fileSize = 0;
    HANDLE handle = CreateFileW(filename, GENERIC_READ, FILE_SHARE_READ, 0,
                                OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
    if (handle != INVALID_HANDLE_VALUE)
    {
        DWORD sz = (u32)GetFileSize(handle, 0);
        if (sz != INVALID_FILE_SIZE)
        {
            fileSize = (u32)sz;
        }
        CloseHandle(handle);
    }
    return fileSize;
}

u32 DebugReadFileToBuffer(void* buffer, u32 bufferSize, const wchar_t* filename)
{
    u32 written = 0;
    LARGE_INTEGER fileSize = {0};
    HANDLE fileHandle = CreateFileW(filename, GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING, 0, 0);
    if (fileHandle != INVALID_HANDLE_VALUE)
    {
        if (GetFileSizeEx(fileHandle, &fileSize))
        {
            DWORD readSize = (DWORD)bufferSize;
            if (fileSize.QuadPart < bufferSize)
            {
                readSize = (DWORD)fileSize.QuadPart;
            }
            if (buffer)
            {
                DWORD read;
                BOOL result = ReadFile(fileHandle, buffer, readSize, &read, 0);
                if (!result && !(read == readSize))
                {
                    log_print("[Warn] Failed to open file");
                }
                else
                {
                    written = read;
                }
            }
        }
        CloseHandle(fileHandle);
    }
    return written;
}

u32 DebugReadTextFileToBuffer(void* buffer, u32 bufferSize, const wchar_t* filename)
{
    u32 bytesRead = 0;
    char* string = nullptr;
    LARGE_INTEGER fileSize = {};
    HANDLE fileHandle = CreateFileW(filename, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, 0);
    if (fileHandle != INVALID_HANDLE_VALUE)
    {
        if (GetFileSizeEx(fileHandle, &fileSize))
        {
            if (fileSize.QuadPart + 1 > bufferSize)
            {
                log_print("[Warn] Failed to open file");
                CloseHandle(fileHandle);
                return 0;
            }
            if (buffer)
            {
                DWORD read;
                BOOL result = ReadFile(fileHandle, buffer,
                                       (DWORD)fileSize.QuadPart, &read, 0);
                if (!result && !(read == (DWORD)fileSize.QuadPart))
                {
                    log_print("[Warn] Failed to open file");
                    return 0;
                }
                else
                {
                    ((char*)buffer)[fileSize.QuadPart] = '\0';
                    bytesRead = (u32)fileSize.QuadPart + 1;
                }
            }
        }
        CloseHandle(fileHandle);
    }
    return bytesRead;
}

bool DebugWriteFile(const wchar_t* filename, void* data, u32 dataSize)
{
    HANDLE fileHandle = CreateFileW(filename, GENERIC_WRITE, 0, 0, CREATE_ALWAYS, 0, 0);
    if (fileHandle != INVALID_HANDLE_VALUE)
    {
        DWORD bytesWritten;
        BOOL writeResult = WriteFile(fileHandle, data,
                                     dataSize, &bytesWritten, 0);
        if (writeResult && (dataSize == bytesWritten))
        {
            CloseHandle(fileHandle);
            return true;
        }
    }
    CloseHandle(fileHandle);
    return false;
}

FileHandle DebugOpenFile(const wchar_t* filename)
{
    FileHandle result = InvalidFileHandle;
    HANDLE w32Handle = CreateFileW(filename, GENERIC_WRITE | GENERIC_READ, FILE_SHARE_READ, 0, CREATE_NEW, 0, 0);
    if (w32Handle != INVALID_HANDLE_VALUE)
    {
        result = (FileHandle)w32Handle;
    }
    return result;
}

bool DebugCloseFile(FileHandle handle)
{
    bool result = false;
    if (CloseHandle((HANDLE)handle))
    {
        result = true;
    }
    return result;
}

u32 DebugWriteToOpenedFile(FileHandle handle, void* data, u32 size)
{
    u32 result = 0;
    DWORD bytesWritten;
    BOOL writeResult = WriteFile((HANDLE)handle, data, size, &bytesWritten, 0);
    if (writeResult && (size == bytesWritten))
    {
        result = size;
    }
    return result;
}

b32 DebugCopyFile(const wchar_t* source, const wchar_t* dest, bool overwrite)
{
    BOOL failIfExists = overwrite ? FALSE : TRUE;
    auto result = CopyFile(source, dest, failIfExists);
    return (b32)result;
}

DirectoryContents EnumerateFilesInDirectory(const wchar_t* dirName, MemoryArena* tempArena)
{
    DirectoryContents result = {};
    return result;
}

// NOTE: Based on Raymond Chen example
// https://devblogs.microsoft.com/oldnewthing/20100412-00/?p=14353
void WindowToggleFullscreen(Win32Context* app, bool enable)
{
    DWORD style = GetWindowLong(app->windowHandle, GWL_STYLE);
    if ((style & WS_OVERLAPPEDWINDOW) && enable)
    {
        app->fullscreen = true;
        MONITORINFO mInfo = { sizeof(MONITORINFO) };
        if (GetWindowPlacement(app->windowHandle, &app->wpPrev) &&
            GetMonitorInfo(MonitorFromWindow(app->windowHandle,
                                             MONITOR_DEFAULTTOPRIMARY), &mInfo))
        {
            SetWindowLong(app->windowHandle, GWL_STYLE,
                          style & ~WS_OVERLAPPEDWINDOW);
            SetWindowPos(app->windowHandle, HWND_TOP,
                         mInfo.rcMonitor.left, mInfo.rcMonitor.top,
                         mInfo.rcMonitor.right - mInfo.rcMonitor.left,
                         mInfo.rcMonitor.bottom - mInfo.rcMonitor.top,
                         SWP_NOOWNERZORDER | SWP_FRAMECHANGED);
        }
    }
    else if (!enable)
    {
        app->fullscreen = false;
        SetWindowLong(app->windowHandle, GWL_STYLE,
                      style | WS_OVERLAPPEDWINDOW);
        SetWindowPlacement(app->windowHandle, &app->wpPrev);
        SetWindowPos(app->windowHandle, nullptr, 0, 0, 0, 0,
                     SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER |
                     SWP_NOOWNERZORDER | SWP_FRAMECHANGED);
    }
}

bool ConsoleSetColor(ConsoleColor textColor, ConsoleColor backColor)
{
    if (textColor == CONSOLE_COLOR_DEFAULT)
        textColor = CONSOLE_DEFAULT_TEXT_COLOR;
    if (backColor == CONSOLE_COLOR_DEFAULT)
        backColor = CONSOLE_DEFAULT_BACK_COLOR;

    WORD consoleColor = (WORD)textColor | ((WORD)backColor << 4);
    BOOL result = SetConsoleTextAttribute(GlobalContext.consoleHandle, consoleColor);

    return (bool)result;
}

DateTime GetLocalTime()
{
    DateTime datetime = {};
    SYSTEMTIME time;
    GetLocalTime(&time);

    datetime.year = time.wYear;
    datetime.month = time.wMonth;
    datetime.dayOfWeek = time.wDayOfWeek;
    datetime.day = time.wDay;
    datetime.hour = time.wHour;
    datetime.minute = time.wMinute;
    datetime.seconds = time.wSecond;
    datetime.milliseconds = time.wMilliseconds;

    return datetime;
}

b32 ConsolePrint(const char* string)
{
    DWORD written;
    BOOL retVal = WriteConsoleA(GlobalContext.consoleHandle, string, (DWORD)strlen(string), &written, NULL);
    b32 result = (b32)retVal;
    return result;
}

b32 ConsolePrint(const void* data, u32 count)
{
    DWORD written;
    BOOL result = WriteConsoleA(GlobalContext.consoleHandle, data, count, &written, NULL);
    if (written != count || result == 0)
    {
        return 0;
    }
    return 1;
}

// TODO: Fill the table at compile time
void Win32InitKeyTable(u8* keytable)
{
    memset((void*)keytable, 0, InputState::KeyCount);

    keytable[0x08] = (u8)Key::Backspace;
    keytable[0x09] = (u8)Key::Tab;
    keytable[0x0c] = (u8)Key::Clear;
    keytable[0x0d] = (u8)Key::Enter;
    keytable[0x10] = (u8)Key::Shift;
    keytable[0x11] = (u8)Key::Ctrl;
    keytable[0x12] = (u8)Key::Alt;
    keytable[0x13] = (u8)Key::Pause;
    keytable[0x14] = (u8)Key::CapsLock;
    keytable[0x1b] = (u8)Key::Escape;
    keytable[0x20] = (u8)Key::Space;
    keytable[0x21] = (u8)Key::PageUp;
    keytable[0x22] = (u8)Key::PageDown;
    keytable[0x23] = (u8)Key::End;
    keytable[0x24] = (u8)Key::Home;
    keytable[0x25] = (u8)Key::Left;
    keytable[0x26] = (u8)Key::Up;
    keytable[0x27] = (u8)Key::Right;
    keytable[0x28] = (u8)Key::Down;
    keytable[0x2c] = (u8)Key::PrintScreen;
    keytable[0x2d] = (u8)Key::Insert;
    keytable[0x2e] = (u8)Key::Delete;
    keytable[0x30] = (u8)Key::_0;
    keytable[0x31] = (u8)Key::_1;
    keytable[0x32] = (u8)Key::_2;
    keytable[0x33] = (u8)Key::_3;
    keytable[0x34] = (u8)Key::_4;
    keytable[0x35] = (u8)Key::_5;
    keytable[0x36] = (u8)Key::_6;
    keytable[0x37] = (u8)Key::_7;
    keytable[0x38] = (u8)Key::_8;
    keytable[0x39] = (u8)Key::_9;
    keytable[0x41] = (u8)Key::A;
    keytable[0x42] = (u8)Key::B;
    keytable[0x43] = (u8)Key::C;
    keytable[0x44] = (u8)Key::D;
    keytable[0x45] = (u8)Key::E;
    keytable[0x46] = (u8)Key::F;
    keytable[0x47] = (u8)Key::G;
    keytable[0x48] = (u8)Key::H;
    keytable[0x49] = (u8)Key::I;
    keytable[0x4a] = (u8)Key::J;
    keytable[0x4b] = (u8)Key::K;
    keytable[0x4c] = (u8)Key::L;
    keytable[0x4d] = (u8)Key::M;
    keytable[0x4e] = (u8)Key::N;
    keytable[0x4f] = (u8)Key::O;
    keytable[0x50] = (u8)Key::P;
    keytable[0x51] = (u8)Key::Q;
    keytable[0x52] = (u8)Key::R;
    keytable[0x53] = (u8)Key::S;
    keytable[0x54] = (u8)Key::T;
    keytable[0x55] = (u8)Key::U;
    keytable[0x56] = (u8)Key::V;
    keytable[0x57] = (u8)Key::W;
    keytable[0x58] = (u8)Key::X;
    keytable[0x59] = (u8)Key::Y;
    keytable[0x5a] = (u8)Key::Z;
    keytable[0x5b] = (u8)Key::LeftSuper;
    keytable[0x5c] = (u8)Key::RightSuper;
    keytable[0x60] = (u8)Key::Num0;
    keytable[0x61] = (u8)Key::Num1;
    keytable[0x62] = (u8)Key::Num2;
    keytable[0x63] = (u8)Key::Num3;
    keytable[0x64] = (u8)Key::Num4;
    keytable[0x65] = (u8)Key::Num5;
    keytable[0x66] = (u8)Key::Num6;
    keytable[0x67] = (u8)Key::Num7;
    keytable[0x68] = (u8)Key::Num8;
    keytable[0x69] = (u8)Key::Num9;
    keytable[0x6a] = (u8)Key::NumMultiply;
    keytable[0x6b] = (u8)Key::NumAdd;
    keytable[0x6d] = (u8)Key::NumSubtract;
    keytable[0x6e] = (u8)Key::NumDecimal;
    keytable[0x6f] = (u8)Key::NumDivide;
    keytable[0x70] = (u8)Key::F1;
    keytable[0x71] = (u8)Key::F2;
    keytable[0x72] = (u8)Key::F3;
    keytable[0x73] = (u8)Key::F4;
    keytable[0x74] = (u8)Key::F5;
    keytable[0x75] = (u8)Key::F6;
    keytable[0x76] = (u8)Key::F7;
    keytable[0x77] = (u8)Key::F8;
    keytable[0x78] = (u8)Key::F9;
    keytable[0x79] = (u8)Key::F10;
    keytable[0x7a] = (u8)Key::F11;
    keytable[0x7b] = (u8)Key::F12;
    keytable[0x7c] = (u8)Key::F13;
    keytable[0x7d] = (u8)Key::F14;
    keytable[0x7e] = (u8)Key::F15;
    keytable[0x7f] = (u8)Key::F16;
    keytable[0x80] = (u8)Key::F17;
    keytable[0x81] = (u8)Key::F18;
    keytable[0x82] = (u8)Key::F19;
    keytable[0x83] = (u8)Key::F20;
    keytable[0x84] = (u8)Key::F21;
    keytable[0x85] = (u8)Key::F22;
    keytable[0x86] = (u8)Key::F23;
    keytable[0x87] = (u8)Key::F24;
    keytable[0x90] = (u8)Key::NumLock;
    keytable[0x91] = (u8)Key::ScrollLock;
    keytable[0xa0] = (u8)Key::LeftShift;
    keytable[0xa1] = (u8)Key::RightShift;
    // NOTE: only ctrl Now works and processed by syskey events
    //keytable[0xa2] = Keyboardkey::leftctrl;
    //keytable[0xa3] = Keyboardkey::rightctrl;
    //keytable[0xa4] = Keyboardkey::leftalt; 0x11
    keytable[0xa5] = (u8)Key::Menu;
    keytable[0xba] = (u8)Key::Semicolon;
    keytable[0xbb] = (u8)Key::Equal;
    keytable[0xbc] = (u8)Key::Comma;
    keytable[0xbd] = (u8)Key::Minus;
    keytable[0xbe] = (u8)Key::Period;
    keytable[0xbf] = (u8)Key::Slash;
    keytable[0xc0] = (u8)Key::Tilde;
    keytable[0xdb] = (u8)Key::LeftBracket;
    keytable[0xdc] = (u8)Key::BackSlash;
    keytable[0xdd] = (u8)Key::RightBracket;
    keytable[0xde] = (u8)Key::Apostrophe;
}

u8 ConvertWin32Keycode(Win32Context* app, u64 win32Key)
{
    if (win32Key < InputState::KeyCount)
    {
        return app->keyTable[win32Key];
    }
    return (u8)Key::Invalid;
}


bool WglLoadFunctions(Win32Context* ctx, HDC windowDC)
{
    ctx->wglGetExtensionsStringARB = (wglGetExtensionsStringARBFn*)wglGetProcAddress("wglGetExtensionsStringARB");
    if (!ctx->wglGetExtensionsStringARB) return false;

    const char* extensions = ctx->wglGetExtensionsStringARB(windowDC);

    if (!strstr(extensions, "WGL_ARB_pixel_format")) return false;
    if (!strstr(extensions, "WGL_ARB_create_context_profile")) return false;
    if (!strstr(extensions, "EXT_swap_control")) return false;

    ctx->wglChoosePixelFormatARB = (wglChoosePixelFormatARBFn*)wglGetProcAddress("wglChoosePixelFormatARB");
    if (!ctx->wglChoosePixelFormatARB) return false;
    ctx->wglCreateContextAttribsARB = (wglCreateContextAttribsARBFn*)wglGetProcAddress("wglCreateContextAttribsARB");
    if (!ctx->wglCreateContextAttribsARB) return false;
    ctx->wglSwapIntervalEXT = (wglSwapIntervalEXTFn*)wglGetProcAddress("wglSwapIntervalEXT");
    if (!ctx->wglCreateContextAttribsARB) return false;
    ctx->wglGetSwapIntervalEXT = (wglGetSwapIntervalEXTFn*)wglGetProcAddress("wglGetSwapIntervalEXT");
    if (!ctx->wglCreateContextAttribsARB) return false;

    return true;
}

void ProcessMButtonEvent(InputState* input, MouseButton button, b32 state)
{
    input->mouseButtons[(u32)button].wasPressed =  input->mouseButtons[(u32)button].pressedNow;
    input->mouseButtons[(u32)button].pressedNow = state;
}

void ProcessMButtonDownAndDblclkEventImGui(Win32Context* application, ImGuiIO* io, UINT msg, WPARAM wParam)
{
    int button = 0;
    if (msg == WM_LBUTTONDOWN || msg == WM_LBUTTONDBLCLK) { button = 0; }
    if (msg == WM_RBUTTONDOWN || msg == WM_RBUTTONDBLCLK) { button = 1; }
    if (msg == WM_MBUTTONDOWN || msg == WM_MBUTTONDBLCLK) { button = 2; }
    if (msg == WM_XBUTTONDOWN || msg == WM_XBUTTONDBLCLK) { button = (GET_XBUTTON_WPARAM(wParam) == XBUTTON1) ? 3 : 4; }
    if (!ImGui::IsAnyMouseDown() && GetCapture() == NULL)
    {
        SetCapture(application->windowHandle);
    }
    io->MouseDown[button] = true;
}


inline static void
ProcessMButtonUpEventImGui(Win32Context* application, ImGuiIO* io, UINT msg, WPARAM wParam)
{
    int button = 0;
    if (msg == WM_LBUTTONUP) { button = 0; }
    if (msg == WM_RBUTTONUP) { button = 1; }
    if (msg == WM_MBUTTONUP) { button = 2; }
    if (msg == WM_XBUTTONUP) { button = (GET_XBUTTON_WPARAM(wParam) == XBUTTON1) ? 3 : 4; }
    io->MouseDown[button] = false;
    if (!ImGui::IsAnyMouseDown() && ::GetCapture() == application->windowHandle)
        ::ReleaseCapture();
}

void WindowSetMousePosition(Win32Context* app, u32 x, u32 y)
{
    u32 yFlipped = 0;
    if (y < app->state.windowHeight)
    {
        yFlipped = app->state.windowHeight - y;
        POINT pt = { (LONG)x, (LONG)yFlipped };
        if (ClientToScreen(app->windowHandle, &pt))
        {
            SetCursorPos(pt.x, pt.y);
        }
    }
}

void WindowPollEvents(Win32Context* app)
{
    MSG message;
    BOOL result;
    while (GlobalRunning && (result = PeekMessage(&message, 0, 0, 0, PM_REMOVE)) != 0)
    {
        if (result == -1)
        {
            assert(false, "Window recieve error message.");
        }
        else
        {
#if 0
            if (message.message == WM_INPUT) {
                RAWINPUT input;
                UINT inputSize = sizeof(RAWINPUT);
                GetRawInputData((HRAWINPUT)message.lParam, RID_INPUT, &input, &inputSize, sizeof(RAWINPUTHEADER));
                if (input.header.dwType == RIM_TYPEMOUSE) {
                    log_print("Raw input: mouse position: %d, %d\n", input.data.mouse.lLastX, input.data.mouse.lLastY);
                }
            }
#endif
            TranslateMessage(&message);
            DispatchMessage(&message);
        }
    }

    if (app->state.input.activeApp) {
        POINT point;
        GetCursorPos(&point);
        i32 mousePositionX = point.x;
        i32 mousePositionY = point.y;

        if ((mousePositionX != app->mousePosX) || (mousePositionY != app->mousePosY)) {
            i32 offsetX = mousePositionX - app->mousePosX;
            i32 offsetY = -(mousePositionY - app->mousePosY);

            app->state.input.mouseFrameOffsetX = offsetX / (f32)app->state.windowWidth;
            app->state.input.mouseFrameOffsetY = offsetY / (f32)app->state.windowHeight;

            if (app->state.inputMode == InputMode::CaptureCursor) {
                u32 x = app->state.windowWidth / 2;
                u32 y = app->state.windowHeight / 2;
                WindowSetMousePosition(app, x, y);

                POINT p;
                GetCursorPos(&p);
                app->mousePosX = p.x;
                app->mousePosY = p.y;

                app->state.input.mouseX = x / (f32)app->state.windowWidth;
                app->state.input.mouseY = y / (f32)app->state.windowHeight;
            } else {
                app->mousePosX = mousePositionX;
                app->mousePosY = mousePositionY;
                app->state.input.mouseX = app->mousePosX / (f32)app->state.windowWidth;
                app->state.input.mouseY = (app->state.windowHeight - mousePositionY) / (f32)app->state.windowHeight;
            }
        }
    }
}

LRESULT CALLBACK Win32WindowProc(HWND windowHandle, UINT message, WPARAM wParam, LPARAM lParam)
{
    LRESULT result = 0;
    Win32Context* app = &GlobalContext;

    // NOTE: IMPORTANT: Always checking for nullptr
    ImGuiIO* io = nullptr;
    if (ImGui::GetCurrentContext())
    {
        ImGuiIO& _io = ImGui::GetIO();
        io = &_io;
    }

    switch (message)
    {
    case WM_SIZE:
    {
        app->state.windowWidth = LOWORD(lParam);
        app->state.windowHeight = HIWORD(lParam);
    } break;

    case WM_DESTROY:
    {
        PostQuitMessage(0);
    } break;

    case WM_CLOSE:
    {
        GlobalRunning = false;
        ShowWindow(app->windowHandle, SW_HIDE);
    } break;

    case WM_MOUSEMOVE:
    {
        if (!app->state.input.mouseInWindow)
        {
            app->state.input.mouseInWindow = true;
            TrackMouseEvent(&app->Win32MouseTrackEvent);
        }
    } break;

    case WM_LBUTTONDOWN:
    {
        ProcessMButtonEvent(&app->state.input, MouseButton::Left, true);
        ProcessMButtonDownAndDblclkEventImGui(app, io, message, wParam);
    } break;

    case WM_LBUTTONUP:
    {
        ProcessMButtonEvent(&app->state.input, MouseButton::Left, false);
        ProcessMButtonUpEventImGui(app, io, message, wParam);
    } break;

    case WM_RBUTTONDOWN:
    {
        ProcessMButtonEvent(&app->state.input, MouseButton::Right, true);
        ProcessMButtonDownAndDblclkEventImGui(app, io, message, wParam);
    } break;

    case WM_RBUTTONUP:
    {
        ProcessMButtonEvent(&app->state.input, MouseButton::Right, false);
        ProcessMButtonUpEventImGui(app, io, message, wParam);
    } break;

    case WM_MBUTTONDOWN:
    {
        ProcessMButtonEvent(&app->state.input, MouseButton::Middle, true);
        ProcessMButtonDownAndDblclkEventImGui(app, io, message, wParam);
    } break;

    case WM_MBUTTONUP:
    {
        ProcessMButtonEvent(&app->state.input, MouseButton::Middle, false);
        ProcessMButtonUpEventImGui(app, io, message, wParam);
    } break;

    case WM_XBUTTONDOWN:
    {
        auto state = HIWORD(wParam);
        if (state & XBUTTON1)
        {
            ProcessMButtonEvent(&app->state.input, MouseButton::XButton1, true);
        }
        else
        {
            ProcessMButtonEvent(&app->state.input, MouseButton::XButton2, true);
        }
        ProcessMButtonDownAndDblclkEventImGui(app, io, message, wParam);
    } break;

    case WM_XBUTTONUP:
    {
        auto state = HIWORD(wParam);
        if (state & XBUTTON1)
        {
            ProcessMButtonEvent(&app->state.input, MouseButton::XButton1, false);
        }
        else
        {
            ProcessMButtonEvent(&app->state.input,MouseButton::XButton2, false);
        }
        ProcessMButtonUpEventImGui(app, io, message, wParam);
    } break;

    case WM_LBUTTONDBLCLK:
    case WM_RBUTTONDBLCLK:
    case WM_MBUTTONDBLCLK:
    case WM_XBUTTONDBLCLK:
    {
        ProcessMButtonDownAndDblclkEventImGui(app, io, message, wParam);
    } break;

    case WM_MOUSELEAVE:
    {
        app->state.input.mouseInWindow = false;
    } break;

    case WM_MOUSEWHEEL:
    {
        i32 delta = GET_WHEEL_DELTA_WPARAM(wParam);
        i32 numSteps = delta / WHEEL_DELTA;
        app->state.input.scrollOffset = numSteps;
        app->state.input.scrollFrameOffset = numSteps;
        if (io)
        {
            io->MouseWheel += (float)numSteps;
        }
    } break;

    case WM_SETCURSOR:
    {
        if (LOWORD(lParam) == HTCLIENT && ImGui_ImplWin32_UpdateMouseCursor())
        {
            result = 1;
        }
    } break;

    // ^^^^ MOUSE INPUT
    // KEYBOARD INPUT

    case WM_SYSKEYDOWN:
    case WM_KEYDOWN:
    {
        // NOTE: Here are might be propblems with repeat count
        // Here were many todo's requesting to fix something
        // but I don't see any errors now
        u32 key = ConvertWin32Keycode(app, wParam);
        b32 state = true;
        u16 sys_repeat_count =  KEY_REPEAT_COUNT_FROM_LPARAM(lParam);
        app->state.input.keys[key].wasPressed = app->state.input.keys[key].pressedNow;
        app->state.input.keys[key].pressedNow = state;

        // TODO: Temorary
        if (app->state.input.keys[(u32)Key::L].pressedNow &&
            !app->state.input.keys[(u32)Key::L].wasPressed &&
            app->state.input.keys[(u32)Key::Ctrl].pressedNow)
        {
            WindowToggleFullscreen(app, !app->fullscreen);
        }
        if (io && wParam < 256)
        {
            io->KeysDown[wParam] = 1;
        }
    } break;

    case WM_SYSKEYUP:
    case WM_KEYUP:
    {
        u32 key = ConvertWin32Keycode(app, wParam);
        b32 state = false;
        u16 sys_repeat_count = 0;
        app->state.input.keys[key].wasPressed = app->state.input.keys[key].pressedNow;
        app->state.input.keys[key].pressedNow = state;
        if (io && wParam < 256)
        {
            io->KeysDown[wParam] = 0;
        }
    } break;

    case WM_CHAR:
    {
        if (io)
        {
            DWORD wChar = (DWORD)wParam;
            if (wChar <= 127)
            {
                io->AddInputCharacter((unsigned int)wParam);
            }
            else
            {
                // TODO: utf8 input support
                // NOTE: swap lower and upper part.
                BYTE low = (BYTE)(wChar & 0x00FF);
                BYTE high = (BYTE)((wChar & 0xFF00) >> 8);
                wChar = MAKEWORD(high, low);
                wchar_t ch[6];
                MultiByteToWideChar(CP_OEMCP, 0, (LPCSTR)&wChar, 4, ch, 3);
                io->AddInputCharacter(ch[0]);
            }
        }
    } break;
    // ^^^^ KEYBOARD INPUT

    case WM_ACTIVATEAPP:
    {
        if (wParam == TRUE)
        {
            app->state.input.activeApp = true;
        }
        else
        {
            app->state.input.activeApp = false;
        }
    } break;

    default:
    {
        result = DefWindowProc(windowHandle, message, wParam, lParam);
    } break;
    }
    return result;
}

#if 0
void Win32CreateGLContextForWorkerThread(u32 threadNum) {
    HGLRC glrc = GlobalContext.wglCreateContextAttribsARB(GlobalContext.windowDC, GlobalContext.openGLRC, OpenGLContextAttribs);
    panic(glrc, "[Error] Win32: failed to initialize OpenGL extended context (%lu)", HRESULT_CODE(GetLastError()));
    auto resultMC = wglMakeCurrent(GlobalContext.windowDC, glrc);
    panic(resultMC, "[Error] Win32: failed to initialize OpenGL extended context (%lu)", HRESULT_CODE(GetLastError()));
}
#endif

void Win32Init(Win32Context* ctx)
{
    ctx->wpPrev = {sizeof(WINDOWPLACEMENT)};
    auto instance = GetModuleHandle(0);

    WNDCLASS windowClass = {};
    windowClass.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
    windowClass.lpfnWndProc = Win32WindowProc;
    windowClass.hInstance = instance;
    windowClass.lpszClassName = TEXT("Flux");
    windowClass.hCursor = LoadCursor(0 ,IDC_ARROW);

    auto RCresult = RegisterClass(&windowClass);
    panic(RCresult, "[Error] Win32: failed to create window.");

    HWND fakeWindow = CreateWindowEx(0, windowClass.lpszClassName,
                                     TEXT("Dummy window"), WS_OVERLAPPEDWINDOW,
                                     CW_USEDEFAULT,  CW_USEDEFAULT, 1, 1,
                                     0, 0, instance, NULL);

    HDC fakeWindowDC = GetDC(fakeWindow);

    PIXELFORMATDESCRIPTOR fakeDesiredPixelFormat = {};
    fakeDesiredPixelFormat.nSize = sizeof(PIXELFORMATDESCRIPTOR);
    fakeDesiredPixelFormat.nVersion = 1;
    fakeDesiredPixelFormat.dwFlags = PFD_SUPPORT_OPENGL | PFD_DRAW_TO_WINDOW | PFD_DOUBLEBUFFER;
    fakeDesiredPixelFormat.cColorBits = 32;
    fakeDesiredPixelFormat.cAlphaBits = 8;
    fakeDesiredPixelFormat.cDepthBits = 24;
    fakeDesiredPixelFormat.cStencilBits = 8;

    auto fakeActualPFIndex = ChoosePixelFormat(fakeWindowDC, &fakeDesiredPixelFormat);

    PIXELFORMATDESCRIPTOR fakeActualPixelFormat = {};
    DescribePixelFormat(fakeWindowDC, fakeActualPFIndex, sizeof(PIXELFORMATDESCRIPTOR), &fakeActualPixelFormat);
    SetPixelFormat(fakeWindowDC, fakeActualPFIndex, &fakeActualPixelFormat);

    HGLRC fakeGLRC = wglCreateContext(fakeWindowDC);
    auto resultMC = wglMakeCurrent(fakeWindowDC, fakeGLRC);
    panic(resultMC, "[Error] Win32: failed to create OpenGL context.");
    // TODO: Should it release dc?
    //ReleaseDC(windowHandle, windowDC);

    auto wglLoadProcsResult = WglLoadFunctions(ctx, fakeWindowDC);
    panic(wglLoadProcsResult, "[Error] Win32: failed to load WGL extensions.");

    // NOTE: ACTUAL WINDOW

    RECT actualSize = {};
    actualSize.top = 0;
    actualSize.left = 0;
    actualSize.right = ctx->state.windowWidth;
    actualSize.bottom = ctx->state.windowHeight;

    AdjustWindowRectEx(&actualSize, WS_OVERLAPPEDWINDOW | WS_VISIBLE,  0, 0);

    i32 width = Abs(actualSize.left) + Abs(actualSize.right);
    i32 height = Abs(actualSize.top) + Abs(actualSize.bottom);

    auto styleFlags = WS_THICKFRAME | WS_OVERLAPPEDWINDOW | WS_VISIBLE;
    HWND actualWindowHandle = CreateWindowEx(0, windowClass.lpszClassName,
                                             ctx->windowTitle,
                                             styleFlags,
                                             CW_USEDEFAULT, CW_USEDEFAULT,
                                             width,
                                             height,
                                             0, 0, instance, ctx);

    panic(actualWindowHandle, "[Error] Win32: failed to create window.");

    HDC actualWindowDC = GetDC(actualWindowHandle);

    // NOTE: ^^^^ ACTUAL WINDOW

    int attribList[] = {
        WGL_DRAW_TO_WINDOW_ARB, GL_TRUE,
        WGL_SUPPORT_OPENGL_ARB, GL_TRUE,
        WGL_DOUBLE_BUFFER_ARB, GL_TRUE,
        WGL_FRAMEBUFFER_SRGB_CAPABLE_ARB, GL_TRUE,
        WGL_ACCELERATION_ARB, WGL_FULL_ACCELERATION_ARB,
        WGL_PIXEL_TYPE_ARB, WGL_TYPE_RGBA_ARB,
        WGL_COLOR_BITS_ARB, 32,
        WGL_DEPTH_BITS_ARB, 24,
        //WGL_STENCIL_BITS_ARB, 8,
        0
    };

    int actualPixelFormatID = 0;
    UINT numFormats = 0;
    auto resultCPF = ctx->wglChoosePixelFormatARB(actualWindowDC,
                                                  attribList, 0,
                                                  1, &actualPixelFormatID,
                                                  &numFormats);
    panic(resultCPF, "[Error] Win32: failed to initialize OpenGL extended context.");

    PIXELFORMATDESCRIPTOR actualPixelFormat = {};
    auto resultDPF = DescribePixelFormat(actualWindowDC,
                                         actualPixelFormatID,
                                         sizeof(PIXELFORMATDESCRIPTOR),
                                         &actualPixelFormat);
    panic(resultDPF, "[Error] Win32: failed to initialize OpenGL extended context.");
    SetPixelFormat(actualWindowDC, actualPixelFormatID, &actualPixelFormat);

    HGLRC actualGLRC = ctx->wglCreateContextAttribsARB(actualWindowDC, 0, OpenGLContextAttribs);
    panic(actualGLRC, "[Error] Win32: failed to initialize OpenGL extended context");

#if defined (OPENGL_WORKER_CONTEXTS)
    b32 supportsAsyncGPUTransfer = true;

    for (u32 i = 0; i < NumOfWorkerThreads; i++) {
        HGLRC glrc = ctx->wglCreateContextAttribsARB(actualWindowDC, actualGLRC, OpenGLContextAttribs);
        if (!glrc) {
            log_print("[win32] Failed to initialize OpenGL context for worker thread. Error %lu\n", HRESULT_CODE(GetLastError()));
            supportsAsyncGPUTransfer = false;
            break;
        }
        ctx->workersGLRC[i] = glrc;
    }
    ctx->state.supportsAsyncGPUTransfer = supportsAsyncGPUTransfer;
#endif

    wglMakeCurrent(0, 0);
    wglDeleteContext(fakeGLRC);
    ReleaseDC(fakeWindow, fakeWindowDC);
    DestroyWindow(fakeWindow);

    resultMC = wglMakeCurrent(actualWindowDC, actualGLRC);
    panic(resultMC, "[Error] Win32: failed to initialize OpenGL extended context");

    ctx->windowHandle = actualWindowHandle;
    ctx->windowDC = actualWindowDC;
    ctx->openGLRC = actualGLRC;

    ctx->Win32MouseTrackEvent.cbSize = sizeof(TRACKMOUSEEVENT);
    ctx->Win32MouseTrackEvent.dwFlags = TME_LEAVE;
    ctx->Win32MouseTrackEvent.dwHoverTime = HOVER_DEFAULT;
    ctx->Win32MouseTrackEvent.hwndTrack = ctx->windowHandle;
    TrackMouseEvent(&ctx->Win32MouseTrackEvent);
#if 0
    // Raw input
    RAWINPUTDEVICE mouseDevice = {};
    mouseDevice.usUsagePage = 0x01; // Generic Desktop Controls
    mouseDevice.usUsage = 0x02; // Mouse
    mouseDevice.dwFlags = 0;
    mouseDevice.hwndTarget = 0;

    auto regResult = RegisterRawInputDevices(&mouseDevice, 1, sizeof(mouseDevice));
    assert(regResult != FALSE);
#endif
    Win32InitKeyTable(ctx->keyTable);

    SetFocus(ctx->windowHandle);
}

f64 GetTimeStamp()
{
    f64 time = 0.0;
    LARGE_INTEGER currentTime = {};
    if (QueryPerformanceCounter(&currentTime))
    {
        time = (f64)(currentTime.QuadPart) / (f64)GlobalPerformanceFrequency.QuadPart;
    }
    return time;
}

void GetExecutablePath(TCHAR* buffer, u32 bufferSizeBytes, u32* bytesWritten)
{
    GetModuleFileName(NULL, buffer, bufferSizeBytes / sizeof(TCHAR));
}

#if defined(COMPILER_MSVC)
__declspec(restrict)
#endif
void* Allocate(uptr size, uptr alignment, void* data)
{
    auto memory = malloc(size);
    assert(memory);
    //log_print("[Platform] Allocate %llu bytes at address %llu\n", size, (u64)memory);
    return memory;
}

void Deallocate(void* ptr, void* data)
{
    //log_print("[Platform] Deallocating memory at address %llu\n", (u64)ptr);
    free(ptr);
}

#if defined(COMPILER_MSVC)
__declspec(restrict)
#endif
void* Reallocate(void* ptr, uptr newSize)
{
    return realloc(ptr, newSize);
}

void Win32Sleep(u32 ms)
{
    Sleep(ms);
}

void LoadResourceLoader(Win32Context* context)
{
    auto handle = LoadLibrary(L"flux_resource_loader.dll");
    assert(handle != INVALID_HANDLE_VALUE);
    context->state.functions.ResourceLoaderLoadImage = (ResourceLoaderLoadImageFn*)GetProcAddress(handle, "ResourceLoaderLoadImage");
    context->state.functions.ResourceLoaderValidateImageFile = (ResourceLoaderValidateImageFileFn*)GetProcAddress(handle, "ResourceLoaderValidateImageFile");

    assert(context->state.functions.ResourceLoaderLoadImage);
    assert(context->state.functions.ResourceLoaderValidateImageFile);
}

void* ImguiAllocWrapper(size_t size, void* _) { return Allocate((uptr)size, 0, nullptr); }
void ImguiFreeWrapper(void* ptr, void*_) { Deallocate(ptr, nullptr); }

#include "../ext/imgui/imgui_impl_opengl3.h"

#define gl_function(func) GlobalContext.state.gl->functions.fn.##func

#define glViewport gl_function(glViewport)
#define glClearColor gl_function(glClearColor)
#define glClear gl_function(glClear)

b32 Win32PushWork(WorkQueue* queue, WorkFn* fn, void* data0, void* data1, void* data2) {
    bool result = false;
    auto nextEntry = (queue->end + 1) % array_count(queue->queue);
    if (nextEntry != queue->begin) {
        auto entry = queue->queue + queue->end;
        entry->function = fn;
        entry->data0 = data0;
        entry->data1 = data1;
        entry->data2 = data2;
        queue->pendingWorkCount++;
        WriteFence();
        // TODO: Maybe it should be atomic?
        queue->end = nextEntry;
        ReleaseSemaphore(GlobalContext.workQueueSemaphore, 1, nullptr);
        result = true;
    }
    return result;
}

bool Win32DoWorkerWork(WorkQueue* queue, u32 threadIndex) {
    bool result = false;
    auto oldBegin = queue->begin;
    auto newBegin = (oldBegin + 1) % array_count(queue->queue);
    // TODO: new? or old?
    if (oldBegin != queue->end) {
        u32 index = InterlockedCompareExchange(((LONG volatile*)&queue->begin), newBegin, oldBegin);
        if (index == oldBegin) {
            auto entry = queue->queue + index;
            if (entry->function) {
                entry->function(entry->data0, entry->data1, entry->data2,  threadIndex);
            }
            InterlockedIncrement((LONG volatile*)&queue->completedWorkCount);
            result = true;
        }
    }
    return result;
}

void Win32CompleteAllWork(WorkQueue* queue) {
    auto threadId = GetCurrentThreadId();
    while (queue->pendingWorkCount != queue->completedWorkCount) {
        Win32DoWorkerWork(queue, threadId);
    }
    queue->pendingWorkCount = 0;
    queue->completedWorkCount = 0;
}

DWORD WINAPI Win32ThreadProc(void* param) {
    auto threadInfo = (Win32ThreadInfo*)param;
    auto lowPriorityQueue = threadInfo->lowPriorityQueue;
    auto highPriorityQueue = threadInfo->highPriorityQueue;

    // NOTE: This way of initializaing shared contexts appears to be working.
    // Multiple context support for working threads in OpenGL seems to be super inconsistent
    // so the way of creaing them should be carefully explored.
    // Nvidia's way of creating contexts (http://on-demand.gputechconf.com/gtc/2012/presentations/S0356-GTC2012-Texture-Transfers.pdf)
    // apperas not to be working.
#if defined (OPENGL_WORKER_CONTEXTS)
    auto windowDC = GetDC(GlobalContext.windowHandle);
    auto result = wglMakeCurrent(windowDC, threadInfo->glrc);
    if (!result) {
        log_print("[win32] Failed to make OpenGL context current for worker thread. Error %lu", HRESULT_CODE(GetLastError()));
        _InterlockedExchange((long volatile*)&GlobalContext.state.supportsAsyncGPUTransfer, 0);
    }
#endif
    while (true) {
        auto didHighPriorityWork = Win32DoWorkerWork(highPriorityQueue, threadInfo->index);
        if (!didHighPriorityWork) {
            //WaitForSingleObjectEx(highPriorityQueue->semaphore, 0, FALSE);
            auto didLowPriorityWork = Win32DoWorkerWork(lowPriorityQueue, threadInfo->index);
            if (!didLowPriorityWork) {
                WaitForSingleObjectEx(GlobalContext.workQueueSemaphore, INFINITE, FALSE);
            }
        }
    }
}

void Win32SetSaveThreadWork(SaveThreadWorkFn* func, void* data, u32 timeoutMs) {
    // TODO: Disallow code reloading while save thread is active
    WaitForSingleObjectEx(GlobalContext.saveThreadMutex, INFINITE, FALSE);
    GlobalContext.SaveThreadData = data;
    GlobalContext.SaveThreadWork = func;
    GlobalContext.saveTimeout = timeoutMs;
    auto releaseResult = ReleaseMutex(GlobalContext.saveThreadMutex);
    assert(releaseResult);
    ResumeThread(GlobalContext.saveThreadHandle);
}

DWORD WINAPI Win32SaveThreadProc(void* param) {
    while (true) {
        WaitForSingleObjectEx(GlobalContext.saveThreadMutex, INFINITE, FALSE);

        if (GlobalContext.SaveThreadWork) {
            GlobalContext.SaveThreadWork(GlobalContext.SaveThreadData);
        }

        auto releaseResult = ReleaseMutex(GlobalContext.saveThreadMutex);
        assert(releaseResult);

        Sleep(GlobalContext.saveTimeout);
    }
}

void* Win32AllocatePages(uptr size) {
    void* block = VirtualAlloc(0, size, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
    assert(block);
    return block;
}

void Win32DeallocatePages(void* memory, uptr size) {
    auto result = VirtualFree(memory, 0, MEM_RELEASE);
    assert(result);
}

MemoryArena* Win32AllocateArena(uptr size) {
    uptr headerSize = sizeof(MemoryArena);
    void* mem = VirtualAlloc(0, size + headerSize,
                             MEM_RESERVE | MEM_COMMIT,
                             PAGE_READWRITE);
    assert(mem, "Allocation failed");
    assert((uptr)mem % 128 == 0, "Memory aligment violation");
    MemoryArena header = {};
    header.free = size;
    header.size = size;
    header.begin = (void*)((byte*)mem + headerSize);
    memcpy(mem, &header, sizeof(MemoryArena));
    return (MemoryArena*)mem;
}

void Win32FreeArena(MemoryArena* arena) {
    void* base = (void*)((byte*)arena->begin - sizeof(MemoryArena));
    auto result = VirtualFree(base, 0, MEM_RELEASE);
    assert(result);
}


int WINAPI WinMain(HINSTANCE instance, HINSTANCE prevInstance, LPSTR cmdLine, int showCmd)
{
#if defined(ENABLE_CONSOLE)
    AllocConsole();
    freopen("CONOUT$", "w", stdout);
#endif

    auto app = &GlobalContext;

    UINT sleepGranularityMs = 1;
    auto granularityWasSet = (timeBeginPeriod(sleepGranularityMs) == TIMERR_NOERROR);

    QueryPerformanceFrequency(&GlobalPerformanceFrequency);

    _tcscpy(app->windowTitle, TEXT("PBR Demo"));
    app->state.windowWidth = 1280;
    app->state.windowHeight = 720;

    Win32Init(app);

    app->saveThreadMutex = CreateMutex(0, FALSE, nullptr);
    assert(app->saveThreadMutex);

    app->saveTimeout = 3000;

    DWORD saveThreadID;
    app->saveThreadHandle = CreateThread(0, 0, Win32SaveThreadProc, nullptr, CREATE_SUSPENDED, &saveThreadID);

    app->wglSwapIntervalEXT(1);

    auto lowQueue = &app->lowPriorityQueue;
    auto highQueue = &app->highPriorityQueue;

    Win32ThreadInfo threadInfo[NumOfWorkerThreads];
    auto semaphore = CreateSemaphoreEx(0, 0, array_count(threadInfo), nullptr, 0, SEMAPHORE_ALL_ACCESS);

    app->workQueueSemaphore = semaphore;

    for (u32x i = 0; i < array_count(threadInfo); i++) {
        auto info = threadInfo + i;
        DWORD threadId;
        auto threadHandle = CreateThread(0, 0, Win32ThreadProc, (void*)info, CREATE_SUSPENDED, &threadId);
        info->index = GetThreadId(threadHandle);
        info->lowPriorityQueue = lowQueue;
        info->highPriorityQueue = highQueue;
        info->glrc = app->workersGLRC[i];
        ResumeThread(threadHandle);
        CloseHandle(threadHandle);
    }

    Win32CompleteAllWork(lowQueue);
    Win32CompleteAllWork(highQueue);

    OpenGLLoadResult glResult = LoadOpenGL();
    panic(glResult.success, "Failed to load OpenGL functions");
    app->state.gl = glResult.context;

    LoadResourceLoader(app);

    app->state.functions.DebugGetFileSize = DebugGetFileSize;
    app->state.functions.DebugReadFile = DebugReadFileToBuffer;
    app->state.functions.DebugReadTextFile = DebugReadTextFileToBuffer;
    app->state.functions.DebugWriteFile = DebugWriteFile;
    app->state.functions.DebugOpenFile = DebugOpenFile;
    app->state.functions.DebugCloseFile = DebugCloseFile;
    app->state.functions.DebugCopyFile = DebugCopyFile;
    app->state.functions.DebugWriteToOpenedFile = DebugWriteToOpenedFile;

    app->state.functions.Allocate = Allocate;
    app->state.functions.Deallocate = Deallocate;
    app->state.functions.Reallocate = Reallocate;

    app->state.functions.AllocatePages = Win32AllocatePages;
    app->state.functions.DeallocatePages = Win32DeallocatePages;

    app->state.functions.AllocateArena = Win32AllocateArena;
    app->state.functions.FreeArena = Win32FreeArena;

    app->state.functions.PushWork = Win32PushWork;
    app->state.functions.CompleteAllWork = Win32CompleteAllWork;
    app->state.functions.SetSaveThreadWork = Win32SetSaveThreadWork;

    app->state.functions.GetTimeStamp = GetTimeStamp;

    app->state.functions.EnumerateFilesInDirectory = EnumerateFilesInDirectory;

    app->state.lowPriorityQueue = lowQueue;
    app->state.highPriorityQueue = highQueue;

    //SetupDirs(&app->gameLib);

    b32 codeLoaded = UpdateGameCode(&app->gameLib);
    assert(codeLoaded, "Failed to load game lib");

    IMGUI_CHECKVERSION();
    ImGui::SetAllocatorFunctions(ImguiAllocWrapper, ImguiFreeWrapper, 0);
    app->state.imguiContext = ImGui::CreateContext();
    assert(app->state.imguiContext);
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;


    ImGui::StyleColorsDark();
    //ImGuiStyleColorsLightGreen();
    io.IniFilename = 0;

    ImGuiStyle& style = ImGui::GetStyle();
    style.WindowRounding = 0.0f;
    style.Colors[ImGuiCol_WindowBg].w = 1.0f;

    ImGui_ImplWin32_Init(app->windowHandle);
    auto imResult = ImGui_ImplOpenGL3_Init("#version 330 core");
    panic(imResult);

    f64 tickTimer = 1.0f;
    u32 updatesSinceLastTick = 0;

    app->state.localTime = GetLocalTime();

    app->gameLib.GameUpdateAndRender(&app->state, GameInvoke::Init, &GlobalGameData);

    while (GlobalRunning)
    {
        app->state.tickCount++;
        auto tickStartTime = GetTimeStamp();

        WindowPollEvents(app);

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplWin32_NewFrame();
        ImGui::NewFrame();

        if (tickTimer <= 0)
        {
            tickTimer = 1.0f;
            app->state.ups = updatesSinceLastTick;
            updatesSinceLastTick= 0;
        }

        app->state.localTime = GetLocalTime();

        bool codeReloaded = UpdateGameCode(&app->gameLib);
        if (codeReloaded)
        {
            app->gameLib.GameUpdateAndRender(&app->state, GameInvoke::Reload, &GlobalGameData);
        }

        updatesSinceLastTick++;
        app->gameLib.GameUpdateAndRender(&app->state, GameInvoke::Update, &GlobalGameData);

        app->gameLib.GameUpdateAndRender(&app->state, GameInvoke::Render, &GlobalGameData);

        ImGui::Render();

        glViewport(0, 0, app->state.windowWidth, app->state.windowHeight);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        SwapBuffers(app->windowDC);

        for (u32 keyIndex = 0; keyIndex < InputState::KeyCount; keyIndex ++)
        {
            app->state.input.keys[keyIndex].wasPressed = app->state.input.keys[keyIndex].pressedNow;
        }

        for (u32 mbIndex = 0; mbIndex < InputState::MouseButtonCount; mbIndex++)
        {
            app->state.input.mouseButtons[mbIndex].wasPressed = app->state.input.mouseButtons[mbIndex].pressedNow;
        }

        app->state.input.scrollFrameOffset = 0;
        app->state.input.mouseFrameOffsetX = 0;
        app->state.input.mouseFrameOffsetY = 0;

        auto tickEndTime = GetTimeStamp();
        auto timeElapsed = tickEndTime - tickStartTime;
        while (timeElapsed < SECONDS_PER_TICK)
        {
            if (granularityWasSet)
            {
                auto waitTime = (DWORD)(SECONDS_PER_TICK - timeElapsed) * 1000;
                if (waitTime)
                {
                    Sleep(waitTime);
                }
            }
            auto nowTime = GetTimeStamp();
            timeElapsed = nowTime - tickStartTime;
        }

        tickTimer -= timeElapsed;
        app->state.absDeltaTime = (f32)timeElapsed;
        app->state.fps = (i32)(1.0f / app->state.absDeltaTime);
        app->state.gameDeltaTime = app->state.absDeltaTime * app->state.gameSpeed;
    }
}

#include "Win32CodeLoader.cpp"

// Functions used by imgui

#define glGetIntegerv gl_function(glGetIntegerv)
#define glBindSampler gl_function(glBindSampler)
#define glIsEnabled gl_function(glIsEnabled)
#define glScissor gl_function(glScissor)
#define glDrawElementsBaseVertex gl_function(glDrawElementsBaseVertex)
#define glDeleteVertexArrays gl_function(glDeleteVertexArrays)
#define glBindSampler gl_function(glBindSampler)
#define glBlendEquationSeparate gl_function(glBlendEquationSeparate)
#define glBlendFuncSeparate gl_function(glBlendFuncSeparate)
#define glPixelStorei gl_function(glPixelStorei)
#define glGetAttribLocation gl_function(glGetAttribLocation)
#define glDeleteBuffers gl_function(glDeleteBuffers)
#define glDetachShader gl_function(glDetachShader)
#define glDeleteProgram gl_function(glDeleteProgram)
#define glEnable gl_function(glEnable)
#define glBlendEquation gl_function(glBlendEquation)
#define glBlendFunc gl_function(glBlendFunc)
#define glDisable gl_function(glDisable)
#define glPolygonMode gl_function(glPolygonMode)
#define glUseProgram gl_function(glUseProgram)
#define glUniform1i gl_function(glUniform1i)
#define glUniformMatrix4fv gl_function(glUniformMatrix4fv)
#define glBindVertexArray gl_function(glBindVertexArray)
#define glBindBuffer gl_function(glBindBuffer)
#define glEnableVertexAttribArray gl_function(glEnableVertexAttribArray)
#define glVertexAttribPointer gl_function(glVertexAttribPointer)
#define glActiveTexture gl_function(glActiveTexture)
#define glGenVertexArrays gl_function(glGenVertexArrays)
#define glBufferData gl_function(glBufferData)
#define glBindTexture gl_function(glBindTexture)
#define glTexParameteri gl_function(glTexParameteri)
#define glTexImage2D gl_function(glTexImage2D)
#define glGenTextures gl_function(glGenTextures)
#define glDeleteTextures gl_function(glDeleteTextures)
#define glGetShaderiv gl_function(glGetShaderiv)
#define glGetShaderInfoLog gl_function(glGetShaderInfoLog)
#define glGetProgramiv gl_function(glGetProgramiv)
#define glCreateShader gl_function(glCreateShader)
#define glShaderSource gl_function(glShaderSource)
#define glCompileShader gl_function(glCompileShader)
#define glCreateProgram gl_function(glCreateProgram)
#define glAttachShader gl_function(glAttachShader)
#define glLinkProgram gl_function(glLinkProgram)
#define glGetUniformLocation gl_function(glGetUniformLocation)
#define glGetProgramInfoLog gl_function(glGetProgramInfoLog)
#define glGenBuffers gl_function(glGenBuffers)
#define glDeleteShader gl_function(glDeleteShader)
#define glDrawElements gl_function(glDrawElements)

// NOTE: IMGUI
#include "../ext/imgui/imconfig.h"
#include "../ext/imgui/imgui.cpp"
#include "../ext/imgui/imgui_draw.cpp"
#include "../ext/imgui/imgui_widgets.cpp"
#include "../ext/imgui/imgui_demo.cpp"
#include "../ext/imgui/imgui_impl_opengl3.cpp"
#include "../ext/imgui/imgui_impl_win32.cpp"
