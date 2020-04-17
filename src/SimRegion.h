#pragma once

#include "World.h"
#include "MeshGenerator.h"

struct SimRegion {
    GameWorld* world;
    iv3 origin; // chunk pos
    u32 span;
    iv3 min;
    iv3 max;
    u32 maxChunkCount;
    u32 chunkCount;
    Chunk* firstChunk;
    byte* chunkMeshPoolUsage;
    ChunkMesh* chunkMeshPool;
};

void DrawRegion(SimRegion* region, RenderGroup* renderGroup, Camera* camera);
void ResizeRegion(SimRegion* region, u32 newSpan, MemoryArena* arena);
void MoveRegion(SimRegion* region, iv3 newP);
void RegionUpdateChunkStates(SimRegion* region);
