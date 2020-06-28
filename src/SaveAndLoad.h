#pragma once

#include "Common.h"

struct Chunk;
struct GameWorld;

struct WorldFile {
    constant u32 MagicValue = 0xcabccabc;
    constant u32 LatestVersion = 1;
    u32 magic;
    u32 version;
    u64 entitySerialCount;
};

struct EntityFileHeader {
    constant u32 MagicValue = 0xabf537ac;
    constant u32 LatestVersion = 1;
    u32 magic;
    u32 version;
    u32 entityCount;
    u32 _reserved;
};

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

    u32 dataOffset;
    u32 dataSize;
};

void SaveThreadWork(void* data);

bool SaveChunk(Chunk* chunk);
bool TryLoadChunk(Chunk* chunk);
void TryLoadEntities(Chunk* chunk);

bool SaveWorldData(GameWorld* world);
bool LoadWorldData(GameWorld* world);

bool SaveWorld(GameWorld* world);
