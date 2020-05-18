#include "SimRegion.h"

bool IsInside(iv3 min, iv3 max, iv3 x) {
    bool result = false;
    if (x.x >= min.x && x.x <= max.x &&
        x.y >= min.y && x.y <= max.y &&
        x.z >= min.z && x.z <= max.z) {
        result = true;
    }
    return result;
}

void ReturnChunkMeshToPool(SimRegion* region, u32 index) {
    assert(region->chunkMeshPoolFree < region->maxChunkCount);
    region->chunkMeshPoolFree++;
    region->chunkMeshPoolUsage[index] = false;
    auto mesh = region->chunkMeshPool + index;
    FreeChunkMesh(region->world->mesher, mesh);
}

void RemoveChunkFromRegion(SimRegion* region, Chunk* chunk) {
    assert(chunk->active);
    assert(chunk->primaryMesh);
    assert(!chunk->secondaryMesh);
    auto prev = chunk->prevActive;
    auto next = chunk->nextActive;
    chunk->prevActive = nullptr;
    chunk->nextActive = nullptr;
    chunk->active = false;

    ReturnChunkMeshToPool(region, chunk->primaryMeshPoolIndex);
    chunk->primaryMesh = nullptr;
    chunk->primaryMeshValid = false;

    assert(region->chunkCount);
    region->chunkCount--;
    if (!prev) {
        region->firstChunk = next;
        if (next) {
            next->prevActive = nullptr;
        }
    } else {
        prev->nextActive = next;
        if (next) {
            next->prevActive = prev;
        }
    }

    foreach (chunk->spatialEntityStorage) {
        UnregisterSpatialEntity(region, it->id);
    }

    chunk->region = nullptr;
}

void EvictFurthestChunkFromRegion(SimRegion* region) {
    Chunk* furthestChunkOutside = nullptr;
    i32 furthestDistOutside = 0;

    Chunk* furthestChunk = nullptr;
    i32 furthestDist = 0;
    Chunk* chunk = region->firstChunk;
    while (chunk) {
        if (!chunk->locked) {
            i32 dist = LengthSq(region->origin - chunk->p);
            if (!IsInside(region->min, region->max, chunk->p)) {
                if (dist > furthestDistOutside) {
                    furthestDistOutside = dist;
                    furthestChunkOutside = chunk;
                }
            }
            if (dist > furthestDist) {
                furthestDist = dist;
                furthestChunk = chunk;
            }
        }
        chunk = chunk->nextActive;
    }

    if (furthestChunkOutside) {
        RemoveChunkFromRegion(region, furthestChunkOutside);
    } else {
        log_print("[Sim region]: Warn! Evicting a chunk which is inside region bounds\n");
        RemoveChunkFromRegion(region, furthestChunk);
    }
}

struct GetChunkMeshFromPoolResult {
    ChunkMesh* mesh;
    u32 index;
};

GetChunkMeshFromPoolResult GetChunkMeshFromPool(SimRegion* region) {
    GetChunkMeshFromPoolResult result{};
    i32 index = -1;
    for (u32x i = 0; i < region->maxChunkCount; i++) {
        if (!region->chunkMeshPoolUsage[i]) {
            region->chunkMeshPoolUsage[i] = 1;
            index = i;
            break;
        }
    }

    if (index != -1) {
        assert(region->chunkMeshPoolFree);
        region->chunkMeshPoolFree--;
        result = { region->chunkMeshPool + index, (u32)index };
    }
    return result;
}

void ValidateChunkMeshPool(SimRegion* region) {
    u32 count = 0;
    for (u32x i = 0; i < region->maxChunkCount; i++) {
        if (!region->chunkMeshPoolUsage[i]) {
            count++;
        }
    }
    assert(count == region->chunkMeshPoolFree);
}

void AddChunkToRegion(SimRegion* region, Chunk* chunk) {
    assert(!chunk->active);
    assert(!chunk->prevActive);
    assert(!chunk->nextActive);
    assert(!chunk->primaryMeshValid);
    chunk->active = true;
    chunk->nextActive = region->firstChunk;
    if (chunk->nextActive) {
        chunk->nextActive->prevActive = chunk;
    }
    region->firstChunk = chunk;
    region->chunkCount++;

    auto mesh = GetChunkMeshFromPool(region);
    assert(mesh.mesh);
    assert(!chunk->primaryMesh);
    chunk->primaryMesh = mesh.mesh;
    chunk->primaryMeshPoolIndex = mesh.index;

    foreach (chunk->spatialEntityStorage) {
        RegisterSpatialEntity(region, it);
    }

    chunk->region = region;
}

void SwapChunkMeshes(Chunk* chunk) {
    auto tmpMesh = chunk->secondaryMesh;
    chunk->secondaryMesh = chunk->primaryMesh;
    chunk->primaryMesh = tmpMesh;

    auto tmpValid = chunk->secondaryMeshValid;
    chunk->secondaryMeshValid = chunk->primaryMeshValid;
    chunk->primaryMeshValid = tmpValid;

    auto tmpPoolIndex = chunk->secondaryMeshPoolIndex;
    chunk->secondaryMeshPoolIndex = chunk->primaryMeshPoolIndex;
    chunk->primaryMeshPoolIndex = tmpPoolIndex;
}

void RegionUpdateChunkStates(SimRegion* region) {
    Chunk* chunk = region->firstChunk;
    while (chunk) {
        switch (chunk->state) {
        case ChunkState::Complete: {
            if (!chunk->filled) {
                chunk->locked = true;
                if (!ScheduleChunkFill(&region->world->worldGen, chunk)) {
                    chunk->locked = false;
                }
            } else {
                // TODO BeginMeshTask (invalidate mesh) and EndMeshTask
                if (chunk->shouldBeRemeshedAfterEdit) {
                    //log_print("[Sim region] Begining remesing edited chunk\n");
                    assert(chunk->priority == ChunkPriority::Low);
                    if (region->chunkCount == region->maxChunkCount) {
                        EvictFurthestChunkFromRegion(region);
                    }
                    auto mesh = GetChunkMeshFromPool(region);
                    if (mesh.mesh) {
                        chunk->secondaryMesh = mesh.mesh;
                        chunk->secondaryMeshPoolIndex = mesh.index;
                        chunk->secondaryMeshValid = false;

                        assert(!chunk->remeshingAfterEdit);
                        chunk->remeshingAfterEdit = true;

                        SwapChunkMeshes(chunk);
                        chunk->priority = ChunkPriority::High;

                        chunk->shouldBeRemeshedAfterEdit = false;

                        chunk->locked = true;

                        if (!ScheduleChunkMeshing(region->world, chunk)) {
                            SwapChunkMeshes(chunk);
                            chunk->priority = ChunkPriority::Low;
                            chunk->remeshingAfterEdit = false;

                            chunk->shouldBeRemeshedAfterEdit = true;

                            chunk->locked = false;

                            chunk->secondaryMesh = nullptr;
                            chunk->secondaryMeshPoolIndex = 0;
                            chunk->secondaryMeshValid = false;
                            ReturnChunkMeshToPool(region, mesh.index);
                        }
                    }
                } else if (!chunk->primaryMeshValid) {
                    chunk->locked = true;
                    if (!ScheduleChunkMeshing(region->world, chunk)) {
                        chunk->locked = false;
                    }
                }
            }
        } break;
        case ChunkState::Filled: {
            chunk->filled = true;
            chunk->shouldBeRemeshedAfterEdit = false;
            chunk->state = ChunkState::Complete;
            chunk->locked = false;
        } break;
        case ChunkState::MeshingFinished: {
            if (chunk->remeshingAfterEdit) {
                //log_print("[Sim region] End remesing edited chunk\n");
                assert(chunk->priority == ChunkPriority::High);
                chunk->priority = ChunkPriority::Low;
                chunk->remeshingAfterEdit = false;

                //SwapChunkMeshes(chunk);
                ReturnChunkMeshToPool(region, chunk->secondaryMeshPoolIndex);
                chunk->secondaryMesh = nullptr;
                chunk->secondaryMeshPoolIndex = 0;
                chunk->secondaryMeshValid = false;
            }
            chunk->primaryMeshValid = true;
            chunk->state = ChunkState::Complete;
            chunk->locked = false;
            ValidateChunkMeshPool(region);
        } break;
        case ChunkState::WaitsForUpload: {
            if (chunk->primaryMesh->vertexCount) {
                ScheduleChunkMeshUpload(chunk);
            } else {
                chunk->state = ChunkState::MeshingFinished;
            }
        } break;
        case ChunkState::MeshUploadingFinished: {
            CompleteChunkMeshUpload(chunk);
        } break;
        case ChunkState::Filling: {} break;
        case ChunkState::Meshing: {} break;
        case ChunkState::UploadingMesh: {} break;
        invalid_default();
        }
        chunk = chunk->nextActive;
    }
}

void InitRegion(SimRegion* region) {
    region->spatialEntityTable = HashMap<EntityID, SpatialEntity*, SimRegionHashFunc, SimRegionHashCompFunc>::Make();
}

// TODO: Make this an actual function
// TODO: Entity gather
void ResizeRegion(SimRegion* region, u32 newSpan, MemoryArena* arena) {
    region->span = newSpan;
    u32 regionSide = newSpan * 2 + 1;
    u32 regionHeight = GameWorld::MaxHeightChunk - GameWorld::MinHeightChunk + 1;
    region->maxChunkCount = regionSide * regionSide * regionHeight + 16; // TODO: Formalize the number of extra chunks
    region->chunkMeshPool = (ChunkMesh*)PushSize(arena, sizeof(ChunkMesh) * region->maxChunkCount);
    region->chunkMeshPoolUsage = (byte*)PushSize(arena, sizeof(byte) * region->maxChunkCount);
    region->chunkMeshPoolFree = region->maxChunkCount;
    for (u32x i = 0; i < region->maxChunkCount; i++) {
        region->chunkMeshPool[i].mesher = region->world->mesher;
    }
}

void MoveRegion(SimRegion* region, iv3 newP) {
    iv3 newMin = newP - IV3(region->span);
    newMin.y = GameWorld::MinHeightChunk;
    iv3 newMax = newP + IV3(region->span);
    newMax.y = GameWorld::MaxHeightChunk;

    region->origin = newP;
    region->min = newMin;
    region->max = newMax;

    for (i32 z = newMin.z; z <= newMax.z; z++) {
        for (i32 y = newMin.y; y <= newMax.y; y++) {
            for (i32 x = newMin.x; x <= newMax.x; x++) {
                Chunk* chunk = GetChunk(region->world, x, y, z);
                if (!chunk) {
                    chunk = AddChunk(region->world, IV3(x, y, z));
                    assert(chunk);
                }
                // TODO: Checking that chunk is not currently in this region. For now we assume that there is only one
                // region in thr world, so just checking for chunk to be not active. In the future we probably
                // want to have region id's and check is chunk already in this region by this id
                if (!chunk->active) {
                    ValidateChunkMeshPool(region);
                    assert(region->chunkCount <= region->maxChunkCount);
                    if ((region->chunkCount == region->maxChunkCount) || (!region->chunkMeshPoolFree)) {
                        EvictFurthestChunkFromRegion(region);
                    }
                    assert(region->chunkCount < region->maxChunkCount);
                    AddChunkToRegion(region, chunk);
                }
            }
        }
    }
}

void DrawRegion(SimRegion* region, RenderGroup* renderGroup, Camera* camera) {
    auto chunk = region->firstChunk;
    Push(renderGroup, &RenderCommandBeginChunkBatch{});
    while (chunk) {
        auto state = chunk->state;
        ChunkMesh* mesh = nullptr;
        if (chunk->primaryMeshValid) {
            mesh = chunk->primaryMesh;
        } else if (chunk->secondaryMeshValid) {
            mesh = chunk->secondaryMesh;
        }

        if (mesh) {
            if (mesh->vertexCount) {
                assert(chunk->primaryMesh);
                RenderCommandPushChunk chunkCommand = {};
                chunkCommand.mesh = mesh;
                chunkCommand.offset = RelativePos(camera->targetWorldPosition, MakeWorldPos(chunk->p * (i32)Chunk::Size));
                Push(renderGroup, &chunkCommand);
            }
        }
        chunk = chunk->nextActive;
    }
    Push(renderGroup, &RenderCommandEndChunkBatch{});
}

void RegisterSpatialEntity(SimRegion* region, SpatialEntity* entity) {
    auto entry = Add(&region->spatialEntityTable, &entity->id);
    assert(entry);
    *entry = entity;
}

bool UnregisterSpatialEntity(SimRegion* region, EntityID id) {
    bool result = Delete(&region->spatialEntityTable, &id);
    return result;
}

void UpdateEntities(SimRegion* region, RenderGroup* renderGroup, Camera* camera, Context* context) {
    auto chunk = region->firstChunk;
    while (chunk) {
        foreach (chunk->spatialEntityStorage) {
            auto entity = it;
            if (entity->id.id) {
                v3 frameAcceleration = V3(0.0f, -20.8f, 0.0f);
                v3 movementDelta = 0.5f * frameAcceleration * GlobalGameDeltaTime * GlobalGameDeltaTime + entity->velocity * GlobalGameDeltaTime;
                entity->velocity += frameAcceleration * GlobalGameDeltaTime;
                MoveSpatialEntity(region->world, entity, movementDelta, nullptr, nullptr);

                if(UpdateEntityResidence(entity)) {
                    // HACK: Just aborting loop for now
                    // If entity changed it's residence then iterator bocomes invalid
                    // We need a way to continue this loop
                    // Maybe ensure that iterator is still valid if onlu current element changes or smth
                    break;
                }

                if (entity->type == SpatialEntityType::CoalOre) {
                    RenderCommandDrawMesh command {};
                    command.transform = Translate(RelativePos(camera->targetWorldPosition, entity->p));
                    command.mesh = context->coalOreMesh;
                    command.material = &context->coalOreMaterial;
                    Push(renderGroup, &command);
                }
            }
        }
        chunk = chunk->nextActive;
    }
}

SpatialEntity* GetSpatialEntity(SimRegion* region, EntityID id) {
    SpatialEntity* result = nullptr;
    auto ptr = Get(&region->spatialEntityTable, &id);
    if (ptr) {
        result = *ptr;
    }
    return result;
}
