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
    Entity* entity = nullptr;
    switch (header->kind) {
    case EntityKind::Spatial: {
        auto p = WorldPos::Make(header->spatial.pBlock, header->spatial.pOffset);
        entity = RestoreSpatialEntity(world, header->id, header->type, header->flags, p, header->spatial.velocity, header->spatial.scale, header->spatial.acceleration, header->spatial.friction);
    } break;
    case EntityKind::Block: {
        entity = RestoreBlockEntity(world, header->id, header->type, header->flags, header->block.p);
    } break;
    invalid_default();
    }
    return entity;
}

bool SaveWorldData(GameWorld* world) {
    wchar_t nameBuffer[256];
    swprintf_s(nameBuffer, 128, L"%hs\\%hs.world", world->name, world->name);
    WorldFile data {};
    data.magic = WorldFile::MagicValue;
    data.version = WorldFile::LatestVersion;
    data.entitySerialCount = world->entitySerialCount;

    auto writeResult = PlatformDebugWriteFile(nameBuffer, &data, sizeof(data));

    return writeResult;
}

bool LoadWorldData(GameWorld* world) {
    bool result = false;
    wchar_t nameBuffer[256];
    swprintf_s(nameBuffer, 128, L"%hs\\%hs.world", world->name, world->name);
    auto fileSize = PlatformDebugGetFileSize(nameBuffer);
    if (fileSize == sizeof(WorldFile)) {
        WorldFile data;
        auto readResult = PlatformDebugReadFile(&data, sizeof(data), nameBuffer);
        if (readResult == sizeof(data)) {
            if (data.magic == WorldFile::MagicValue && data.version == WorldFile::LatestVersion) {
                world->entitySerialCount = data.entitySerialCount;
                result = true;
            }
        }
    }
    return result;
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

        auto fileHeader = (EntityFileHeader*)headerTable.Write(sizeof(EntityFileHeader));
        fileHeader->magic = EntityFileHeader::MagicValue;
        fileHeader->version = EntityFileHeader::LatestVersion;
        fileHeader->entityCount = chunk->entityStorage.count;

        ForEach(&chunk->entityStorage, [&](Entity* it) {
            // TODO: Handle player
            if (it->type == EntityType::Player) {
                auto fileHeader = (EntityFileHeader*)headerTable.data;
                fileHeader->entityCount--;
            }
            if (it->type != EntityType::Player) {
                auto header = (EntityHeaderV1*)headerTable.Write(sizeof(EntityHeaderV1));
                SerializeEntityV1(it, header);

                auto info = GetEntityInfo(it->type);
                if (info->Serialize) {
                    header->dataOffset = entityData.at;
                    info->Serialize(it, &entityData);
                    header->dataSize = entityData.at - header->dataOffset;
                } else {
                    header->dataOffset = 0;
                    header->dataSize = 0;
                }
            }
        });

        result = true;

        if (headerTable.at) {
            swprintf_s(nameBuffer, 128, L"%hs\\%ld.%ld.%ld.entities", world->name, chunk->p.x, chunk->p.y, chunk->p.z);
            auto entityHeadersWriteResult = PlatformDebugWriteFile(nameBuffer, headerTable.data, (u32)headerTable.at);
            if (!entityHeadersWriteResult) {
                result = false;
            }
        }

        if (entityData.at) {
            swprintf_s(nameBuffer, 128, L"%hs\\%ld.%ld.%ld.data", world->name, chunk->p.x, chunk->p.y, chunk->p.z);
            auto entityDataWriteResult = PlatformDebugWriteFile(nameBuffer, entityData.data, (u32)entityData.at);
            if (!entityDataWriteResult) {
                result = false;
            }
        }
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
        auto data = PlatformAlloc(headersSize, 0, nullptr);
        defer { PlatformFree(data, nullptr); };
        assert(data);
        auto readHeadersSize = PlatformDebugReadFile(data, (u32)headersSize, nameBuffer);
        if (readHeadersSize == headersSize) {
            // Trying to find and read data file
            void* entityData = nullptr;
            usize entityDataSize = 0;
            swprintf_s(nameBuffer, 128, L"%hs\\%ld.%ld.%ld.data", world->name, chunk->p.x, chunk->p.y, chunk->p.z);
            auto dataSize = PlatformDebugGetFileSize(nameBuffer);
            if (dataSize) {
                auto entityDataBlob = PlatformAlloc(dataSize, 0, nullptr);
                assert(entityDataBlob);
                auto readDataSize = PlatformDebugReadFile(entityDataBlob, (u32)dataSize, nameBuffer);
                if (readDataSize == dataSize) {
                    entityData = entityDataBlob;
                    entityDataSize = dataSize;
                }
            }

            auto fileHeader = (EntityFileHeader*)data;
            if (fileHeader->magic == EntityFileHeader::MagicValue) {
                // TODO: Implement versioning
                assert(fileHeader->version == EntityFileHeader::LatestVersion);
                if (fileHeader->entityCount <= ((headersSize - sizeof(EntityFileHeader)) / sizeof(EntityHeaderV1))) {
                    auto world = GetWorld();
                    auto entityHeadersArrayBegin = (void*)((u8*)data + sizeof(EntityFileHeader));
                    for (usize i = 0; i < fileHeader->entityCount; i++) {
                        auto header = ((EntityHeaderV1*)entityHeadersArrayBegin) + i;
                        auto entity = DeserializeEntityV1(world, header);
                        assert(entity);
                        if (header->dataSize) {
                            auto info = GetEntityInfo(entity->type);
                            if (!entityData) {
                                log_print("[Load] Can't find data for %s entity %llu of type %s\n", ToString(entity->kind), entity->id, info->name);
                            } else {
                                if (info->Deserialize) {
                                    if ((header->dataOffset + header->dataSize) <= entityDataSize) {
                                        auto dataPtr = (u8*)entityData + header->dataOffset;
                                        EntitySerializedData data;
                                        data.data = dataPtr;
                                        data.size = header->dataSize;
                                        data.at = 0;
                                        info->Deserialize(entity, data);
                                    } else {
                                        log_print("[Load] Failed to deserialize %s entity %llu of type %s. It has incorrect data offset or size\n", ToString(entity->kind), entity->id, info->name);
                                    }
                                }
                            }
                        }
                    }
                }
            }
            if (entityData) {
                PlatformFree(entityData, nullptr);
            }
        }
    }
}

bool SaveWorld(GameWorld* world) {
    bool result = true;
    auto pool = &world->chunkPool;
    auto tick = GetPlatform()->tickCount;
    auto worldDataResult = SaveWorldData(world);
    if (worldDataResult) {
        ForEachSimChunk(pool, [&](Chunk* chunk) {
            auto chunkIsCurrentlySaving = AtomicLoad(&chunk->saving);
            if (!chunkIsCurrentlySaving && ((chunk->lastSaveTick < chunk->lastModificationTick) || chunk->simPropagationCount)) {
                auto saved = SaveChunk(chunk);
                if (!saved) {
                    result = false;
                } else {
                    chunk->lastSaveTick = tick;
                }
            }
        });
    }
    while (AtomicLoad(&pool->pendingSavesCount)) {
        log_print("[Save] Spin-lock waiting for all chunks to be saved. %lu in process", pool->pendingSavesCount);
    }
    return result;
}
