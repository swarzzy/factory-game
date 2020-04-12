#pragma once
#include "Platform.h"
#include <windows.h>

struct MemoryArena;
struct PlatformState;
struct Application;

typedef void (__cdecl GameUpdateAndRenderFn)(PlatformState*, GameInvoke, void** data);

struct LibraryData
{
    inline static const wchar_t* DllName = L"flux.dll";
    inline static const wchar_t* TempDllName = L"TEMP_flux.dll";
    inline static constexpr u32 MaxPathLen = 256;
    GameUpdateAndRenderFn* GameUpdateAndRender;
    u64 lastChangeTime;
    HMODULE handle;
};

b32 UpdateGameCode(LibraryData* lib);
void UnloadGameCode(LibraryData* lib);
