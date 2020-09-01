#include "../Common.h"
#include "../Platform.h"
#include "../DebugOverlay.h"
#include "OpenGL.h"

#define DEBUG_OPENGL

// TODO(swarzzy): Use logger from the game
void DefaultLogger(void* data, const char* fmt, va_list* args) {
    vprintf(fmt, *args);
}

inline void DefaultAssertHandler(void* data, const char* file, const char* func, u32 line, const char* assertStr, const char* fmt, va_list* args) {
    log_print("[Assertion failed] Expression (%s) result is false\nFile: %s, function: %s, line: %d.\n", assertStr, file, func, (int)line);
    if (args) {
        GlobalLogger(GlobalLoggerData, fmt, args);
    }
    debug_break();
}

static LoggerFn* GlobalLogger = DefaultLogger;
static void* GlobalLoggerData = nullptr;;

static AssertHandlerFn* GlobalAssertHandler = DefaultAssertHandler;
static void* GlobalAssertHandlerData = nullptr;

static PlatformState* _GlobalPlatform = nullptr;
static OpenGL* _GlobalAPI = nullptr;
static struct RendererContext* _GlobalContext = nullptr;

#include "OpenglDefs.h"
#include "OpenglRenderer.h"

// NOTE(swarzzy): For memcpy
#include <string.h>

struct RendererContext {
    Renderer renderer;
};

inline const PlatformState* GetPlatform() { return _GlobalPlatform; }
inline const InputState* GetInput() { return &_GlobalPlatform->input; }
inline Renderer* GetRenderer() { return &_GlobalContext->renderer; }

#define Platform (*(const PlatformCalls*)(&_GlobalPlatform->functions))

// ImGui
#define IMGUI_DEFINE_MATH_OPERATORS
#include "../../ext/imgui/imgui.h"

void* ImguiAllocWrapper(size_t size, void* _) { return Platform.Allocate((uptr)size, 0, nullptr); }
void ImguiFreeWrapper(void* ptr, void*_) { Platform.Deallocate(ptr, nullptr); }

#include "../../ext/imgui/imgui_internal.h"


void OpenglDebugCallback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, const GLvoid* userParam);

extern "C" GAME_CODE_ENTRY void __cdecl RendererPlatformInvoke(RendererInvoke invoke, PlatformState* platform, void* apiData, void** rendererData) {
    switch (invoke) {
    case RendererInvoke::Init: {
        _GlobalPlatform = platform;
        _GlobalAPI = (OpenGL*)apiData;
        _GlobalContext = (RendererContext*)Platform.Allocate(sizeof(RendererContext), alignof(RendererContext), nullptr);
        (*_GlobalContext) = {};
        assert(_GlobalContext);
        *rendererData = (void*)_GlobalContext;

        IMGUI_CHECKVERSION();
        ImGui::SetAllocatorFunctions(ImguiAllocWrapper, ImguiFreeWrapper, nullptr);
        ImGui::SetCurrentContext(platform->imguiContext);

#if defined(DEBUG_OPENGL)
        glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
        glDebugMessageCallback(OpenglDebugCallback, 0);
        glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, 0, GL_TRUE);
        glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DEBUG_SEVERITY_NOTIFICATION, 0, 0, GL_FALSE);
        glDebugMessageControl(GL_DONT_CARE, GL_DEBUG_TYPE_OTHER, GL_DEBUG_SEVERITY_LOW, 0, 0, GL_FALSE);
#endif

    } break;
    case RendererInvoke::Reload: {
        _GlobalPlatform = platform;
        _GlobalAPI = (OpenGL*)apiData;
        _GlobalContext = (RendererContext*)(*rendererData);

        IMGUI_CHECKVERSION();
        ImGui::SetAllocatorFunctions(ImguiAllocWrapper, ImguiFreeWrapper, nullptr);
        ImGui::SetCurrentContext(platform->imguiContext);

#if defined(DEBUG_OPENGL)
        glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
        glDebugMessageCallback(OpenglDebugCallback, 0);
#endif

    } break;
    invalid_default();
    }
}

extern "C" GAME_CODE_ENTRY RendererInfo __cdecl RendererGetInfo() {
    RendererInfo info {};
    auto renderer = GetRenderer();
    info.state.wRenderResolution = renderer->renderRes.x;
    info.state.hRenderResolution = renderer->renderRes.y;
    info.state.sampleCount = renderer->sampleCount;
    info.caps.maxSampleCount = renderer->maxSupportedSampleCount;

    return info;
}

extern "C" GAME_CODE_ENTRY void __cdecl RendererExecuteCommand(RendererCommand command, void* args) {
    auto renderer = GetRenderer();
    switch (command) {
    case RendererCommand::ChangeRenderResolution: {
        auto data = (ChangeRenderResolutionArgs*)args;
        ChangeRenderResolution(renderer, data->wRenderResolution, data->hRenderResolution, data->sampleCount);
    } break;
    case RendererCommand::BeginFrame: {
        auto data = (BeginFrameArgs*)args;
        Begin(renderer, data->group);
    } break;
    case RendererCommand::ShadowPass: {
        auto data = (ShadowPassArgs*)args;
        ShadowPass(renderer, data->group);
    } break;
    case RendererCommand::MainPass: {
        u32 someTestVariable = 2;
        DEBUG_OVERLAY_TRACE(someTestVariable);
        auto data = (MainPassArgs*)args;
        MainPass(renderer, data->group);
    } break;
    case RendererCommand::EndFrame: {
        End(renderer);
    } break;
    case RendererCommand::LoadResource: {
        auto data = (LoadResourceArgs*)args;
        switch (data->type) {
        case RenderResourceType::CubeTexture: {
            UploadToGPU(data->cubeTexture.texture);
        } break;
        case RenderResourceType::Mesh: {
            UploadToGPU(data->mesh.mesh);
        } break;
        case RenderResourceType::ChunkMesh: {
            UploadToGPU(data->chunkMesh.mesh, data->chunkMesh.async);
        } break;
        case RenderResourceType::Texture: {
            UploadToGPU(data->texture.texture);
        } break;
            invalid_default();
        }
    } break;
    case RendererCommand::BeginLoadResource: {
        auto data = (BeginLoadResourceArgs*)args;
        BeginGPUUpload(data->mesh);
    } break;
    case RendererCommand::EndLoadResource: {
        auto data = (EndLoadResourceArgs*)args;
        // TODO(swarzzy): Stop returning result a hacky way
        auto result = EndGPUUpload(data->mesh);
        data->result = (b32)result;
    } break;
    case RendererCommand::FreeResource: {
        auto data = (FreeResourceArgs*)args;
        switch (data->type) {
        case RenderResourceType::CubeTexture: { unreachable(); } break;
        case RenderResourceType::Mesh:
        case RenderResourceType::ChunkMesh: {
            FreeGPUBuffer((u32)data->handle);
        } break;
        case RenderResourceType::Texture: {
            FreeGPUTexture((u32)data->handle);
        } break;
            invalid_default();
        }
    } break;
    case RendererCommand::BeginLoadTexture: {
        auto data = (BeginLoadTextureArgs*)args;
        auto info = GetTextureTransferBuffer(renderer, (u32)data->bufferSize);
        data->bufferInfo = info;
    } break;
    case RendererCommand::EndLoadTexture: {
        auto data = (EndLoadTextureArgs*)args;
        CompleteTextureTransfer(&data->bufferInfo, data->result);
    } break;
    case RendererCommand::SetBlockTexture: {
        auto data = (SetBlockTextureArgs*)args;
        SetBlockTexture(renderer, data->value, data->imageBits);
    } break;
    case RendererCommand::RecompileShaders: {
        RecompileShaders(renderer);
    } break;
    case RendererCommand::Initialize: {
        auto data = (InitializeArgs*)args;
        InitializeRenderer(renderer, data->tempArena, UV2(data->wResolution, data->hResolution), data->sampleCount);
    } break;
    case RendererCommand::SetLogger: {
        auto data = (SetLoggerArgs*)args;
        GlobalLogger = data->logger;
        GlobalLoggerData = data->loggerData;
        GlobalAssertHandler = data->assertHandler;
        GlobalAssertHandlerData = data->assertHandlerData;
    } break;
    case RendererCommand::ToggleDebugOverlay: {
        auto data = (ToggleDebugOverlayArgs*)args;
        Globals::ShowDebugOverlay = data->enabled;
    } break;


    default: { log_print("[Renderer] Unknown command with code %lu was submitted", (unsigned long)command); } break;
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
    log_print("[OpenGL] Debug message (source: %s, type: %s, severity: %s): %s\n", sourceStr, typeStr, severityStr, message);
    //assert(false);
}

#include "OpenglRenderer.cpp"
#include "../Resource.cpp"
#include "OpenglShaders.cpp"
#include "../DebugOverlay.cpp"

#include "../../ext/imgui/imconfig.h"
#include "../../ext/imgui/imgui.cpp"
#include "../../ext/imgui/imgui_draw.cpp"
#include "../../ext/imgui/imgui_widgets.cpp"
#include "../../ext/imgui/imgui_demo.cpp"

// NOTE: Platform specific intrinsics implementation begins here
#if defined(PLATFORM_WINDOWS)
#include <windows.h>
#else
#error Unsupported OS
#endif
#include "../Intrinsics.cpp"
