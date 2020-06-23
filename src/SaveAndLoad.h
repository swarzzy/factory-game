#pragma once

#include "Common.h"

struct Chunk;

struct EntityHeaderV1 {
    u16 headerVersion;
    u64 id;
    u32 type;
    u8 kind;
    u32 flags;

    union {
        struct  {
            iv3 pBlock;
            v3 pOffset;
            v3 velocity;
            f32 scale;
            f32 acceleration;
            f32 friction;
        } spatial;
        struct {
            iv3 p;
        } block;
    };

    u32 offset;
};

void SaveThreadWork(void* data);

bool SaveChunk(Chunk* chunk);
bool TryLoadChunk(Chunk* chunk);
void TryLoadEntities(Chunk* chunk);

bool SaveWorld(GameWorld* world);
