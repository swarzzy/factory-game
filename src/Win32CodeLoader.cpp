#include "Win32CodeLoader.h"
#include <stdlib.h>

static void __cdecl GameUpdateAndRenderDummy(PlatformState*, GameInvoke, void**)
{

}

void UnloadGameCode(LibraryData* lib)
{
    FreeLibrary(lib->handle);
    lib->GameUpdateAndRender = GameUpdateAndRenderDummy;
    DeleteFile(LibraryData::TempDllName);
}

b32 UpdateGameCode(LibraryData* lib)
{
    b32 updated = false;
    WIN32_FIND_DATA findData;
    HANDLE findHandle = FindFirstFile(LibraryData::DllName, &findData);
    if (findHandle != INVALID_HANDLE_VALUE)
    {
        FindClose(findHandle);
        FILETIME fileTime = findData.ftLastWriteTime;
        u64 writeTime = ((u64)0 | fileTime.dwLowDateTime) | ((u64)0 | fileTime.dwHighDateTime) << 32;
        if (writeTime != lib->lastChangeTime)
        {
            UnloadGameCode(lib);

            auto result = CopyFile(LibraryData::DllName, LibraryData::TempDllName, FALSE);
            if (result)
            {
                lib->handle = LoadLibrary(LibraryData::TempDllName);
                if (lib->handle)
                {
                    GameUpdateAndRenderFn* gameUpdateAndRender = (GameUpdateAndRenderFn*)GetProcAddress(lib->handle, "GameUpdateAndRender");
                    if (gameUpdateAndRender)
                    {
                        lib->GameUpdateAndRender = gameUpdateAndRender;
                        updated = true;
                        lib->lastChangeTime = writeTime;
                    }
                    else
                    {
                        log_print("[Error] Failed to get GameUpdateAndRender() address.\n");
                    }
                }
                else
                {
                    log_print("[Error] Failed to load game library.\n");
                }
            }
            else
            {
                //AB_CORE_INFO("Waiting for game code loading.");
            }
        }
    }
    else
    {
        //AB_CORE_ERROR("Game code not found");
    }
    return updated;
}
