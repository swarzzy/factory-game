#include "SaveAndLoad.h"
#include "Intrinsics.h"

void SaveThreadWork(void* data) {
    // TODO: Thread safe logging
    log_print("Save thread is working...\n");
}

bool SaveChunk(Chunk* chunk) {
    auto world = GetWorld();
    wchar_t nameBuffer[256];
    swprintf_s(nameBuffer, 128, L"%hs\\%ld.%ld.%ld.chunk", world->name, chunk->p.x, chunk->p.y, chunk->p.z);
    auto dataSize = Chunk::Size * Chunk::Size * Chunk::Size * sizeof(BlockValue);
    auto result = PlatformDebugWriteFile(nameBuffer, chunk->blocks, (u32)dataSize);
    return result;
}

bool TryLoadChunk(Chunk* chunk) {
    auto world = GetWorld();
    wchar_t nameBuffer[256];
    swprintf_s(nameBuffer, 128, L"%hs\\%ld.%ld.%ld.chunk", world->name, chunk->p.x, chunk->p.y, chunk->p.z);
    auto dataSize = Chunk::Size * Chunk::Size * Chunk::Size * sizeof(BlockValue);
    auto result = PlatformDebugReadFile(chunk->blocks, (u32)dataSize, nameBuffer);
    return result != 0;
}

bool SaveWorld(GameWorld* world) {
    bool result = true;
    auto pool = &world->chunkPool;
    auto tick = GetPlatform()->tickCount;
    ForEachSimChunk(pool, [&](Chunk* chunk) {
        if (chunk->lastSaveTick < chunk->lastModificationTick) {
            auto saved = SaveChunk(chunk);
            if (!saved) {
                result = false;
            } else {
                chunk->lastSaveTick = tick;
            }
        }
    });
    return result;
}
