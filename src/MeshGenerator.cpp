#include "MeshGenerator.h"

#include "flux_intrinsics.h"

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
        block = (ChunkMeshBlock*)PlatformAlloc(sizeof(ChunkMeshBlock));
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
    assert(chunk->mesh);
    ChunkMesh* mesh = chunk->mesh;

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

void ClaimFurthestChunkMesh(GameWorld* world, ChunkMesher* mesher, Chunk* chunk) {
    // TODO: If new and old chunk are currently drawn we should not reclaim that mesh
    // in order to avoid them ping-ponging each other
    u32 furthestDistSq = 0;
    u32 index;
    for (u32x i = 0; i < array_count(mesher->chunkMeshPool); i++) {
        auto entry = mesher->chunkMeshPool + i;
        if (!entry->chunk) {
            index = i;
            break;
        }
        auto state = entry->mesh.state;
        assert(state != ChunkMesh::State::Empty);
        if (state == ChunkMesh::State::Complete) {
            iv3 p = entry->chunk->p;
            u32 distSq = LengthSq(chunk->p - p);
            if (distSq > furthestDistSq) {
                furthestDistSq = distSq;
                index = i;
            }
        }
    }

    auto furthestRecord = mesher->chunkMeshPool + index;
    furthestRecord->mesh.poolIndex = index;
    if (furthestRecord->chunk) {
        auto chunkToDelete = furthestRecord->chunk;
        furthestRecord->chunk->mesh = nullptr;
        // TODO: HACK: Temporary deleting evicted and not modified by player chunks here
        if (chunk && !chunk->modified) {
            Delete(&world->chunkHashMap, &chunkToDelete->p);
            PlatformFree(chunkToDelete);
        }
    }

    FreeChunkMesh(mesher, &furthestRecord->mesh);
    furthestRecord->chunk = chunk;
    chunk->mesh = &furthestRecord->mesh;
    chunk->mesh->state = ChunkMesh::State::Empty;
}

void ReturnChunkMeshToPool(ChunkMesher* mesher, Chunk* chunk) {
    auto mesh = chunk->mesh;
    chunk->mesh = nullptr;
    auto poolIndex= mesh->poolIndex;
    auto record = mesher->chunkMeshPool + poolIndex;
    record->chunk = nullptr;
    assert(mesh->state == ChunkMesh::State::Complete || mesh->state == ChunkMesh::State::Empty);
    if (mesh->state == ChunkMesh::State::Complete) {
        FreeChunkMesh(mesher, mesh);
        mesh->state = ChunkMesh::State::Empty;
    }
}

void UploadChunkMeshToGPUWork(void* data, u32 threadID) {
    auto mesh = (ChunkMesh*)data;
    assert(mesh->state == ChunkMesh::State::ScheduledUpload);
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
    mesh->state = ChunkMesh::State::UploadComplete;
}

void ChunkMesherWork(void* data, u32 threadID) {
    auto chunk = (Chunk*)data;
    auto mesh = chunk->mesh;
    GenMesh(mesh->mesher, chunk);
    assert(mesh->state == ChunkMesh::State::Queued);
    if (GlobalPlatform.supportsAsyncGPUTransfer) {
        if (chunk->mesh->vertexCount) {
            auto uploaded = UploadToGPU(chunk->mesh, false);
            assert(uploaded);
        }
        WriteFence();
        mesh->state = ChunkMesh::State::Complete;
    } else {
        WriteFence();
        mesh->state = ChunkMesh::State::ReadyForUpload;
    }
}

void ScheduleChunkMeshing(GameWorld* world, ChunkMesher* mesher, Chunk* chunk) {
    assert(!chunk->mesh);
    ClaimFurthestChunkMesh(world, mesher, chunk);
    auto mesh = chunk->mesh;
    assert(mesh->state == ChunkMesh::State::Empty);
    mesh->state = ChunkMesh::State::Queued;
    mesh->mesher = mesher;
    WriteFence();
    if (!PlatformPushWork(GlobalPlaformWorkQueue, chunk, ChunkMesherWork)) {
        mesh->state = ChunkMesh::State::Empty;
        ReturnChunkMeshToPool(mesher, chunk);
    }
}

void ScheduleChunkMeshUpload(ChunkMesh* mesh) {
    assert(mesh->state == ChunkMesh::State::ReadyForUpload);
    BeginGPUUpload(mesh);
    assert(mesh->gpuBufferPtr);
    mesh->state = ChunkMesh::State::ScheduledUpload;
    WriteFence();
    if (!PlatformPushWork(GlobalPlaformWorkQueue, mesh, UploadChunkMeshToGPUWork)) {
        mesh->state = ChunkMesh::State::ReadyForUpload;
    }
}

void CompleteChunkMeshUpload(ChunkMesh* mesh) {
    assert(mesh->state == ChunkMesh::State::UploadComplete);
    bool completed = EndGPUpload(mesh);
    if (completed) {
        mesh->state = ChunkMesh::State::Complete;
    }
}
