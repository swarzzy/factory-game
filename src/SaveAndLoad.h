#pragma once

#include "Common.h"

struct Chunk;

struct EntityHeaderV1 {
    u16 headerVersion;
    u64 id;
    u32 type;
    u8 kind;
    u32 flags;

    u32 offset;
};

void SaveThreadWork(void* data);

bool SaveChunk(Chunk* chunk);
bool TryLoadChunk(Chunk* chunk);

bool SaveWorld(GameWorld* world);
