#include "Win32CodeLoader.h"
#include <stdlib.h>

static void __cdecl GameUpdateAndRenderDummy(PlatformState*, GameInvoke, void**)
{

}

static RendererInfo __cdecl RendererGetInfoDummy()
{
    return {};
}

static void __cdecl RendererExecuteCommandDummy(RendererCommand command, void* args)
{
}

void UnloadGameLib(LibraryData* lib)
{
    FreeLibrary(lib->gameLibHandle);
    lib->GameUpdateAndRender = GameUpdateAndRenderDummy;
    DeleteFile(LibraryData::TempGameLibName);
}

void UnloadRendererLib(LibraryData* lib)
{
    FreeLibrary(lib->rendererLibHandle);
    lib->RendererGetInfo = RendererGetInfoDummy;
    lib->RendererExecuteCommand = RendererExecuteCommandDummy;
    DeleteFile(LibraryData::TempRendererLibName);
}

struct GetFileWriteTimeResult {
    u64 time;
    b32 found;
};

GetFileWriteTimeResult GetFileWriteTime(const wchar_t* filename) {
    GetFileWriteTimeResult result {};

    WIN32_FIND_DATA findData;
    HANDLE findHandle = FindFirstFile(filename, &findData);
    if (findHandle != INVALID_HANDLE_VALUE)
    {
        FindClose(findHandle);
        FILETIME fileTime = findData.ftLastWriteTime;
        u64 writeTime = ((u64)0 | fileTime.dwLowDateTime) | ((u64)0 | fileTime.dwHighDateTime) << 32;
        result.found = true;
        result.time = writeTime;
    }
    return result;
}


b32 UpdateGameLib(LibraryData* lib)
{
    b32 updated = false;

    auto fileTime = GetFileWriteTime(LibraryData::GameLibName);
    if (fileTime.found && fileTime.time != lib->gameLibLastChangeTime) {
        UnloadGameLib(lib);
        auto result = CopyFile(LibraryData::GameLibName, LibraryData::TempGameLibName, FALSE);
        if (result) {
            lib->gameLibHandle = LoadLibrary(LibraryData::TempGameLibName);
            if (lib->gameLibHandle) {
                GameUpdateAndRenderFn* gameUpdateAndRender = (GameUpdateAndRenderFn*)GetProcAddress(lib->gameLibHandle, "GameUpdateAndRender");
                if (gameUpdateAndRender) {
                    lib->GameUpdateAndRender = gameUpdateAndRender;
                    updated = true;
                    lib->gameLibLastChangeTime = fileTime.time;
                } else {
                    log_print("[Error] Failed to get GameUpdateAndRender() address.\n");
                }
            } else {
                log_print("[Error] Failed to load game library.\n");
            }
        } else {
        }
    }
    return updated;
}

b32 UpdateRendererLib(LibraryData* lib)
{
    b32 updated = false;

    auto fileTime = GetFileWriteTime(LibraryData::RendererLibName);
    if (fileTime.found && fileTime.time != lib->rendererLibLastChangeTime) {
        UnloadRendererLib(lib);
        auto result = CopyFile(LibraryData::RendererLibName, LibraryData::TempRendererLibName, FALSE);
        if (result) {
            lib->rendererLibHandle = LoadLibrary(LibraryData::TempRendererLibName);
            if (lib->rendererLibHandle) {
                auto rendererGetInfo = (RendererGetInfoFn*)GetProcAddress(lib->rendererLibHandle, "RendererGetInfo");
                auto rendererExecuteCommand = (RendererExecuteCommandFn*)GetProcAddress(lib->rendererLibHandle, "RendererExecuteCommand");

                if (rendererGetInfo && rendererExecuteCommand) {
                    lib->RendererGetInfo = rendererGetInfo;
                    lib->RendererExecuteCommand = rendererExecuteCommand;
                    updated = true;
                    lib->rendererLibLastChangeTime = fileTime.time;
                } else {
                    log_print("[Error] Failed to get renderer library functions addresses.\n");
                }
            } else {
                log_print("[Error] Failed to load renderer library.\n");
            }
        } else {
        }
    }
    return updated;
}
