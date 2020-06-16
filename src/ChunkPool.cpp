#include "ChunkPool.h"
#include "SaveAndLoad.h"

bool IsInside(iv3 min, iv3 max, iv3 x) {
    bool result = false;
    if (x.x >= min.x && x.x <= max.x &&
        x.y >= min.y && x.y <= max.y &&
        x.z >= min.z && x.z <= max.z) {
        result = true;
    }
    return result;
}

void ReturnChunkMeshToPool(ChunkPool* pool, u32 index) {
    assert(pool->chunkMeshPoolFree < pool->maxRenderedChunkCount);
    pool->chunkMeshPoolFree++;
    pool->chunkMeshPoolUsage[index] = false;
    auto mesh = pool->chunkMeshPool + index;
    FreeChunkMesh(pool->mesher, mesh);
}

void RemoveChunkFromRenderPool(ChunkPool* pool, Chunk* chunk) {
    assert(chunk->visible);
    assert(chunk->primaryMesh);
    assert(!chunk->secondaryMesh);
    assert(!chunk->locked);
    auto prev = chunk->prevRendered;
    auto next = chunk->nextRendered;
    chunk->prevRendered = nullptr;
    chunk->nextRendered = nullptr;
    chunk->visible = false;

    ReturnChunkMeshToPool(pool, chunk->primaryMeshPoolIndex);
    chunk->primaryMesh = nullptr;
    chunk->primaryMeshValid = false;

    assert(pool->renderedChunkCount);
    pool->renderedChunkCount--;
    if (!prev) {
        pool->firstRenderedChunk = next;
        if (next) {
            next->prevRendered = nullptr;
        }
    } else {
        prev->nextRendered = next;
        if (next) {
            next->prevRendered = prev;
        }
    }
}

void RemoveChunkFromSimPool(ChunkPool* pool, Chunk* chunk) {
    assert(!chunk->visible);
    assert(chunk->active);
    assert(!chunk->simPropagationCount);
    assert(!chunk->locked);
    auto prev = chunk->prevActive;
    auto next = chunk->nextActive;
    chunk->prevActive = nullptr;
    chunk->nextActive = nullptr;
    chunk->active = false;

    assert(chunk->primaryMesh == nullptr);
    assert(chunk->primaryMeshValid == false);

    assert(pool->simChunkCount);
    pool->simChunkCount--;

    if (!prev) {
        pool->firstSimChunk = next;
        if (next) {
            next->prevActive = nullptr;
        }
    } else {
        prev->nextActive = next;
        if (next) {
            next->prevActive = prev;
        }
    }

    ForEach (&chunk->entityStorage, [&] (Entity* it) {
        UnregisterEntity(pool->world, it->id);
    });

    if (!chunk->simPropagationCount) {
        if (chunk->lastModificationTick) {
            auto saveResult = SaveChunk(chunk);
            assert(saveResult);
        }
        DeleteChunk(pool->world, chunk);
    }
}

void MakeRoomForChunkInRenderPool(ChunkPool* pool) {
    Chunk* furthestChunkOutside = nullptr;
    i32 furthestDistOutside = 0;

    Chunk* furthestChunk = nullptr;
    i32 furthestDist = 0;
    Chunk* chunk = pool->firstRenderedChunk;
    while (chunk) {
        if (!chunk->locked && chunk->visible) {
            i32 dist = LengthSq(pool->playerRegion.origin - chunk->p);
            if (!IsInside(pool->playerRegion.min, pool->playerRegion.max, chunk->p)) {
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
        chunk = chunk->nextRendered;
    }

    if (furthestChunkOutside) {
        RemoveChunkFromRenderPool(pool, furthestChunkOutside);
    } else {
        //pool->maxChunkCount++;
        log_print("[Render pool]: Warn! Chunk {%ld, %ld, %ld}, which is inside visible region was evicted from render pool\n", furthestChunk->p.x, furthestChunk->p.y, furthestChunk->p.z);
        RemoveChunkFromRenderPool(pool, furthestChunk);
    }
}

void MakeRoomForChunkInSimPool(ChunkPool* pool) {
    Chunk* furthestChunkOutside = nullptr;
    i32 furthestDistOutside = 0;

    Chunk* chunk = pool->firstSimChunk;
    while (chunk) {
        if (!chunk->locked && (chunk->simPropagationCount == 0) && (!chunk->visible)) {
            i32 dist = LengthSq(pool->playerRegion.origin - chunk->p);
            if (!IsInside(pool->playerRegion.min, pool->playerRegion.max, chunk->p)) {
                if (dist > furthestDistOutside) {
                    furthestDistOutside = dist;
                    furthestChunkOutside = chunk;
                }
            }
        }
        chunk = chunk->nextActive;
    }

    if (furthestChunkOutside) {
        RemoveChunkFromSimPool(pool, furthestChunkOutside);
    } else {
        pool->maxSimChunkCount++;
        log_print("[Sim pool]: Growing sim pool because all chunks either simulated or visible. New pool size: %lu\n", pool->maxSimChunkCount);
        //RemoveChunkFromSimPool(pool, furthestChunk);
    }
}

struct GetChunkMeshFromPoolResult {
    ChunkMesh* mesh;
    u32 index;
};

GetChunkMeshFromPoolResult GetChunkMeshFromPool(ChunkPool* pool) {
    GetChunkMeshFromPoolResult result{};
    i32 index = -1;
    for (u32x i = 0; i < pool->maxRenderedChunkCount; i++) {
        if (!pool->chunkMeshPoolUsage[i]) {
            pool->chunkMeshPoolUsage[i] = 1;
            index = i;
            break;
        }
    }

    if (index != -1) {
        assert(pool->chunkMeshPoolFree);
        pool->chunkMeshPoolFree--;
        result = { pool->chunkMeshPool + index, (u32)index };
    }
    return result;
}

void ValidateChunkMeshPool(ChunkPool* pool) {
    u32 count = 0;
    for (u32x i = 0; i < pool->maxRenderedChunkCount; i++) {
        if (!pool->chunkMeshPoolUsage[i]) {
            count++;
        }
    }
    assert(count == pool->chunkMeshPoolFree);
}

void AddChunkToRenderPool(ChunkPool* pool, Chunk* chunk) {
    assert(!chunk->visible);
    assert(!chunk->prevRendered);
    assert(!chunk->nextRendered);
    assert(!chunk->primaryMeshValid);
    chunk->visible = true;
    chunk->nextRendered = pool->firstRenderedChunk;
    if (chunk->nextRendered) {
        chunk->nextRendered->prevRendered = chunk;
    }
    pool->firstRenderedChunk = chunk;
    pool->renderedChunkCount++;

    auto mesh = GetChunkMeshFromPool(pool);
    assert(mesh.mesh);
    assert(!chunk->primaryMesh);
    chunk->primaryMesh = mesh.mesh;
    chunk->primaryMeshPoolIndex = mesh.index;


    ForEach(&chunk->entityStorage, [&](Entity* it) {
            RegisterEntity(pool->world, it);
    });
}

void AddChunkToSimPool(ChunkPool* pool, Chunk* chunk) {
    assert(!chunk->active);
    assert(!chunk->prevActive);
    assert(!chunk->nextActive);
    //assert(chunk->simPropagationCount);
    chunk->active = true;

    chunk->nextActive = pool->firstSimChunk;
    if (chunk->nextActive) {
        chunk->nextActive->prevActive = chunk;
    }

    pool->firstSimChunk = chunk;
    pool->simChunkCount++;

    ForEach(&chunk->entityStorage, [&](Entity* it) {
        RegisterEntity(pool->world, it);
    });
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

void UpdateChunks(ChunkPool* pool) {
    Chunk* chunk = pool->firstSimChunk;
    // TODO: Mause cache chunks that are not filled or smth and update them in a separate loop.
    // Then we don't need to loop over all active chunks each frame, but only over visible ones
    while (chunk) {
        if (!chunk->filled) {
            if (chunk->state == ChunkState::Complete) {
                if (TryLoadChunk(chunk)) {
                    chunk->filled = true;
                    chunk->lastModificationTick = true;
                    chunk->shouldBeRemeshedAfterEdit = false;
                    chunk->state = ChunkState::Complete;
                    chunk->locked = false;
                } else {
                    chunk->locked = true;
                    if (!ScheduleChunkFill(&pool->worldGen, chunk)) {
                        chunk->locked = false;
                    }
                }
            } else if (chunk->state == ChunkState::Filled) {
                chunk->filled = true;
                chunk->shouldBeRemeshedAfterEdit = false;
                chunk->state = ChunkState::Complete;
                chunk->locked = false;
            } else if (chunk->state == ChunkState::Filling) {
            } else {
                unreachable();
            }
        } else if (chunk->visible) {
            switch (chunk->state) {
            case ChunkState::Complete: {
                // TODO BeginMeshTask (invalidate mesh) and EndMeshTask
                if (chunk->shouldBeRemeshedAfterEdit) {
                    //log_print("[Sim pool] Begining remesing edited chunk\n");
                    assert(chunk->priority == ChunkPriority::Low);
                    if (pool->renderedChunkCount == pool->maxRenderedChunkCount) {
                        MakeRoomForChunkInRenderPool(pool);
                    }
                    auto mesh = GetChunkMeshFromPool(pool);
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

                        if (!ScheduleChunkMeshing(pool->world, chunk)) {
                            SwapChunkMeshes(chunk);
                            chunk->priority = ChunkPriority::Low;
                            chunk->remeshingAfterEdit = false;

                            chunk->shouldBeRemeshedAfterEdit = true;

                            chunk->locked = false;

                            chunk->secondaryMesh = nullptr;
                            chunk->secondaryMeshPoolIndex = 0;
                            chunk->secondaryMeshValid = false;
                            ReturnChunkMeshToPool(pool, mesh.index);
                        }
                    }
                } else if (!chunk->primaryMeshValid) {
                    chunk->locked = true;
                    if (!ScheduleChunkMeshing(pool->world, chunk)) {
                        chunk->locked = false;
                    }
                }
            } break;
            case ChunkState::MeshingFinished: {
                if (chunk->remeshingAfterEdit) {
                    //log_print("[Sim pool] End remesing edited chunk\n");
                    assert(chunk->priority == ChunkPriority::High);
                    chunk->priority = ChunkPriority::Low;
                    chunk->remeshingAfterEdit = false;

                    //SwapChunkMeshes(chunk);
                    ReturnChunkMeshToPool(pool, chunk->secondaryMeshPoolIndex);
                    chunk->secondaryMesh = nullptr;
                    chunk->secondaryMeshPoolIndex = 0;
                    chunk->secondaryMeshValid = false;
                }
                chunk->primaryMeshValid = true;
                chunk->state = ChunkState::Complete;
                chunk->locked = false;
                ValidateChunkMeshPool(pool);
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
        }
        chunk = chunk->nextActive;
    }
}

void InitChunkPool(ChunkPool* pool, GameWorld* world, ChunkMesher* mesher, u32 newSpan, u32 seed) {
    auto region = &pool->playerRegion;
    pool->world = world;
    region->pool = pool;
    region->span = newSpan;

    pool->worldGen.Init(seed);
    pool->mesher = mesher;


    u32 regionSide = newSpan * 2 + 1;
    u32 regionHeight = GameWorld::MaxHeightChunk - GameWorld::MinHeightChunk + 1;
    pool->maxRenderedChunkCount = regionSide * regionSide * regionHeight + 16; // TODO: Formalize the number of extra chunks
    pool->maxSimChunkCount = regionSide * regionSide * regionHeight + 32; // TODO: Formalize the number of extra chunks
    pool->chunkMeshPool = (ChunkMesh*)PlatformAlloc(sizeof(ChunkMesh) * pool->maxRenderedChunkCount, 0, nullptr);
    pool->chunkMeshPoolUsage = (byte*)PlatformAlloc(sizeof(byte) * pool->maxRenderedChunkCount, 0, nullptr);
    ClearArray(pool->chunkMeshPool, pool->maxRenderedChunkCount);
    ClearArray(pool->chunkMeshPoolUsage, pool->maxRenderedChunkCount);
    pool->chunkMeshPoolFree = pool->maxRenderedChunkCount;
    for (u32x i = 0; i < pool->maxRenderedChunkCount; i++) {
        pool->chunkMeshPool[i].mesher = pool->mesher;
    }
}

void MoveRegion(SimRegion* region, iv3 newP) {
    auto pool = region->pool;

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
                Chunk* chunk = GetChunkInternal(pool->world, x, y, z);
                if (!chunk) {
                    chunk = AddChunk(pool->world, IV3(x, y, z));
                    assert(chunk);
                }

                if (!chunk->active) {
                    assert(pool->simChunkCount <= pool->maxSimChunkCount);
                    if ((pool->simChunkCount == pool->maxSimChunkCount)) {
                        MakeRoomForChunkInSimPool(pool);
                    }
                    assert(pool->simChunkCount < pool->maxSimChunkCount);
                    AddChunkToSimPool(pool, chunk);
                }

                if (!chunk->visible) {
                    assert(chunk->active);
                    ValidateChunkMeshPool(pool);
                    assert(pool->renderedChunkCount <= pool->maxRenderedChunkCount);
                    if ((pool->renderedChunkCount == pool->maxRenderedChunkCount) || (!pool->chunkMeshPoolFree)) {
                        MakeRoomForChunkInRenderPool(pool);
                    }
                    assert(pool->renderedChunkCount < pool->maxRenderedChunkCount);
                    AddChunkToRenderPool(pool, chunk);
                }
            }
        }
    }
}

void DrawChunks(ChunkPool* pool, RenderGroup* renderGroup, Camera* camera) {
    auto chunk = pool->firstRenderedChunk;
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
                chunkCommand.offset = WorldPos::Relative(camera->targetWorldPosition, WorldPos::Make(chunk->p * (i32)Chunk::Size));
                Push(renderGroup, &chunkCommand);
            }
        }
        chunk = chunk->nextRendered;
    }
    Push(renderGroup, &RenderCommandEndChunkBatch{});
}

void UpdateChunkEntities(ChunkPool* pool, RenderGroup* renderGroup, Camera* camera) {
    ForEachEntity(pool, [&](Entity* it) {
        auto info = GetEntityInfo(it->type);
        if (info->Behavior) {
            it->generation = GetPlatform()->tickCount;
            EntityUpdateAndRenderData data;
            data.deltaTime = GetPlatform()->gameDeltaTime;
            data.group = renderGroup;
            data.camera = camera;
            info->Behavior(it, EntityBehaviorInvoke::UpdateAndRender, &data);
        }
        if (it->id) {
            if (it->kind == EntityKind::Spatial) {
                auto entity = static_cast<SpatialEntity*>(it);
                if (entity->flags & EntityFlag_ProcessOverlaps) {
                    FindOverlapsFor(pool->world, entity);
                }
                if (EntityShouldBeMovedIntoAnotherChunk(entity)) {
                    auto slot = FlatArrayPush(&pool->world->entitiesToMove);
                    *slot = entity;
                }
            }
        }
    });
}

template <typename F>
void ForEachEntity(ChunkPool* pool, F func) {
    auto chunk = pool->firstSimChunk;
    while (chunk) {
        ForEach(&chunk->entityStorage, func);
        chunk = chunk->nextActive;
    }
}

template <typename F>
void ForEachSimChunk(ChunkPool* pool, F func) {
    auto chunk = pool->firstSimChunk;
    while (chunk) {
        func(chunk);
        chunk = chunk->nextActive;
    }
}
