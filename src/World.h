#pragma once

#include "Common.h"
#include "HashMap.h"
#include "BucketArray.h"
#include "WorldGen.h"
#include "Memory.h"
#include "Position.h"
#include "Entity.h"
#include "ChunkPool.h"

struct ChunkMesh;
struct ChunkMesher;
struct Camera;
struct RenderGroup;
struct Mesh;
struct Material;
struct Context;

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

u32 EntityRegionHashFunc(void* value) {
    EntityID* p = (EntityID*)value;
    // TODO: Reasonable hashing
    u32 hash = (u32)(*p);
    return hash;
}

bool EntityRegionHashCompFunc(void* a, void* b) {
    EntityID* p1 = (EntityID*)a;
    EntityID* p2 = (EntityID*)b;
    bool result = (*p1 == *p2);
    return result;
}

struct GameWorld {
    static const i32 MinHeight = -(i32)Chunk::Size * 3;
    static const i32 MaxHeight = (i32)Chunk::Size * 3 - 1;
    static const i32 MinHeightChunk = -3;
    static const i32 MaxHeightChunk = 2;
    static const i32 InvalidCoord = I32::Max;
    inline static const iv3 InvalidPos = IV3(InvalidCoord);
    EntityID playerID;
    HashMap<iv3, Chunk*, ChunkHashFunc, ChunkHashCompFunc> chunkHashMap;
    HashMap<EntityID, Entity*, EntityRegionHashFunc, EntityRegionHashCompFunc> entityHashMap;
    // TODO: Dynamic view distance
    static const u32 ViewDistance = 4;
    Camera* camera;
    //Chunk* firstActive;

    u64 entitySerialCount;

    // Be carefull with pointers
    // Be carefull with pointers
    BucketArray<Entity*, 16> entitiesToDelete;
    // TODO: Is nullBlock actually good idea?
    Block nullBlock;
    ChunkPool chunkPool;
};

void InitWorld(GameWorld* world, Context* context, ChunkMesher* mesher, u32 seed);

const Block* GetBlock(GameWorld* world, i32 x, i32 y, i32 z);
inline const Block* GetBlock(GameWorld* world, iv3 p) { return GetBlock(world, p.x, p.y, p.z); }

Chunk* AddChunk(GameWorld* world, iv3 coord);
void DeleteChunk(GameWorld* world, Chunk* chunk);

// NOTE: Returns chunk if it exist
Chunk* GetChunkInternal(GameWorld* world, i32 x, i32 y, i32 z);
// NOTE: Returns chunk if it exist and filled
Chunk* GetChunk(GameWorld* world, i32 x, i32 y, i32 z);
inline Chunk* GetChunk(GameWorld* world, iv3 chunkP) { return GetChunk(world, chunkP.x, chunkP.y, chunkP.z); }

bool CheckWorldBounds(WorldPos p);
bool CheckWorldBounds(ChunkPos p);

template <typename T>
T* AddSpatialEntity(GameWorld* world, WorldPos p);

template <typename T>
T* AddBlockEntity(GameWorld* world, iv3 p);

void DeleteEntity(GameWorld* world, Entity* entity);
void ScheduleEntityForDelete(GameWorld* world, Entity* entity);

void PostEntityNeighborhoodUpdate(GameWorld* world, BlockEntity* entity);

void MoveSpatialEntity(GameWorld* world, SpatialEntity* entity, v3 delta, Camera* camera, RenderGroup* renderGroup);

bool UpdateEntityResidence(GameWorld* world, SpatialEntity* entity);

void ConvertBlockToPickup(GameWorld* world, iv3 voxelP);

void FindOverlapsFor(GameWorld* world, SpatialEntity* entity);

bool SetBlockEntityPos(GameWorld* world, BlockEntity* entity, iv3 newP);

bool BuildBlock(Context* context, GameWorld* world, iv3 p, Item item);

Entity* GetEntity(GameWorld* world, iv3 p);
Entity* GetEntity(GameWorld* world, EntityID id);

void RegisterEntity(GameWorld* world, Entity* entity);
bool UnregisterEntity(GameWorld* world, EntityID id);
