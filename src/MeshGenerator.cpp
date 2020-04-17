#include "MeshGenerator.h"

#include "Intrinsics.h"

bool IsVoxelOccluder(Chunk* chunk, i32 x, i32 y, i32 z) {
    bool occluder = false;
    // NOTE: Assuming blocks on chunk edges always visible
    if (x < Chunk::Size && y < Chunk::Size && z < Chunk::Size &&
        x >= 0 && y >= 0 && z >= 0) {
        auto voxel = GetVoxelRaw(chunk, (u32)x, (u32)y, (u32)z);
        occluder = voxel->value != VoxelValue::Empty;
    }
    return occluder;
}

void ChunkMesherLock(ChunkMesher* mesher) {
    while (true) {
        if (AtomicCompareExchange(&mesher->freeListLock, 0, 1) == 0) {
            break;
        }
    }
}

void ChunkMesherUnlock(ChunkMesher* mesher) {
    WriteFence();
    mesher->freeListLock = 0;
}

ChunkMeshBlock* GetChunkMeshBlock(ChunkMesher* mesher) {
    ChunkMeshBlock* block;
    ChunkMesherLock(mesher);
    if (mesher->freeBlockCount) {
        assert(mesher->freeBlockList);
        block = mesher->freeBlockList;
        mesher->freeBlockList = block->next;
        mesher->freeBlockCount--;
    } else {
        block = (ChunkMeshBlock*)PlatformAlloc(sizeof(ChunkMeshBlock), 0, nullptr);
        mesher->totalBlockCount++;
    }
    ChunkMesherUnlock(mesher);

    block->next = nullptr;
    block->prev = nullptr;
    block->vertexCount = 0;
    return block;
}

void FreeChunkMeshBlock(ChunkMesher* mesher, ChunkMeshBlock* block) {
    ChunkMesherLock(mesher);
    block->next = mesher->freeBlockList;
    mesher->freeBlockList = block;
    mesher->freeBlockCount++;
    ChunkMesherUnlock(mesher);
}

void FreeChunkMesh(ChunkMesher* mesher, ChunkMesh* mesh) {
    auto block = mesh->end;
    while (block) {
        auto nextBlock = block->next;
        FreeChunkMeshBlock(mesher, block);
        block = nextBlock;
    }
    mesh->begin = nullptr;
    mesh->end = nullptr;
    mesh->vertexCount = 0;
}

void PushVertex(ChunkMesher* mesher, ChunkMesh* mesh, v3 v, v3 n, v3 t, u16 terrainIndex) {
    if (!mesh->begin) {
        auto newBlock = GetChunkMeshBlock(mesher);
        mesh->begin = newBlock;
        mesh->end = newBlock;
    }
    auto block = mesh->begin;
    assert(block->vertexCount <= ChunkMeshBlock::Size);
    if (block->vertexCount == ChunkMeshBlock::Size) {
        auto newBlock = GetChunkMeshBlock(mesher);
        mesh->begin = newBlock;
        block->next = newBlock;
        newBlock->prev = block;
        block = newBlock;
    }

    auto at = block->vertexCount++;
    block->vertices[at] = v;
    block->normals[at] = n;
    block->tangents[at] = t;
    block->values[at] = (u16)terrainIndex;
    mesh->vertexCount++;
}

void PushQuad(ChunkMesher* mesher, ChunkMesh* mesh, v3 vt0, v3 vt1, v3 vt2, v3 vt3, VoxelValue value) {
    v3 n = Cross(vt2 - vt1, vt0 - vt1);
    v3 t = vt1 - vt0;
    u16 terrainIndex = VoxelValueToTerrainIndex(value);
    PushVertex(mesher, mesh, vt0, n, t, terrainIndex);
    PushVertex(mesher, mesh, vt1, n, t, terrainIndex);
    PushVertex(mesher, mesh, vt2, n, t, terrainIndex);
    PushVertex(mesher, mesh, vt3, n, t, terrainIndex);
}

void GenMesh(ChunkMesher* mesher, Chunk* chunk) {
    assert(chunk->primaryMesh);
    ChunkMesh* mesh = chunk->primaryMesh;

    for (u32 z = 0; z < Chunk::Size; z++) {
        for (u32 y = 0; y < Chunk::Size; y++) {
            for (u32 x = 0; x < Chunk::Size; x++) {
                auto voxel = GetVoxelRaw(chunk, x, y, z);
                if (voxel->value != VoxelValue::Empty) {
                    auto value = voxel->value;
                    if (chunk->p.x == 1 && chunk->p.y == -1 && chunk->p.z == 1 && x == 0 && y == 0 && z == (Chunk::Size - 1))
                    {
                        int r = 5;

                    }

                    bool up = IsVoxelOccluder(chunk, (i32)x, ((i32)y) + 1, (i32)z);
                    bool down = IsVoxelOccluder(chunk, (i32)x, ((i32)y) - 1, (i32)z);
                    bool left = IsVoxelOccluder(chunk, ((i32)x) - 1, (i32)y, (i32)z);
                    bool right = IsVoxelOccluder(chunk, ((i32)x) + 1, ((i32)y), (i32)z);
                    bool front = IsVoxelOccluder(chunk, (i32)x, (i32)y, ((i32)z) + 1);
                    bool back = IsVoxelOccluder(chunk, (i32)x, (i32)y, ((i32)z) - 1);

                    v3 offset = V3(x, y, z) * Voxel::Dim;
                    v3 min = offset - V3(Voxel::HalfDim, Voxel::HalfDim, Voxel::HalfDim);
                    v3 max = offset + V3(Voxel::HalfDim, Voxel::HalfDim, Voxel::HalfDim);

                    v3 vt0 = V3(min.x, min.y, max.z);
                    v3 vt1 = V3(max.x, min.y, max.z);
                    v3 vt2 = V3(max.x, max.y, max.z);
                    v3 vt3 = V3(min.x, max.y, max.z);

                    v3 vt4 = V3(max.x, min.y, min.z);
                    v3 vt5 = V3(max.x, max.y, min.z);
                    v3 vt6 = V3(min.x, min.y, min.z);
                    v3 vt7 = V3(min.x, max.y, min.z);

                    if (!up) PushQuad(mesher, mesh, vt3, vt2, vt5, vt7, value);
                    if (!down) PushQuad(mesher, mesh, vt6, vt4, vt1, vt0, value);
                    if (!left) PushQuad(mesher, mesh, vt6, vt0, vt3, vt7, value);
                    if (!right) PushQuad(mesher, mesh, vt1, vt4, vt5, vt2, value);
                    if (!front) PushQuad(mesher, mesh, vt0, vt1, vt2, vt3, value);
                    if (!back) PushQuad(mesher, mesh, vt4, vt6, vt7, vt5, value);
                }
            }
        }
    }
}



void ChunkMesherWork(void* data0, void* data1, void* data2, u32 threadID) {
    auto chunk = (Chunk*)data0;
    auto mesh = chunk->primaryMesh;
    GenMesh(mesh->mesher, chunk);
    if (GlobalPlatform.supportsAsyncGPUTransfer) {
        if (chunk->mesh->vertexCount) {
            auto uploaded = UploadToGPU(chunk->mesh, false);
            assert(uploaded);
        }
        auto prevState = AtomicExchange((volatile u32*)&chunk->state, (u32)ChunkState::MeshingFinished);
        assert(prevState == (u32)ChunkState::Meshing);
    } else {
        auto prevState = AtomicExchange((volatile u32*)&chunk->state, (u32)ChunkState::WaitsForUpload);
        assert(prevState == (u32)ChunkState::Meshing);
    }
}

bool ScheduleChunkMeshing(GameWorld* world, Chunk* chunk) {
    assert(chunk->primaryMesh);
    assert(chunk->state == ChunkState::Complete);
    bool result = true;
    chunk->state = ChunkState::Meshing;
    WriteFence();
    if (!PlatformPushWork(GlobalLowPriorityWorkQueue, ChunkMesherWork, chunk, nullptr, nullptr)) {
        chunk->state = ChunkState::Complete;
        result = false;
    }
    return result;
}

void UploadChunkMeshToGPUWork(void* data0, void* data1, void* data2, u32 threadID) {
    auto chunk = (Chunk*)data0;
    auto mesh = chunk->primaryMesh;
    void* ptr = mesh->gpuBufferPtr;
    assert(ptr);
    uptr size = mesh->vertexCount * ChunkMesh::VertexSize;
    uptr offset = 0;

    auto block = mesh->end;
    while (block) {
        memcpy((byte*)ptr + offset, block->vertices, sizeof(block->vertices[0]) * block->vertexCount);
        offset += sizeof(block->vertices[0]) * block->vertexCount;
        block = block->next;
    }

    block = mesh->end;
    while (block) {
        memcpy((byte*)ptr + offset, block->normals, sizeof(block->normals[0]) * block->vertexCount);
        offset += sizeof(block->normals[0]) * block->vertexCount;
        block = block->next;
    }

    block = mesh->end;
    while (block) {
        memcpy((byte*)ptr + offset, block->tangents, sizeof(block->tangents[0]) * block->vertexCount);
        offset += sizeof(block->tangents[0]) * block->vertexCount;
        block = block->next;
    }

    block = mesh->end;
    while (block) {
        memcpy((byte*)ptr + offset, block->values, sizeof(block->values[0]) * block->vertexCount);
        offset += sizeof(block->values[0]) * block->vertexCount;
        block = block->next;
    }

    assert(offset == size);
    WriteFence();
    auto prevState = AtomicExchange((volatile u32*)&chunk->state, (u32)ChunkState::MeshUploadingFinished);
    assert(prevState == (u32)ChunkState::UploadingMesh);
}

void ScheduleChunkMeshUpload(Chunk* chunk) {
    assert(chunk->state == ChunkState::WaitsForUpload);
    BeginGPUUpload(chunk->primaryMesh);
    assert(chunk->primaryMesh->gpuBufferPtr);
    chunk->state = ChunkState::UploadingMesh;
    WriteFence();
    if (!PlatformPushWork(GlobalLowPriorityWorkQueue, UploadChunkMeshToGPUWork, chunk, nullptr, nullptr)) {
        chunk->state = ChunkState::WaitsForUpload;
    }
}

void CompleteChunkMeshUpload(Chunk* chunk) {
    assert(chunk->state == ChunkState::MeshUploadingFinished);
    bool completed = EndGPUpload(chunk->primaryMesh);
    if (completed) {
        chunk->state = ChunkState::MeshingFinished;
    }
}
