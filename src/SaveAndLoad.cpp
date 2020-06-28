#include "SaveAndLoad.h"
#include "Intrinsics.h"
#include "BinaryBlob.h"

void SaveThreadWork(void* data) {
    // TODO: Thread safe logging
    log_print("Save thread is working...\n");
}

void SerializeEntityV1(Entity* entity, EntityHeaderV1* out) {
    out->headerVersion = 1;
    out->id = entity->id;
    out->type = (u32)entity->type;
    out->kind = (u32)entity->kind;
    switch (entity->kind) {
    case EntityKind::Spatial: {
        auto spatial = (SpatialEntity*)entity;
        out->spatial.pBlock = spatial->p.block;
        out->spatial.pOffset = spatial->p.offset;
        out->spatial.velocity = spatial->velocity;
        out->spatial.scale = spatial->scale;
        out->spatial.acceleration = spatial->acceleration;
        out->spatial.friction = spatial->friction;
    } break;
    case EntityKind::Block: {
        auto block = (BlockEntity*)entity;
        out->block.p = block->p;
    } break;
    invalid_default();
    }
}

Entity* DeserializeEntityV1(GameWorld* world, EntityHeaderV1* header) {
    auto info = GetEntityInfo(header->type);
    assert(info->Create);
    WorldPos p;
    switch (header->kind) {
    case EntityKind::Spatial: {
        p = WorldPos::Make(header->spatial.pBlock, header->spatial.pOffset);
    } break;
    case EntityKind::Block: {
        p = WorldPos::Make(header->block.p);
    } break;
    invalid_default();
    }
    auto entity = info->Create(world, p);
    return entity;
}

bool SaveChunk(Chunk* chunk) {
    bool result = false;
    auto world = GetWorld();
    wchar_t nameBuffer[256];
    swprintf_s(nameBuffer, 128, L"%hs\\%ld.%ld.%ld.chunk", world->name, chunk->p.x, chunk->p.y, chunk->p.z);
    auto dataSize = Chunk::Size * Chunk::Size * Chunk::Size * sizeof(BlockValue);
    auto blockDataWriteResult = PlatformDebugWriteFile(nameBuffer, chunk->blocks, (u32)dataSize);

    if (blockDataWriteResult && chunk->entityStorage.count) {
        BinaryBlob headerTable {};
        BinaryBlob entityData {};
        BinaryBlob::Init(&headerTable, MakeAllocator(PlatformAlloc, PlatformFree, nullptr));
        BinaryBlob::Init(&entityData, MakeAllocator(PlatformAlloc, PlatformFree, nullptr));
        defer {
            headerTable.Destroy();
            entityData.Destroy();
        };


        ForEach(&chunk->entityStorage, [&](Entity* it) {
            if (it->type != EntityType::Player) {
                auto header = (EntityHeaderV1*)headerTable.Write(sizeof(EntityHeaderV1));
                SerializeEntityV1(it, header);
            }
        });

        if (headerTable.at) {
            swprintf_s(nameBuffer, 128, L"%hs\\%ld.%ld.%ld.entities", world->name, chunk->p.x, chunk->p.y, chunk->p.z);
            auto entityHeadersWriteResult = PlatformDebugWriteFile(nameBuffer, headerTable.data, (u32)headerTable.at);
            if (entityHeadersWriteResult) {
                result = true;
            }
        } else {
            result = true;
        }
    } else {
        result = blockDataWriteResult;
    }

    return result;
}

bool TryLoadChunk(Chunk* chunk) {
    bool result = false;
    auto world = GetWorld();
    wchar_t nameBuffer[256];
    swprintf_s(nameBuffer, 128, L"%hs\\%ld.%ld.%ld.chunk", world->name, chunk->p.x, chunk->p.y, chunk->p.z);
    auto dataSize = Chunk::Size * Chunk::Size * Chunk::Size * sizeof(BlockValue);
    auto blockDataLoadResult = PlatformDebugReadFile(chunk->blocks, (u32)dataSize, nameBuffer);
    return blockDataLoadResult;
}

void TryLoadEntities(Chunk* chunk) {
    bool result = false;
    auto world = GetWorld();
    wchar_t nameBuffer[256];
    swprintf_s(nameBuffer, 128, L"%hs\\%ld.%ld.%ld.entities", world->name, chunk->p.x, chunk->p.y, chunk->p.z);
    auto headersSize = PlatformDebugGetFileSize(nameBuffer);
    if (headersSize) {
        assert((headersSize % sizeof(EntityHeaderV1)) == 0);
        auto data = PlatformAlloc(headersSize, 0, nullptr);
        defer { PlatformFree(data, nullptr); };
        assert(data);
        auto readHeadersSize = PlatformDebugReadFile(data, (u32)headersSize, nameBuffer);
        if (readHeadersSize == headersSize) {
            auto headerCount = headersSize / sizeof(EntityHeaderV1);
            auto world = GetWorld();
            for (usize i = 0; i < headerCount; i++) {
                auto header = ((EntityHeaderV1*)data) + i;
                auto entity = DeserializeEntityV1(world, header);
                assert(entity);
            }
        }
    }
}

bool SaveWorld(GameWorld* world) {
    bool result = true;
    auto pool = &world->chunkPool;
    auto tick = GetPlatform()->tickCount;
    ForEachSimChunk(pool, [&](Chunk* chunk) {
        auto chunkIsCurrentlySaving = AtomicLoad(&chunk->saving);
        if (!chunkIsCurrentlySaving && chunk->lastSaveTick < chunk->lastModificationTick) {
            auto saved = SaveChunk(chunk);
            if (!saved) {
                result = false;
            } else {
                chunk->lastSaveTick = tick;
            }
        }
    });
    while (AtomicLoad(&pool->pendingSavesCount)) {
        log_print("[Save] Spin-lock waiting for all chunks to be saved. %lu in process", pool->pendingSavesCount);
    }
    return result;
}
