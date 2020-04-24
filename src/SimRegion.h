#pragma once

#include "World.h"
#include "MeshGenerator.h"

u32 SimRegionHashFunc(void* value) {
    EntityID* p = (EntityID*)value;
    // TODO: Reasonable hashing
    u32 hash = (u32)p->id;
    return hash;
}

bool SimRegionHashCompFunc(void* a, void* b) {
    EntityID* p1 = (EntityID*)a;
    EntityID* p2 = (EntityID*)b;
    bool result = (p1->id == p2->id);
    return result;
}

struct SimRegion {
    GameWorld* world;
    iv3 origin; // chunk pos
    u32 span;
    iv3 min;
    iv3 max;
    u32 maxChunkCount;
    u32 chunkCount;
    Chunk* firstChunk;
    u32 chunkMeshPoolFree;
    byte* chunkMeshPoolUsage;
    ChunkMesh* chunkMeshPool;
    b32 hasPendingRemeshesAfterEdit;
    HashMap<EntityID, SpatialEntity*, SimRegionHashFunc, SimRegionHashCompFunc> spatialEntityTable;
};

void InitRegion(SimRegion* region);
void DrawRegion(SimRegion* region, RenderGroup* renderGroup, Camera* camera);
void ResizeRegion(SimRegion* region, u32 newSpan, MemoryArena* arena);
void MoveRegion(SimRegion* region, iv3 newP);
void RegionUpdateChunkStates(SimRegion* region);

void RegisterSpatialEntity(SimRegion* region, SpatialEntity* entity);
bool UnregisterSpatialEntity(SimRegion* region, EntityID id);

struct Context;
void UpdateEntities(SimRegion* region, RenderGroup* renderGroup, Camera* camera, Context* context);
