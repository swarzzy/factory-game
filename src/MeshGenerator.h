#pragma once

#include "flux_math.h"
#include "World.h"

struct Chunk;

struct ChunkMeshBlock {
    static const u32 Size = 1024;
    ChunkMeshBlock* next;
    ChunkMeshBlock* prev;
    u32 vertexCount;
    v3 vertices[Size];
    v3 normals[Size];
    v3 tangents[Size];
    u16 values[Size];
};

struct ChunkMesh {
    static const u32 VertexSize = sizeof(v3) + sizeof(v3) + sizeof(v3) + sizeof(u16);
    ChunkMeshBlock* begin;
    ChunkMeshBlock* end;
    u32 vertexCount;
    u32 gpuHandle;
};

struct ChunkMesher {
    u32 totalBlockCount;
    u32 freeBlockCount;
    ChunkMeshBlock* freeBlockList;
};

void GenMesh(ChunkMesher* mesher, Chunk* chunk);

void FreeChunkMesh(ChunkMesher* mesher, ChunkMesh* mesh);
