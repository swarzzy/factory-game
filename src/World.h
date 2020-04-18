#pragma once

#include "HashMap.h"
#include "WorldGen.h"

struct ChunkMesh;
struct ChunkMesher;
struct Camera;
struct RenderGroup;

struct WorldPos {
    iv3 voxel;
    v3 offset;
    inline static WorldPos Make(iv3 voxel) { return WorldPos{voxel, V3(0.0f)}; }
    inline static WorldPos Make(i32 x, i32 y, i32 z) { return WorldPos{IV3(x, y, z), V3(0.0f)}; }
    inline static WorldPos Make(iv3 voxel, v3 offset) { return WorldPos{voxel, offset}; }
};

struct ChunkPos {
    iv3 chunk;
    uv3 voxel;
    inline static ChunkPos Make(iv3 chunk, uv3 voxel) { return ChunkPos{chunk, voxel}; }
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

enum struct ChunkState : u32 {
    Complete = 0, Filling, Filled, Meshing, MeshingFinished, WaitsForUpload, MeshUploadingFinished, UploadingMesh
};

enum struct ChunkPriority : u32 {
    Low = 0, High
};

struct Chunk {
    static const u32 BitShift = 5;
    static const u32 BitMask = (1 << BitShift) - 1;
    static const u32 Size = 1 << BitShift;

    volatile ChunkState state;
    b32 locked;
    b32 filled;
    b32 primaryMeshValid;
    b32 secondaryMeshValid;
    b32 remeshingAfterEdit;
    ChunkPriority priority;
    b32 shouldBeRemeshedAfterEdit;

    b32 modified;
    b32 active;
    Chunk* nextActive;
    Chunk* prevActive;
    iv3 p;
    ChunkMesh* primaryMesh;
    ChunkMesh* secondaryMesh;
    u32 primaryMeshPoolIndex;
    u32 secondaryMeshPoolIndex;
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

enum struct SpatialEntityType : u32 {
    Player
};

struct SpatialEntity {
    SpatialEntityType type;
    WorldPos p;
    v3 velocity;
    f32 acceleration;
    f32 friction;
    b32 grounded;
};

struct Player {
    SpatialEntity* entity;
    b32 flightMode;
    f32 height;
    iv3 selectedVoxel;
    f32 jumpAcceleration;
    f32 runAcceleration;
};

struct GameWorld {
    static const i32 MinHeight = -(i32)Chunk::Size * 3;
    static const i32 MaxHeight = (i32)Chunk::Size * 3 - 1;
    static const i32 MinHeightChunk = -3;
    static const i32 MaxHeightChunk = 2;
    static const i32 InvalidCoord = I32::Max;
    inline static const iv3 InvalidPos = IV3(InvalidCoord);
    SpatialEntity playerEntity;
    Player player;
    HashMap<iv3, Chunk*, ChunkHashFunc, ChunkHashCompFunc> chunkHashMap;
    // TODO: Dynamic view distance
    static const u32 ViewDistance = 4;
    WorldGen worldGen;
    ChunkMesher* mesher;
    Chunk* firstActive;

    void Init(ChunkMesher* mesher, u32 seed) {
        this->chunkHashMap = HashMap<iv3, Chunk*, ChunkHashFunc, ChunkHashCompFunc>::Make();
        this->mesher = mesher;
        this->worldGen.Init(seed);
        this->player.entity = &this->playerEntity;
        this->playerEntity.type = SpatialEntityType::Player;
        this->playerEntity.p = WorldPos::Make(IV3(0, 15, 0));
        this->playerEntity.acceleration = 70.0f;
        this->playerEntity.friction = 10.0f;
        this->player.height = 1.8f;
        this->player.selectedVoxel = InvalidPos;
        this->player.jumpAcceleration = 420.0f;
        this->player.runAcceleration = 140.0f;
    }
};

Voxel* GetVoxelRaw(Chunk* chunk, u32 x, u32 y, u32 z);
const Voxel* GetVoxel(Chunk* chunk, u32 x, u32 y, u32 z);
const Voxel* GetVoxel(GameWorld* world, i32 x, i32 y, i32 z);
Voxel* GetVoxelForModification(Chunk* chunk, u32 x, u32 y, u32 z);

Chunk* AddChunk(GameWorld* world, iv3 coord);

Chunk* GetChunk(GameWorld* world, i32 x, i32 y, i32 z);

WorldPos NormalizeWorldPos(WorldPos p);

WorldPos Offset(WorldPos p, v3 offset);

v3 Difference(WorldPos a, WorldPos b);
v3 RelativePos(WorldPos origin, WorldPos target);

iv3 GetChunkCoord(i32 x, i32 y, i32 z);

uv3 GetVoxelCoordInChunk(i32 x, i32 y, i32 z);

ChunkPos ChunkPosFromWorldPos(iv3 tile);
WorldPos WorldPosFromChunkPos(ChunkPos p);

WorldPos DoMovement(GameWorld* world, WorldPos origin, v3 delta, v3* velocity, bool* hitGround, Camera* camera, RenderGroup* renderGroup);
