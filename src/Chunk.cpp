#include "Chunk.h"

void EntityStorageInsert(EntityStorage* storage, Entity* entity) {
    if (storage->first) {
        assert(entity->id != storage->first->id);
    }
    entity->nextInStorage = storage->first;
    if (storage->first) storage->first->prevInStorage = entity;
    storage->first = entity;
    storage->count++;
}

void EntityStorageUnlink(EntityStorage* storage, Entity* entity) {
    auto prev = entity->prevInStorage;
    auto next = entity->nextInStorage;
    if (prev) {
        prev->nextInStorage = next;
    } else {
        storage->first = next;
    }
    if (next) {
        next->prevInStorage = prev;
    }
    assert(storage->count);
    storage->count--;
    entity->prevInStorage = nullptr;
    entity->nextInStorage = nullptr;
}

Block* GetBlockRaw(Chunk* chunk, u32 x, u32 y, u32 z) {
    Block* result = &chunk->voxels[x + Chunk::Size * y + Chunk::Size * Chunk::Size * z];
    return result;
}

const Block* GetBlock(Chunk* chunk, u32 x, u32 y, u32 z) {
    const Block* result = nullptr;
    if (x < Chunk::Size && y < Chunk::Size && z < Chunk::Size) {
        result = GetBlockRaw(chunk, x, y, z);
    } else {
        result = &chunk->nullBlock;
    }
    return result;
}

bool OccupyBlock(Chunk* chunk, BlockEntity* entity, u32 x, u32 y, u32 z) {
    bool result = false;
    if (x < Chunk::Size && y < Chunk::Size && z < Chunk::Size) {
        auto voxel = GetBlockRaw(chunk, x, y, z);
        assert(voxel);
        if (!voxel->entity) {
            voxel->entity = entity;
            if (entity->flags & EntityFlag_PropagatesSim) {
                chunk->simPropagationCount++;
            }
            result = true;
        }
    }
    return result;
}

bool ReleaseBlock(Chunk* chunk, BlockEntity* entity, u32 x, u32 y, u32 z) {
    bool result = false;
    if (x < Chunk::Size && y < Chunk::Size && z < Chunk::Size) {
        auto voxel = GetBlockRaw(chunk, x, y, z);
        assert(voxel);
        // Only entity that lives here allowed to release voxel
        assert(voxel->entity->id == entity->id);
        if (voxel->entity->id == entity->id) {
            voxel->entity = nullptr;
            if (entity->flags & EntityFlag_PropagatesSim) {
                assert(chunk->simPropagationCount > 0);
                chunk->simPropagationCount--;
            }
            result = true;
        }
    }
    return result;
}

Block* GetBlockForModification(Chunk* chunk, u32 x, u32 y, u32 z) {
    Block* result = nullptr;
    if (x < Chunk::Size && y < Chunk::Size && z < Chunk::Size) {
        result = GetBlockRaw(chunk, x, y, z);
        chunk->shouldBeRemeshedAfterEdit = true;
        chunk->modified = true;
    }
    return result;
}

const char* ToString(ChunkState state) {
    switch (state) {
    case ChunkState::Complete: { return "Complete"; } break;
    case ChunkState::Filling: { return "Filling"; } break;
    case ChunkState::Filled: { return "Filled"; } break;
    case ChunkState::Meshing: { return "Meshing"; } break;
    case ChunkState::MeshingFinished: { return "MeshingFinished"; } break;
    case ChunkState::WaitsForUpload: { return "WaitsForUpload"; } break;
    case ChunkState::MeshUploadingFinished: { return "MeshUploadingFinished"; } break;
    case ChunkState::UploadingMesh: { return "UploadingMesh"; } break;
    invalid_default();
    }
    return "<unknown>";
}

const char* ToString(ChunkPriority state) {
    switch (state) {
    case ChunkPriority::Low: { return "Low"; } break;
    case ChunkPriority::High: { return "High"; } break;
        invalid_default();
    }
    return "<unknown>";
}
