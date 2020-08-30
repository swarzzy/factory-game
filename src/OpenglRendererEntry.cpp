#include "Common.h"
#include "Platform.h"

#include "RendererAPI.h"

static LoggerFn* GlobalLogger;
static void* GlobalLoggerData;

static AssertHandlerFn* GlobalAssertHandler;
static void* GlobalAssertHandlerData;

extern "C" GAME_CODE_ENTRY RendererInfo __cdecl RendererGetInfo() {
    RendererInfo info {};
    info.state.wRenderResolution = 1;
    info.state.hRenderResolution = 2;
    info.state.sampleCount = 3;
    return info;
}

extern "C" GAME_CODE_ENTRY void __cdecl RendererExecuteCommand(RendererCommand command, void* args) {
    switch (command) {
    case RendererCommand::LibraryReload: {
        auto data = (LibraryReloadArgs*)args;
        GlobalLogger = data->globalLogger;
        GlobalLoggerData = data->globalLoggerData;
        GlobalAssertHandler = data->globalAssertHandler;
        GlobalAssertHandlerData = data->globalAssertHandlerData;
    } break;
    invalid_default();
    }

}
