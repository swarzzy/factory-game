#include "Chunk.h"

template <typename F>
void ForEach(EntityStorage* storage, F func) {
    auto entity = storage->first;
    while (entity) {
        func(entity);
        entity = entity->nextInStorage;
    }
}

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

BlockValue* GetBlockValueRaw(Chunk* chunk, u32 x, u32 y, u32 z) {
    BlockValue* result = chunk->blocks + (x + Globals::ChunkSize * y + Globals::ChunkSize * Globals::ChunkSize * z);
    return result;
}

BlockValue GetBlockValue(Chunk* chunk, u32 x, u32 y, u32 z) {
    const BlockValue* result = nullptr;
    if (x < Globals::ChunkSize && y < Globals::ChunkSize && z < Globals::ChunkSize) {
        result = GetBlockValueRaw(chunk, x, y, z);
    } else {
        result = &chunk->nullBlockValue;
    }
    return *result;
}

BlockEntity** GetBlockEntityRaw(Chunk* chunk, u32 x, u32 y, u32 z) {
    BlockEntity** result = nullptr;
    result = chunk->livingEntities + (x + Globals::ChunkSize * y + Globals::ChunkSize * Globals::ChunkSize * z);
    return result;
}

BlockEntity* GetBlockEntity(Chunk* chunk, u32 x, u32 y, u32 z) {
    BlockEntity* result = nullptr;
    if (x < Globals::ChunkSize && y < Globals::ChunkSize && z < Globals::ChunkSize) {
        result = *GetBlockEntityRaw(chunk, x, y, z);
    }
    return result;
}

Block GetBlock(Chunk* chunk, u32 x, u32 y, u32 z) {
    Block block { chunk->nullBlockValue, nullptr };
    if (x < Globals::ChunkSize && y < Globals::ChunkSize && z < Globals::ChunkSize) {
        auto value = GetBlockValueRaw(chunk, x, y, z);
        auto entity = GetBlockEntityRaw(chunk, x, y, z);
        block.value = *value;
        block.entity = *entity;
    }
    return block;
}

bool OccupyBlock(Chunk* chunk, BlockEntity* entity, u32 x, u32 y, u32 z) {
    bool result = false;
    if (x < Globals::ChunkSize && y < Globals::ChunkSize && z < Globals::ChunkSize) {
        auto livingEntity = GetBlockEntityRaw(chunk, x, y, z);
        if (!(*livingEntity)) {
            *livingEntity = entity;
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
    if (x < Globals::ChunkSize && y < Globals::ChunkSize && z < Globals::ChunkSize) {
        auto ptr = GetBlockEntityRaw(chunk, x, y, z);
        assert(*ptr);
        auto livingEntity = *ptr;
        // Only entity that lives here allowed to release voxel
        assert(livingEntity->id == entity->id);
        if (livingEntity->id == entity->id) {
            *ptr = nullptr;
            if (entity->flags & EntityFlag_PropagatesSim) {
                assert(chunk->simPropagationCount > 0);
                chunk->simPropagationCount--;
            }
            result = true;
        }
    }
    return result;
}

BlockValue* GetBlockForModification(Chunk* chunk, u32 x, u32 y, u32 z) {
    BlockValue* result = nullptr;
    if (x < Globals::ChunkSize && y < Globals::ChunkSize && z < Globals::ChunkSize) {
        result = GetBlockValueRaw(chunk, x, y, z);
        chunk->shouldBeRemeshedAfterEdit = true;
        chunk->lastModificationTick = GetPlatform()->tickCount;
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
