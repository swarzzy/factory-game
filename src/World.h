#pragma once

#include "flux_hash_map.h"
#include "Region.h"
#include "WorldGen.h"

struct ChunkMesh;
struct ChunkMesher;

struct WorldPos {
    iv3 voxel;
    v3 offset;
    inline static WorldPos Make(iv3 voxel) { return WorldPos{voxel, V3(0.0f)}; }
};

struct ChunkPos {
    iv3 chunk;
    uv3 voxel;
    inline static ChunkPos Make(WorldPos p) {}
};

enum struct VoxelValue : u32 {
    Empty = 0,
    Stone,
    Grass
};

const f32 MeterScale = 1.0f;

struct Voxel {
    inline static const f32 Dim = 1.0f * MeterScale;
    inline static const f32 HalfDim = Dim * 0.5f;
    VoxelValue value = VoxelValue::Empty;
};

struct Chunk {
    static const u32 BitShift = 5;
    static const u32 BitMask = (1 << BitShift) - 1;
    static const u32 Size = 1 << BitShift;

    b32 modified;
    iv3 p;
    ChunkMesh* mesh;
    Voxel voxels[Size * Size * Size];
};

// TODO: We need more complicated structure of chunk hash table
// which is able to handle very large number of entries

u32 ChunkHashFunc(void* value) {
    iv3* p = (iv3*)value;
    // TODO: Reasonable hashing
    u32 hash = p->x * 12342 + p->y * 23423 + p->z * 13;
    return hash;
}

bool ChunkHashCompFunc(void* a, void* b) {
    iv3* p1 = (iv3*)a;
    iv3* p2 = (iv3*)b;
    bool result = (p1->x == p2->x) && (p1->y == p2->y) && (p1->z == p2->z);
    return result;
}

struct GameWorld {
    HashMap<iv3, Chunk*, ChunkHashFunc, ChunkHashCompFunc> chunkHashMap;
    // TODO: Dynamic view distance
    static const u32 ViewDistance = 4;
    WorldGen worldGen;
    ChunkMesher* mesher;

    void Init(ChunkMesher* mesher, u32 seed) {
        this->chunkHashMap = HashMap<iv3, Chunk*, ChunkHashFunc, ChunkHashCompFunc>::Make();
        this->mesher = mesher;
        this->worldGen.Init(seed);
    }
};

Voxel* GetVoxelRaw(Chunk* chunk, u32 x, u32 y, u32 z);
Voxel* GetVoxel(Chunk* chunk, u32 x, u32 y, u32 z);

void DebugFillChunk(Chunk* chunk);

Chunk* AddChunk(GameWorld* world, iv3 coord);

Chunk* GetChunk(GameWorld* world, i32 x, i32 y, i32 z);

WorldPos NormalizeWorldPos(WorldPos p);

WorldPos Offset(WorldPos p, v3 offset);

v3 RelativePos(WorldPos origin, WorldPos target);

iv3 GetChunkCoord(i32 x, i32 y, i32 z);

uv3 GetVoxelCoordInChunk(i32 x, i32 y, i32 z);

ChunkPos ChunkPosFromWorldPos(iv3 tile);
WorldPos WorldPosFromChunkPos(ChunkPos p);
