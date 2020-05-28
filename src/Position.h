#pragma once

#include "Common.h"
#include "Math.h"

struct ChunkPos;

struct WorldPos {
    iv3 block;
    v3 offset;

    static inline WorldPos Make(iv3 voxel) { return WorldPos{voxel, V3(0.0f)}; }
    static inline WorldPos Make(i32 x, i32 y, i32 z) { return WorldPos{IV3(x, y, z), V3(0.0f)}; }
    static inline WorldPos Make(iv3 voxel, v3 offset) { return WorldPos{voxel, offset}; }

    static WorldPos Normalize(WorldPos p);
    static WorldPos Offset(WorldPos p, v3 offset);
    static v3 Difference(WorldPos a, WorldPos b);
    static v3 Relative(WorldPos origin, WorldPos target);
    static iv3 ChunkCoord(iv3 block);
    static uv3 BlockInChunk(iv3 block);
    static ChunkPos ToChunk(iv3 block);
    static ChunkPos ToChunk(WorldPos p);

};

struct ChunkPos {
    iv3 chunk;
    uv3 block;

    static inline ChunkPos Make(iv3 chunk, uv3 voxel) { return ChunkPos{chunk, voxel}; }
    static WorldPos ChunkPos::ToWorld(ChunkPos p);
};
