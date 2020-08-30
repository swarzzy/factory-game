#pragma once

#include "World.h"
#include "MeshGenerator.h"
#include "Chunk.h"

struct SimRegion {
    iv3 origin; // chunk pos
    u32 span;
    iv3 min;
    iv3 max;
    ChunkPool* pool;
};

void MoveRegion(SimRegion* region, iv3 newP);

struct ChunkPool {
    SimRegion playerRegion;
    GameWorld* world;
    WorldGen worldGen;
    ChunkMesher* mesher;

    u32 maxRenderedChunkCount;
    u32 renderedChunkCount;
    Chunk* firstRenderedChunk;

    u32 maxSimChunkCount;
    u32 simChunkCount;
    Chunk* firstSimChunk;

    Chunk* simChunkEvictList;
    volatile u32 pendingSavesCount;

    u32 chunkMeshPoolFree;
    byte* chunkMeshPoolUsage;
    ChunkMesh* chunkMeshPool;
    b32 hasPendingRemeshesAfterEdit;
};

void InitChunkPool(ChunkPool* pool, GameWorld* world, ChunkMesher* mesher, u32 newSpan, u32 seed);

void DrawChunks(ChunkPool* pool, RenderGroup* renderGroup, Camera* camera);
void UpdateChunkEntities(ChunkPool* pool, RenderGroup* renderGroup, Camera* camera);
void UpdateChunks(ChunkPool* region, Renderer* renderer);

template <typename F>
void ForEachEntity(ChunkPool* pool, F func);

template <typename F>
void ForEachSimChunk(ChunkPool* pool, F func);
