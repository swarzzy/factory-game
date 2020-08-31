#pragma once
#include "Platform.h"
#include <windows.h>

#include "RendererAPI.h"

struct MemoryArena;
struct PlatformState;
struct Application;

typedef void (__cdecl GameUpdateAndRenderFn)(PlatformState*, GameInvoke, void** data);

struct LibraryData
{
    inline static const wchar_t* GameLibName = L"flux.dll";
    inline static const wchar_t* TempGameLibName = L"TEMP_flux.dll";

    inline static const wchar_t* RendererLibName = L"ogl_renderer.dll";
    inline static const wchar_t* TempRendererLibName = L"TEMP_ogl_renderer.dll";

    inline static constexpr u32 MaxPathLen = 256;

    GameUpdateAndRenderFn* GameUpdateAndRender;

    RendererPlatformInvokeFn* RendererPlatformInvoke;
    RendererGetInfoFn* RendererGetInfo;
    RendererExecuteCommandFn* RendererExecuteCommand;

    u64 gameLibLastChangeTime;
    HMODULE gameLibHandle;

    u64 rendererLibLastChangeTime;
    HMODULE rendererLibHandle;
};

b32 UpdateGameLib(LibraryData* lib);
void UnloadGameLib(LibraryData* lib);
b32 UpdateRendererLib(LibraryData* lib);
void UnloadRendererLib(LibraryData* lib);
