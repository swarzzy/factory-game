#include "Platform.h"
#include "Game.h"
#include "Memory.h"

#define DEBUG_OPENGL
// NOTE: Defined only in debug build
#include <stdlib.h>

void OpenglDebugCallback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, const GLvoid* userParam);

// NOTE: Platforn globals
static PlatformState* _GlobalPlatform = 0;
#define GlobalPlatform (*((_GlobalPlatform)))
// NOTE: Actual frame time
#define GlobalAbsDeltaTime GlobalPlatform.absDeltaTime
// NOTE: Frame time corrected by game speed
#define GlobalGameDeltaTime GlobalPlatform.gameDeltaTime
#define GlobalInput GlobalPlatform.input
#define GlobalPlaformWorkQueue GlobalPlatform.workQueue

bool KeyHeld(Key key) {
    return GlobalInput.keys[(u32)key].pressedNow;
}

bool KeyPressed(Key key) {
    return GlobalInput.keys[(u32)key].pressedNow && !GlobalInput.keys[(u32)key].wasPressed;
}

bool MouseButtonHeld(MouseButton button) {
    return GlobalInput.mouseButtons[(u32)button].pressedNow;
}

bool MouseButtonPressed(MouseButton button) {
    return GlobalInput.mouseButtons[(u32)button].pressedNow && !GlobalInput.mouseButtons[(u32)button].wasPressed;
}


#if defined(COMPILER_MSVC)
#define platform_call(func) GlobalPlatform.functions.##func
#else
#define platform_call(func) GlobalPlatform.functions. func
#endif

#define PlatformAlloc platform_call(Allocate)
#define PlatformFree platform_call(Deallocate)
#define PlatformRealloc platform_call(Reallocate)
#define PlatformDebugGetFileSize platform_call(DebugGetFileSize)
#define PlatformDebugReadFile platform_call(DebugReadFile)
#define PlatformDebugWriteFile platform_call(DebugWriteFile)
#define PlatformDebugCopyFile platform_call(DebugCopyFile)
#define ResourceLoaderLoadImage platform_call(ResourceLoaderLoadImage)
#define ResourceLoaderValidateImageFile platform_call(ResourceLoaderValidateImageFile)
#define PlatformGetTimeStamp platform_call(GetTimeStamp)
#define PlatformAllocateArena platform_call(AllocateArena)
#define PlatformFreeArena platform_call(FreeArena)

void* PlatformAllocClear(uptr size) {
    void* memory = PlatformAlloc(size, 0, nullptr);
    memset(memory, 0, size);
    return memory;
}

#define PlatformPushWork platform_call(PushWork)
#define PlatformCompleteAllWork platform_call(CompleteAllWork)
#define PlatformSleep platform_call(Sleep)

#if defined(COMPILER_MSVC)
#define gl_call(func) GlobalPlatform.gl->functions.fn.##func
#else
#define platform_call(func) GlobalPlatform.gl->functions.fn. func
#endif

#define glGenTextures gl_call(glGenTextures)
#define glBindTexture gl_call(glBindTexture)
#define glTexParameteri gl_call(glTexParameteri)
#define glTexImage2D gl_call(glTexImage2D)
#define glDeleteTextures gl_call(glDeleteTextures)
#define glPolygonMode gl_call(glPolygonMode)
#define glDisable gl_call(glDisable)
#define glClearColor gl_call(glClearColor)
#define glEnable gl_call(glEnable)
#define glBindBuffer gl_call(glBindBuffer)
#define glBufferData gl_call(glBufferData)
#define glEnableVertexAttribArray gl_call(glEnableVertexAttribArray)
#define glVertexAttribPointer gl_call(glVertexAttribPointer)
#define glUseProgram gl_call(glUseProgram)
#define glActiveTexture gl_call(glActiveTexture)
#define glUniform1i gl_call(glUniform1i)
#define glUniformSubroutinesuiv gl_call(glUniformSubroutinesuiv)
#define glDrawElements gl_call(glDrawElements)
#define glGenBuffers gl_call(glGenBuffers)
#define glCreateShader gl_call(glCreateShader)
#define glShaderSource gl_call(glShaderSource)
#define glCompileShader gl_call(glCompileShader)
#define glGetShaderiv gl_call(glGetShaderiv)
#define glGetShaderInfoLog gl_call(glGetShaderInfoLog)
#define glCreateProgram gl_call(glCreateProgram)
#define glAttachShader gl_call(glAttachShader)
#define glLinkProgram gl_call(glLinkProgram)
#define glGetProgramiv gl_call(glGetProgramiv)
#define glGetProgramInfoLog gl_call(glGetProgramInfoLog)
#define glViewport gl_call(glViewport)
#define glDeleteShader gl_call(glDeleteShader)
#define glGetSubroutineIndex gl_call(glGetSubroutineIndex)
#define glGetUniformLocation gl_call(glGetUniformLocation)
#define glTexImage2DMultisample gl_call(glTexImage2DMultisample)
#define glGenFramebuffers gl_call(glGenFramebuffers)
#define glBindFramebuffer gl_call(glBindFramebuffer)
#define glCheckFramebufferStatus gl_call(glCheckFramebufferStatus)
#define glFramebufferTexture2D gl_call(glFramebufferTexture2D)
#define glClear gl_call(glClear)
#define glMapBuffer gl_call(glMapBuffer)
#define glUnmapBuffer gl_call(glUnmapBuffer)
#define glDepthMask gl_call(glDepthMask)
#define glDepthFunc gl_call(glDepthFunc)
#define glBlendEquation gl_call(glBlendEquation)
#define glBlendFunc gl_call(glBlendFunc)
#define glCullFace gl_call(glCullFace)
#define glFrontface gl_call(glFrontface)
#define glGenVertexArrays gl_call(glGenVertexArrays)
#define glBindVertexArray gl_call(glBindVertexArray)
#define glFrontFace gl_call(glFrontFace)
#define glGetUniformBlockIndex gl_call(glGetUniformBlockIndex)
#define glUniformBlockBinding gl_call(glUniformBlockBinding)
#define glBindBufferRange gl_call(glBindBufferRange)
#define glBindBufferBase gl_call(glBindBufferBase)
#define glDrawArrays gl_call(glDrawArrays)
#define glUniform1f gl_call(glUniform1f)
#define glBufferSubData gl_call(glBufferSubData)
#define glUniform3fv gl_call(glUniform3fv)
#define glLineWidth gl_call(glLineWidth)
#define glUniformMatrix4fv gl_call(glUniformMatrix4fv)
#define glBlitFramebuffer gl_call(glBlitFramebuffer)
#define glTexParameterfv gl_call(glTexParameterfv)
#define glVertexAttribDivisor gl_call(glVertexAttribDivisor)
#define glDrawElementsInstanced gl_call(glDrawElementsInstanced)
#define glDrawArraysInstanced gl_call(glDrawArraysInstanced)
#define glClearDepth gl_call(glClearDepth)
#define glTexImage3D gl_call(glTexImage3D)
#define glTexSubImage3D gl_call(glTexSubImage3D)
#define glTexStorage3D gl_call(glTexStorage3D)
#define glGenerateMipmap gl_call(glGenerateMipmap)
#define glTexParameterf gl_call(glTexParameterf)
#define glCreateBuffers gl_call(glCreateBuffers)
#define glNamedBufferData gl_call(glNamedBufferData)
#define glMapNamedBuffer gl_call(glMapNamedBuffer)
#define glUnmapNamedBuffer gl_call(glUnmapNamedBuffer)
#define glUniformMatrix3fv gl_call(glUniformMatrix3fv)
#define glUniform4fv gl_call(glUniform4fv)
#define glVertexAttribIPointer gl_call(glVertexAttribIPointer)
#define glGetFloatv gl_call(glGetFloatv)
#define glUniform2fv gl_call(glUniform2fv)
#define glFinish gl_call(glFinish)
#define glGetTexImage gl_call(glGetTexImage)
#define glFlush gl_call(glFlush)
#define glDrawBuffer gl_call(glDrawBuffer)
#define glReadBuffer gl_call(glReadBuffer)
#define glPolygonOffset gl_call(glPolygonOffset)
#define glTexImage1D gl_call(glTexImage1D)
#define glFramebufferTextureLayer gl_call(glFramebufferTextureLayer)
#define glNamedBufferStorage gl_call(glNamedBufferStorage)
#define glBindBufferRange gl_call(glBindBufferRange)
#define glNamedBufferSubData gl_call(glNamedBufferSubData)
#define glBufferStorage gl_call(glBufferStorage)
#define glBindTextureUnit gl_call(glBindTextureUnit)
#define glDebugMessageCallback gl_call(glDebugMessageCallback)
#define glDebugMessageControl gl_call(glDebugMessageControl)
#define glDeleteProgram gl_call(glDeleteProgram)
#define glGetIntegerv gl_call(glGetIntegerv)
#define glDeleteBuffers gl_call(glDeleteBuffers)
#define glMapBufferRange gl_call(glMapBufferRange)
#define glMapNamedBufferRange gl_call(glMapNamedBufferRange)
#define glVertexAttribIPointer gl_call(glVertexAttribIPointer)
#define glClientWaitSync gl_call(glClientWaitSync)
#define glFenceSync gl_call(glFenceSync)
#define glDeleteSync gl_call(glDeleteSync)
#include "Memory.h"
// NOTE: Libs

void* PlatformCalloc(uptr num, uptr size) { void* ptr = PlatformAlloc(num * size, 0, nullptr); memset(ptr, 0, num * size); return ptr; }

#define IMGUI_DEFINE_MATH_OPERATORS
#include "../ext/imgui/imgui.h"

void* ImguiAllocWrapper(size_t size, void* _) { return PlatformAlloc((uptr)size, 0, nullptr); }
void ImguiFreeWrapper(void* ptr, void*_) { PlatformFree(ptr, nullptr); }

#include "../ext/imgui/imgui_internal.h"

extern "C" GAME_CODE_ENTRY void __cdecl GameUpdateAndRender(PlatformState* platform, GameInvoke reason, void** data) {
    switch (reason) {
    case GameInvoke::Init: {
        IMGUI_CHECKVERSION();
        ImGui::SetAllocatorFunctions(ImguiAllocWrapper, ImguiFreeWrapper, nullptr);
        ImGui::SetCurrentContext(platform->imguiContext);
        _GlobalPlatform = platform;

        //platform->supportsAsyncGPUTransfer = false;

        printf("[Info] Asynchronous GPU memory transfer supported: %s\n", platform->supportsAsyncGPUTransfer ? "true" : "false");

        platform->gameSpeed = 1.0f;

#if defined(DEBUG_OPENGL)
        glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
        glDebugMessageCallback(OpenglDebugCallback, 0);
        glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, 0, GL_TRUE);
        glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DEBUG_SEVERITY_NOTIFICATION, 0, 0, GL_FALSE);
        glDebugMessageControl(GL_DONT_CARE, GL_DEBUG_TYPE_OTHER, GL_DEBUG_SEVERITY_LOW, 0, 0, GL_FALSE);
#endif
        // NOTE: Looks like this syntax actually incorrect and somehow leads to stack owerflow in this case!
        //*context = {};
        auto gameArena = PlatformAllocateArena(Megabytes(1024));
        auto tempArena = AllocateSubArena(gameArena, gameArena->size / 2, true);
        auto contextMemory = PushSize(gameArena, sizeof(Context));
        auto context = new(contextMemory) Context();
        *data = context;
        context->gameArena = gameArena;
        context->tempArena = tempArena;

        context->renderer = InitializeRenderer(gameArena, tempArena, UV2(GlobalPlatform.windowWidth, GlobalPlatform.windowHeight), 8);

        //context->renderer->clearColor = V4(0.8f, 0.8f, 0.8f, 1.0f);
        context->renderGroup = RenderGroup::Make(gameArena, Megabytes(32), 8192 * 2 * 2);

        FluxInit(context);
    } break;
    case GameInvoke::Reload: {
        auto context = (Context*)(*data);
        IMGUI_CHECKVERSION();
        ImGui::SetAllocatorFunctions(ImguiAllocWrapper, ImguiFreeWrapper, nullptr);
        ImGui::SetCurrentContext(platform->imguiContext);
        _GlobalPlatform = platform;
#if defined(DEBUG_OPENGL)
        glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
        glDebugMessageCallback(OpenglDebugCallback, 0);
#endif
        RecompileShaders(context->tempArena, context->renderer);
        printf("[Info] Game was hot-reloaded\n");
        FluxReload(context);
    } break;
    case GameInvoke::Update: {
        FluxUpdate((Context*)(*data));
    } break;
    case GameInvoke::Render: {
        FluxRender((Context*)(*data));
    } break;
    invalid_default();
    }
}

void OpenglDebugCallback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, const GLvoid* userParam) {
    const char* sourceStr;
    const char* typeStr;
    const char* severityStr;

    switch (source) {
    case GL_DEBUG_SOURCE_API: { sourceStr = "API"; } break;
    case GL_DEBUG_SOURCE_WINDOW_SYSTEM: { sourceStr = "window system"; } break;
    case GL_DEBUG_SOURCE_SHADER_COMPILER: { sourceStr = "shader compiler"; } break;
    case GL_DEBUG_SOURCE_THIRD_PARTY: { sourceStr = "third party"; } break;
    case GL_DEBUG_SOURCE_APPLICATION: { sourceStr = "application"; } break;
    case GL_DEBUG_SOURCE_OTHER: { sourceStr = "other"; } break;
    invalid_default();
    }

    switch (type) {
    case GL_DEBUG_TYPE_ERROR: { typeStr = "error"; } break;
    case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR: { typeStr = "deprecated behavior"; } break;
    case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR: { typeStr = "undefined behavior"; } break;
    case GL_DEBUG_TYPE_PORTABILITY: { typeStr = "portability problem"; } break;
    case GL_DEBUG_TYPE_PERFORMANCE: { typeStr = "performance problem"; } break;
    case GL_DEBUG_TYPE_OTHER: { typeStr = "other"; } break;
    invalid_default();
    }

    switch (severity) {
    case GL_DEBUG_SEVERITY_HIGH: { severityStr = "high"; } break;
    case GL_DEBUG_SEVERITY_MEDIUM: { severityStr = "medium"; } break;
    case GL_DEBUG_SEVERITY_LOW: { severityStr = "low"; } break;
    case GL_DEBUG_SEVERITY_NOTIFICATION: { severityStr = "notification"; } break;
    default: { severityStr = "unknown"; } break;
    }
    printf("[OpenGL] Debug message (source: %s, type: %s, severity: %s): %s\n", sourceStr, typeStr, severityStr, message);
}

#include "Game.cpp"
#include "Math.cpp"
#include "DebugOverlay.cpp"
#include "Camera.cpp"
#include "RenderGroup.cpp"
#include "Renderer.cpp"
#include "Resource.cpp"
#include "Shaders.cpp"
#include "Memory.cpp"
#include "HashMap.cpp"

#include "World.cpp"
#include "MeshGenerator.cpp"
#include "Region.cpp"
#include "WorldGen.cpp"


// NOTE: Platform specific intrinsics implementation begins here
#if defined(PLATFORM_WINDOWS)
#include <windows.h>
#else
#error Unsupported OS
#endif
#include "Intrinsics.cpp"

#include "../ext/imgui/imconfig.h"
#include "../ext/imgui/imgui.cpp"
#include "../ext/imgui/imgui_draw.cpp"
#include "../ext/imgui/imgui_widgets.cpp"
#include "../ext/imgui/imgui_demo.cpp"
