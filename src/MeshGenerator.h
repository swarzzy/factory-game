#pragma once

#include "Math.h"
#include "World.h"

struct Chunk;

struct ChunkMesher {
    u32 totalBlockCount;
    u32 freeBlockCount;
    ChunkMeshBlock* freeBlockList;
    volatile u32 freeListLock;
};

void GenMesh(ChunkMesher* mesher, Chunk* chunk);
void FreeChunkMesh(ChunkMesher* mesher, ChunkMesh* mesh);

bool ScheduleChunkMeshing(GameWorld* world, Chunk* chunk);
void ScheduleChunkMeshUpload(Chunk* chunk);
void CompleteChunkMeshUpload(Chunk* chunk);
