#include "Platform.h"
#include "Profile.h"

void* PlatformAllocClear(uptr size);

#include "Game.h"
#include "Memory.h"

#include <stdlib.h>

inline void AssertHandler(void* data, const char* file, const char* func, u32 line, const char* assertStr, const char* fmt, va_list* args) {
    log_print("[Assertion failed] Expression (%s) result is false\nFile: %s, function: %s, line: %d.\n", assertStr, file, func, (int)line);
    if (args) {
        GlobalLogger(GlobalLoggerData, fmt, args);
    }
    debug_break();
}

// NOTE: Platform globals
static PlatformState* _GlobalPlatform = nullptr;
static Context* _GlobalContext = nullptr;

inline GameWorld* GetWorld() { return &_GlobalContext->gameWorld; }
inline const PlatformState* GetPlatform() { return _GlobalPlatform; }
inline const InputState* GetInput() { return &_GlobalPlatform->input; }
inline void PlatformSetInputMode(InputMode mode) { _GlobalPlatform->inputMode = mode; }

// TODO: Remove this
inline Context* GetContext() { return _GlobalContext; }

#define Renderer (*(const RendererAPI*)(&_GlobalPlatform->rendererAPI))
#define Platform (*(const PlatformCalls*)(&_GlobalPlatform->functions))

LoggerFn* GlobalLogger = LogMessageAPI;
void* GlobalLoggerData;

AssertHandlerFn* GlobalAssertHandler = AssertHandler;
void* GlobalAssertHandlerData = nullptr;

bool KeyHeld(Key key) {
    return GetInput()->keys[(u32)key].pressedNow;
}

bool KeyPressed(Key key) {
    return GetInput()->keys[(u32)key].pressedNow && !GetInput()->keys[(u32)key].wasPressed;
}

bool MouseButtonHeld(MouseButton button) {
    return GetInput()->mouseButtons[(u32)button].pressedNow;
}

bool MouseButtonPressed(MouseButton button) {
    return GetInput()->mouseButtons[(u32)button].pressedNow && !GetInput()->mouseButtons[(u32)button].wasPressed;
}

#define IMGUI_DEFINE_MATH_OPERATORS
#include "../ext/imgui/imgui.h"

void* ImguiAllocWrapper(size_t size, void* _) { return Platform.Allocate((uptr)size, 0, nullptr); }
void ImguiFreeWrapper(void* ptr, void*_) { Platform.Deallocate(ptr, nullptr); }

#include "../ext/imgui/imgui_internal.h"

extern "C" GAME_CODE_ENTRY void __cdecl GameUpdateAndRender(PlatformState* platform, GameInvoke reason, void** data) {
    switch (reason) {
    case GameInvoke::Init: {
        IMGUI_CHECKVERSION();
        ImGui::SetAllocatorFunctions(ImguiAllocWrapper, ImguiFreeWrapper, nullptr);
        ImGui::SetCurrentContext(platform->imguiContext);
        _GlobalPlatform = platform;

        //platform->supportsAsyncGPUTransfer = false;

        platform->gameSpeed = 1.0f;

        // NOTE: Looks like this syntax actually incorrect and somehow leads to stack owerflow in this case!
        //*context = {};
        auto gameArena = Platform.AllocateArena(Megabytes(1024));
        auto tempArena = AllocateSubArena(gameArena, gameArena->size / 2, true);
        auto contextMemory = PushSize(gameArena, sizeof(Context));
        auto context = new(contextMemory) Context();
        *data = context;
        context->gameArena = gameArena;
        context->tempArena = tempArena;

        _GlobalContext = context;

        InitLogger(&context->logger, gameArena);
        GlobalLoggerData = &context->logger;
        InitConsole(&context->console, &context->logger, context->gameArena, context);

        log_print("[Info] Asynchronous GPU memory transfer supported: %s\n", platform->supportsAsyncGPUTransfer ? "true" : "false");

        RendererSetLogger(GlobalLogger, GlobalLoggerData, GlobalAssertHandler, GlobalAssertHandlerData);
        RendererInitialize(tempArena, UV2(GetPlatform()->windowWidth, GetPlatform()->windowHeight), 8);

        context->renderGroup = RenderGroup::Make(gameArena, Megabytes(32), 8192 * 2 * 2);

        FluxInit(context);
    } break;
    case GameInvoke::Reload: {
        auto context = (Context*)(*data);
        GlobalLoggerData = &context->logger;
        IMGUI_CHECKVERSION();
        ImGui::SetAllocatorFunctions(ImguiAllocWrapper, ImguiFreeWrapper, nullptr);
        ImGui::SetCurrentContext(platform->imguiContext);
        _GlobalPlatform = platform;
        _GlobalContext = context;

        RendererSetLogger(GlobalLogger, GlobalLoggerData, GlobalAssertHandler, GlobalAssertHandlerData);
        RendererRecompileShaders();

        log_print("[Info] Game was hot-reloaded\n");
        FluxReload(context);
    } break;
    case GameInvoke::Update: {
        //bool show = true;
        //ImGui::ShowDemoWindow(&show);
        FluxUpdate((Context*)(*data));
    } break;
    case GameInvoke::Render: {
        FluxRender((Context*)(*data));
    } break;
    invalid_default();
    }
}

#include "Game.cpp"
#include "Math.cpp"
#include "DebugOverlay.cpp"
#include "Camera.cpp"
#include "RenderGroup.cpp"
#include "Renderer.cpp"
#include "Resource.cpp"
//#include "Shaders.cpp"
#include "Memory.cpp"
#include "HashMap.cpp"

#include "World.cpp"
#include "MeshGenerator.cpp"
//#include "Region.cpp"
#include "WorldGen.cpp"
#include "ChunkPool.cpp"
#include "Console.cpp"
#include "ConsoleCommands.cpp"
#include "UI.cpp"
#include "Position.cpp"
#include "Inventory.cpp"
#include "Entity.cpp"
#include "entities/Pickup.cpp"
#include "entities/Player.cpp"
#include "entities/Container.cpp"
#include "entities/Pipe.cpp"
#include "entities/Belt.cpp"
#include "entities/Extractor.cpp"
#include "entities/Projectile.cpp"
#include "FlatArray.cpp"
#include "EntityInfo.cpp"
#include "DebugUI.cpp"
#include "Profile.cpp"
#include "Block.cpp"
#include "Chunk.cpp"
#include "SaveAndLoad.cpp"
#include "BinaryBlob.cpp"

// NOTE: Platform specific intrinsics implementation begins here
#if defined(PLATFORM_WINDOWS)
#include <windows.h>
#else
#error Unsupported OS
#endif
#include "Intrinsics.cpp"

const u32 GlobalDebugCallRecordsCount = __COUNTER__;
DebugCallRecord GlobalDebugCallRecords[GlobalDebugCallRecordsCount];

// TODO: API!
#include "opengl/OpenglDefs.h"

#include "../ext/imgui/imconfig.h"
#include "../ext/imgui/imgui.cpp"
#include "../ext/imgui/imgui_draw.cpp"
#include "../ext/imgui/imgui_widgets.cpp"
#include "../ext/imgui/imgui_demo.cpp"
