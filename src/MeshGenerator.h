#pragma once

#include "Math.h"
#include "World.h"

struct Chunk;

struct ChunkMeshBlock {
    static const u32 Size = 4096;
    ChunkMeshBlock* next;
    ChunkMeshBlock* prev;
    u32 vertexCount;
    v3 vertices[Size];
    v3 normals[Size];
    v3 tangents[Size];
    u16 values[Size];
};

struct ChunkMesher;

struct ChunkMesh {
    static const u32 VertexSize = sizeof(v3) + sizeof(v3) + sizeof(v3) + sizeof(u16);

    ChunkMeshBlock* begin;
    ChunkMeshBlock* end;
    u32 vertexCount;
    u32 gpuHandle;
    u64 gpuLock;
    void* gpuBufferPtr;
    ChunkMesher* mesher;
    // TODO: For debug
    b32 gpuMemoryMapped;
    Chunk* chunk;
};

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
