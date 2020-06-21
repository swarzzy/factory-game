#include "World.h"
#include "Renderer.h"
#include "entities/Pickup.h"
#include "entities/Container.h"

Chunk* AllocateWorldChunk(WorldMemory* memory) {
    timed_scope();
    Chunk* result = nullptr;
    if (memory->chunkMemoryFreeList) {
        result = memory->chunkMemoryFreeList;
        memory->chunkMemoryFreeList = memory->chunkMemoryFreeList->nextInFreeList;
        // TODO: Maybe clearing the whole chunk is too slow
        ClearMemory(result);
        memory->chunksUsed++;
        assert(memory->chunksFree);
        memory->chunksFree--;
    } else {
        result = (Chunk*)memory->PageAlloc(sizeof(Chunk));
        if (result) {
            memory->chunksAllocated++;
            memory->chunksUsed++;
        }
    }
    return result;
}

void FreeWorldChunk(WorldMemory* memory, Chunk* chunk) {
    timed_scope();
    // Too many asserts!!!
    assert(!chunk->locked);
    assert(!chunk->visible);
    assert(!chunk->active);
    assert(!chunk->primaryMesh);
    assert(!chunk->secondaryMesh);

    chunk->nextInFreeList = memory->chunkMemoryFreeList;
    memory->chunkMemoryFreeList = chunk;
    assert(memory->chunksUsed);
    memory->chunksUsed--;
    memory->chunksFree++;
}


BlockValue GetBlockValue(GameWorld* world, i32 x, i32 y, i32 z) {
    timed_scope();
    BlockValue result = world->nullBlockValue;
    auto chunkP = WorldPos::ToChunk(IV3(x, y, z));
    Chunk* chunk = GetChunk(world, chunkP.chunk.x, chunkP.chunk.y, chunkP.chunk.z);
    if (chunk) {
        result = GetBlockValue(chunk, chunkP.block.x, chunkP.block.y, chunkP.block.z);
    }
    return result;
}

BlockEntity* GetBlockEntity(GameWorld* world, i32 x, i32 y, i32 z) {
    timed_scope();
    BlockEntity* result = nullptr;
    auto chunkP = WorldPos::ToChunk(IV3(x, y, z));
    Chunk* chunk = GetChunk(world, chunkP.chunk.x, chunkP.chunk.y, chunkP.chunk.z);
    if (chunk) {
        result = GetBlockEntity(chunk, chunkP.block.x, chunkP.block.y, chunkP.block.z);
    }
    return result;
}

Block GetBlock(GameWorld* world, i32 x, i32 y, i32 z) {
    timed_scope();
    Block result = { world->nullBlockValue, nullptr };
    auto chunkP = WorldPos::ToChunk(IV3(x, y, z));
    Chunk* chunk = GetChunk(world, chunkP.chunk.x, chunkP.chunk.y, chunkP.chunk.z);
    if (chunk) {
        result = GetBlock(chunk, chunkP.block.x, chunkP.block.y, chunkP.block.z);
    }
    return result;
}

Chunk* AddChunk(GameWorld* world, iv3 coord) {
    timed_scope();
    auto chunk = AllocateWorldChunk(&world->memory);
    assert(chunk);
    chunk->p = coord;
    chunk->priority = ChunkPriority::Low;
    auto entry = Add(&world->chunkHashMap, &chunk->p);
    assert(entry);
    *entry = chunk;
    return chunk;
}

void DeleteChunk(GameWorld* world, Chunk* chunk) {
    timed_scope();
    bool deleted = Delete(&world->chunkHashMap, &chunk->p);
    assert(deleted);
    FreeWorldChunk(&world->memory, chunk);
}

Chunk* GetChunkInternal(GameWorld* world, i32 x, i32 y, i32 z) {
    Chunk* result = nullptr;
    iv3 key = IV3(x, y, z);
    Chunk** entry = Get(&world->chunkHashMap, &key);
    if (entry) {
        result = *entry;
    }
    return result;
}

Chunk* GetChunk(GameWorld* world, i32 x, i32 y, i32 z) {
    timed_scope();
    Chunk* result = nullptr;
    Chunk* chunk = GetChunkInternal(world, x, y, z);
    if (chunk && chunk->filled) {
        result = chunk;
    }
    return result;
}

void InitWorld(GameWorld* world, Context* context, ChunkMesher* mesher, u32 seed, const char* name) {
    timed_scope();
    world->chunkHashMap = HashMap<iv3, Chunk*, ChunkHashFunc, ChunkHashCompFunc>::Make();
    world->entityHashMap = HashMap<EntityID, Entity*, EntityRegionHashFunc, EntityRegionHashCompFunc>::Make();

    world->memory.PageAlloc = PlatformAllocatePages;
    world->memory.PageDealloc = PlatformDeallocatePages;

    // nocheckin
    // TODO: Error checking
    strcpy_s(world->name, array_count(world->name), name);

    world->camera = &context->camera;
    BucketArrayInit(&world->entitiesToDelete, MakeAllocator(PlatformAlloc, PlatformFree, nullptr));
    FlatArrayInit(&world->entitiesToMove, MakeAllocator(PlatformAlloc, PlatformFree, nullptr), 128);
    InitChunkPool(&world->chunkPool, world, mesher, GameWorld::ViewDistance, seed);
}

template <typename T>
T* AddSpatialEntity(GameWorld* world, WorldPos p) {
    timed_scope();
    // Trigger compiler error if T isn 't inherited from SpatialEntity
    static_cast<T*>(((SpatialEntity*)(0)));
    T* entity = nullptr;
    auto worldPos = WorldPos::Normalize(p);
    auto chunkP = WorldPos::ToChunk(worldPos).chunk;
    auto chunk = GetChunk(world, chunkP.x, chunkP.y, chunkP.z);
    if (chunk) {
        entity = (T*)PlatformAlloc(sizeof(T), alignof(T), nullptr);
        ClearMemory(entity);
        if (entity) {
            EntityStorageInsert(&chunk->entityStorage, entity);
            entity->id = GenEntityID(world, EntityKind::Spatial);
            entity->kind = EntityKind::Spatial;
            // NOTE: All spatial entitites propagates sim for now
            entity->flags |= EntityFlag_PropagatesSim;
            entity->p = worldPos;
            entity->world = world;
            entity->friction = 10.0f;
            entity->currentChunk = chunk->p;
            chunk->simPropagationCount++;
            if (chunk->active) {
                RegisterEntity(world, entity);
            }
        }
    }
    return entity;
}

Entity* GetEntity(GameWorld* world, EntityID id) {
    timed_scope();
    Entity* result = nullptr;
    auto ptr = Get(&world->entityHashMap, &id);
    if (ptr) {
        result = *ptr;
    }
    return result;
}

// TODO: There is no way now for dirty entities to figure out who caused an neighbor update
// So an entity which if set to dirty can not post neighborhood update itself because it will cause
// spinlock. We probably need to neighborhood update to be event-based or smth if we need dirty entities to
// post neighborhood updates
void PostEntityNeighborhoodUpdate(GameWorld* world, BlockEntity* entity) {
    timed_scope();
    iv3 min = entity->p - IV3(1);
    iv3 max = entity->p + IV3(1);

    for (i32 z = min.z; z <= max.z; z++) {
        for (i32 y = min.y; y <= max.y; y++) {
            for (i32 x = min.x; x <= max.x; x++) {
                BlockEntity* neighbor = GetBlockEntity(world, IV3(x, y, z));
                if (neighbor && (neighbor != entity)) {
                    entity->dirtyNeighborhood = true;
                }
            }
        }
    }
}

template <typename T>
T* AddBlockEntity(GameWorld* world, iv3 p) {
    timed_scope();
    // TODO: Validate position
    T* entity = nullptr;
    auto chunkP = WorldPos::ToChunk(p);
    auto chunk = GetChunk(world, chunkP.chunk);
    if (chunk) {
        auto block = GetBlock(chunk, chunkP.block);
        if (!IsBlockCollider(&block) && (block.entity == nullptr)) {
            entity = (T*)PlatformAlloc(sizeof(T), alignof(T), nullptr);
            if (entity) {
                ClearMemory(entity);
                EntityStorageInsert(&chunk->entityStorage, entity);
                entity->id = GenEntityID(world, EntityKind::Block);
                entity->kind = EntityKind::Block;
                entity->p = p;
                entity->flags |= EntityFlag_PropagatesSim;
                entity->world = world;
                auto occupied = OccupyBlock(chunk, entity, chunkP.block);
                assert(occupied);
                if (chunk->active) {
                    RegisterEntity(world, entity);
                }
                PostEntityNeighborhoodUpdate(world, entity);
            }
        }
    }
    return entity;
}

void DeleteEntity(GameWorld* world, Entity* entity) {
    timed_scope();
    auto entityInfo = GetEntityInfo(entity->type);
    if (entityInfo->Delete) {
        entityInfo->Delete(entity);
    }

    Chunk* chunk;
    switch (entity->kind) {
    case EntityKind::Block: {
        auto blockEntity = (BlockEntity*)entity;
        PostEntityNeighborhoodUpdate(world, blockEntity);
        auto chunkP = WorldPos::ToChunk(blockEntity->p);
        chunk = GetChunk(world, chunkP.chunk);
        assert(chunk);
        auto released = ReleaseBlock(chunk, blockEntity, chunkP.block);
        assert(released);
    } break;
    case EntityKind::Spatial: {
        auto spatialEntity = (SpatialEntity*)entity;
        iv3 chunkP;
        if (spatialEntity->outsideOfTheWorld) {
            chunkP = spatialEntity->currentChunk;
        } else {
            chunkP = WorldPos::ToChunk(spatialEntity->p).chunk;
        }
        chunk = GetChunk(world, chunkP);
        if (spatialEntity->flags & EntityFlag_PropagatesSim) {
            assert(chunk->simPropagationCount > 0);
            chunk->simPropagationCount--;
        }
        assert(chunk);
    } break;
    invalid_default();
    }

    UnregisterEntity(world, entity->id);
    EntityStorageUnlink(&chunk->entityStorage, entity);
    PlatformFree(entity, nullptr);
}

void ScheduleEntityForDelete(GameWorld* world, Entity* entity) {
    auto entry = BucketArrayPush(&world->entitiesToDelete);
    assert(entry);
    *entry = entity;
    entity->deleted = true;
}

bool CheckWorldBounds(WorldPos p) {
    bool result = false;
    if (p.block.y <= GameWorld::MaxHeight && p.block.y >= GameWorld::MinHeight) {
        result = true;
    }
    return result;
}

bool CheckWorldBounds(ChunkPos p) {
    bool result = false;
    if (p.chunk.y <= GameWorld::MaxHeightChunk && p.chunk.y >= GameWorld::MinHeightChunk) {
        result = true;
    }
    return result;
}

// TODO: Check is entity should be moved only in this function
// stop checking it in UpdateEntityResidence
bool EntityShouldBeMovedIntoAnotherChunk(SpatialEntity* entity) {
    auto result = false;
    auto inWorldBounds = CheckWorldBounds(entity->p);
    auto residenceChunkP = entity->currentChunk;
    auto currentP = WorldPos::ToChunk(entity->p).chunk;
    if (!inWorldBounds || (residenceChunkP != currentP)) {
        result = true;
    }
    return result;
}

bool UpdateEntityResidence(GameWorld* world, SpatialEntity* entity) {
    timed_scope();
    bool changedResidence = false;

    auto inWorldBounds = CheckWorldBounds(entity->p);
    if (!inWorldBounds) {
        entity->outsideOfTheWorld = true;
        auto info = GetEntityInfo(entity->type);
        if (!(entity->flags & EntityFlag_DisableDeleteWhenOutsideOfWorldBounds)) {
            ScheduleEntityForDelete(world, entity);
            log_print("[World] Entity %llu of type %s was moved outside of world bounds and will be deleted\n", entity->id, info->name);
        } else {
            log_print("[World] Entity %llu of type %s was moved outside of world bounds and won't be deleted because DisableDeleteWhenOutsideOfWorldBounds flag is set\n", entity->id, info->name);
        }
    } else {
        entity->outsideOfTheWorld = false;
        auto residenceChunkP = entity->currentChunk;
        auto currentP = WorldPos::ToChunk(entity->p).chunk;
        if (residenceChunkP != currentP) {
            auto oldChunk = GetChunk(world, residenceChunkP);
            assert(oldChunk);
            auto newChunk = GetChunk(world, currentP.x, currentP.y, currentP.z);
            // TODO: Just asserting for now. Should do something smart here.
            // Entity may move to the chunk which is generated yet (i.e. outside of a region).
            // So we need handle this situation somehow. Just scheduling chunk gen will not solve this problem
            // because we will need to wait somehow them. Maybe just discard whole frame movement for entity
            // is actually ok since spatial entities won't have complicated behavior and movements. Of the will?
            assert(newChunk);

            if (entity->flags & EntityFlag_PropagatesSim) {
                assert(oldChunk->simPropagationCount > 0);
                oldChunk->simPropagationCount--;
                newChunk->simPropagationCount++;
            }

            EntityStorageUnlink(&oldChunk->entityStorage, entity);
            EntityStorageInsert(&newChunk->entityStorage, entity);
            changedResidence = true;
            entity->currentChunk = newChunk->p;
            log_print("[World] Entity %lu changed it's residence (%ld, %ld, %ld) -> (%ld, %ld, %ld)\n", entity->id, oldChunk->p.x, oldChunk->p.y, oldChunk->p.z, newChunk->p.x, newChunk->p.y, newChunk->p.z);
        }
    }

    return changedResidence;
}

void FindOverlapsFor(GameWorld* world, SpatialEntity* entity) {
    timed_scope();
    WorldPos origin = entity->p;

    bool overlaps = false;
    bool resolved = true;

    v3 colliderSize = V3(entity->scale);
    v3 colliderRadius = colliderSize * 0.5f;

    iv3 bboxMin = WorldPos::Offset(origin, -colliderRadius).block - IV3(1);
    iv3 bboxMax = WorldPos::Offset(origin, colliderRadius).block + IV3(1);

    auto minChunk = WorldPos::ToChunk(bboxMin).chunk;
    auto maxChunk = WorldPos::ToChunk(bboxMax).chunk;

    for (i32 z = minChunk.z; z <= maxChunk.z; z++) {
        for (i32 y = minChunk.y; y <= maxChunk.y; y++) {
            for (i32 x = minChunk.x; x <= maxChunk.x; x++) {
                auto chunk = GetChunk(world, x, y, z);
                if (chunk) {
                    ForEach(&chunk->entityStorage, [&] (Entity* it){
                            // TODO: For each spatial entity
                            if (it->kind == EntityKind::Spatial) {
                                auto overlapped = static_cast<SpatialEntity*>(it);
                                if (overlapped->id != entity->id) {
                                    v3 relativePos = WorldPos::Relative(entity->p, overlapped->p);
                                    v3 testSize = V3(overlapped->scale);
                                    v3 testRadius = colliderSize * 0.5f;
                                    v3 testMin = relativePos - testRadius - colliderRadius;
                                    v3 testMax = relativePos + testRadius + colliderRadius;
                                    v3 bary = GetBarycentric(testMin, testMax, V3(0.0f));
                                    // TODO: Inside bary
                                    if ((bary.x > 0.0f && bary.x <= 1.0f) &&
                                        (bary.y > 0.0f && bary.y <= 1.0f) &&
                                        (bary.z > 0.0f && bary.z <= 1.0f)) {
                                        auto entityInfo = GetEntityInfo(entity->type);
                                        assert(entityInfo->kind == EntityKind::Spatial);
                                        entityInfo->ProcessOverlap(world, entity, overlapped);
                                    }
                                }
                            }
                        });
                }
            }
        }
    }
}

struct OverlapResolveResult {
    bool wasOverlapped;
    bool resolved;
};

OverlapResolveResult TryResolveOverlaps(GameWorld* world, SpatialEntity* entity) {
    timed_scope();
    assert(entity->kind == EntityKind::Spatial);
    WorldPos origin = entity->p;

    bool overlaps = false;
    bool resolved = true;

    v3 colliderSize = V3(entity->scale);
    v3 colliderRadius = colliderSize * 0.5f;

    iv3 bboxMin = WorldPos::Offset(origin, -colliderRadius).block - IV3(1);
    iv3 bboxMax = WorldPos::Offset(origin, colliderRadius).block + IV3(1);

    for (i32 z = bboxMin.z; z <= bboxMax.z; z++) {
        for (i32 y = bboxMin.y; y <= bboxMax.y; y++) {
            for (i32 x = bboxMin.x; x <= bboxMax.x; x++) {
#if 0
                v3 min = RelativePos(camera->targetWorldPosition, WorldPos::Make(bboxMin));
                v3 max = RelativePos(camera->targetWorldPosition, WorldPos::Make(bboxMax));
                DrawAlignedBoxOutline(renderGroup, min, max, V3(1.0f, 0.0f, 0.0f), 3.0f);
#endif

                // TODO: Handle case when entity is outside of world bounds
                if (y >= GameWorld::MinHeight && y <= GameWorld::MaxHeight) {
                    auto testBlock = GetBlock(world, x, y, z);
                    bool collides = IsBlockCollider(&testBlock);
                    if (collides) {
                        v3 relOrigin = WorldPos::Relative(WorldPos::Make(IV3(x, y, z)), origin);
                        v3 minCorner = V3(-Globals::BlockHalfDim);
                        v3 maxCorner = V3(Globals::BlockHalfDim);
                        // NOTE: Minkowski sum
                        minCorner += colliderSize * -0.5f;
                        maxCorner += colliderSize * 0.5f;
                        v3 bary = GetBarycentric(minCorner, maxCorner, relOrigin);
                        if ((bary.x > 0.0f && bary.x <= 1.0f) &&
                            (bary.y > 0.0f && bary.y <= 1.0f) &&
                            (bary.z > 0.0f && bary.z <= 1.0f)) {

                            overlaps = true;
                            //log_print("PENETRATION DETECTED!!! at frame %llu\n", GlobalPlatform.tickCount);

                            iv3 penetratedBlock = IV3(x, y, z);
                            bool hasFreeNeighbor = false;
                            f32 closestNeighborDist = F32::Max;
                            iv3 closestNeighborBlock = IV3(0);
                            v3 closestNeighborRelOrigin = {};
                            for (i32 pz = penetratedBlock.z - 1; pz <= penetratedBlock.z + 1; pz++) {
                                for (i32 py = penetratedBlock.y - 1; py <= penetratedBlock.y + 1; py++) {
                                    for (i32 px = penetratedBlock.x - 1; px <= penetratedBlock.x + 1; px++) {
                                        auto neighborBlock = GetBlock(world, px, py, pz);
                                        if (!IsBlockCollider(&neighborBlock)) {
                                            hasFreeNeighbor = true;
                                            v3 neighborRelOrigin = WorldPos::Relative(WorldPos::Make(IV3(x, y, z)), WorldPos::Make(IV3(px, py, pz)));
                                            f32 dist = LengthSq(neighborRelOrigin - relOrigin);
                                            if (dist < closestNeighborDist) {
                                                closestNeighborDist = dist;
                                                closestNeighborBlock = IV3(px, py, pz);
                                                closestNeighborRelOrigin = neighborRelOrigin;
                                            }
                                        }
                                    }
                                }
                            }

                            if (hasFreeNeighbor) {
                                // TODO: this is temporary hack
                                // Wee need an actual way to compute this coordinate
                                v3 relNewOrigin = Normalize(closestNeighborRelOrigin + relOrigin) * Globals::BlockDim + F32::Eps;
                                origin = WorldPos::Offset(WorldPos::Make(IV3(x, y, z)), relNewOrigin);
                            } else {
                                resolved = false;
                            }
                        }
                    }
                } else { // Above max world height or below min world height
                }
            }
        }
    }

    if (overlaps && resolved) {
        entity->p = origin;
    }

    OverlapResolveResult result = { overlaps, resolved };

    return result;
}


void MoveSpatialEntity(GameWorld* world, SpatialEntity* entity, v3 delta, Camera* camera, RenderGroup* renderGroup) {
    timed_scope();

    auto overlapResolveResult = TryResolveOverlaps(world, entity);
#if 0
    if (overlapResolveResult.wasOverlapped) {
        log_print("Entity was overlapping at frame %d. %s\n", GlobalPlatform.tickCount, overlapResolveResult.resolved ? "Resolved" : "Stuck");
    }
#endif
    if (overlapResolveResult.resolved) {
        // TODO: Better ground hit detection
        entity->grounded = false;

        v3 colliderSize = V3(entity->scale);
        v3 colliderRadius = colliderSize * 0.5f;

        WorldPos origin = entity->p;
        v3 velocity = entity->velocity;

        for (u32 pass = 0; pass < 4; pass++) {
            bool hit = false;
            f32 tMin = 1.0f;
            v3 hitNormal = {};

            auto target = WorldPos::Offset(origin, delta);
            iv3 originBoxMin = WorldPos::Offset(origin, -colliderRadius).block - IV3(1);
            iv3 originBoxMax = WorldPos::Offset(origin, colliderRadius).block + IV3(1);
            iv3 targetBoxMin = WorldPos::Offset(target, -colliderRadius).block - IV3(1);
            iv3 targetBoxMax = WorldPos::Offset(target, colliderRadius).block + IV3(1);

            iv3 minB = IV3(Min(originBoxMin.x, targetBoxMin.x) , Min(originBoxMin.y, targetBoxMin.y), Min(originBoxMin.z, targetBoxMin.z));
            iv3 maxB = IV3(Max(originBoxMax.x, targetBoxMax.x), Max(originBoxMax.y, targetBoxMax.y), Max(originBoxMax.z, targetBoxMax.z));
#if 0
            v3 min = RelativePos(camera->targetWorldPosition, WorldPos::Make(minB));
            v3 max = RelativePos(camera->targetWorldPosition, WorldPos::Make(maxB));
            // min -= V3(Globals::BlockHalfDim);
            //max -= V3(Globals::BlockHalfDim);
            DrawAlignedBoxOutline(renderGroup, min, max, V3(0.0f, 0.0f, 1.0f), 2.0f);
#endif
            for (i32 z = minB.z; z <= maxB.z; z++) {
                for (i32 y = minB.y; y <= maxB.y; y++) {
                    for (i32 x = minB.x; x <= maxB.x; x++) {
                        if (y >= GameWorld::MinHeight && y <= GameWorld::MaxHeight) {
                            // TODO: Cache chunk pointer
                            auto testBlock = GetBlock(world, x, y, z);
                            bool collides = IsBlockCollider(&testBlock);
                            if (collides) {
                                v3 relOrigin = WorldPos::Relative(WorldPos::Make(IV3(x, y, z)), origin);
                                v3 minCorner = V3(-Globals::BlockHalfDim);
                                v3 maxCorner = V3(Globals::BlockHalfDim);
                                // NOTE: Minkowski sum
                                minCorner += colliderSize * -0.5f;
                                maxCorner += colliderSize * 0.5f;
                                BBoxAligned testBox;
                                testBox.min = minCorner;
                                testBox.max = maxCorner;

                                auto intersection = Intersect(testBox, relOrigin, delta, 0.0f, F32::Max);
                                if (intersection.hit) {
                                    //log_print("hit something at frame %llu\n", GlobalPlatform.tickCount);
                                    f32 tHit =  Max(0.0f, intersection.t - 0.001f);
                                    if (tHit < tMin) {
                                        tMin = tHit;
                                        hitNormal = intersection.normal;
                                        hit = true;
                                    }
                                }
                            }
                        }
                    }
                }
            }

            origin = WorldPos::Offset(origin, delta * tMin);
            //hitNormal = -hitNormal;

            if (hit) {
                if (hitNormal.y == 1.0f) {
                    entity->grounded = true;
                }
                delta = WorldPos::Difference(target, origin);
                velocity -= Dot(velocity, hitNormal) * hitNormal;
                delta -= Dot(delta, hitNormal) * hitNormal;

            } else {
                break;
            }
        }
        entity->p = origin;
        entity->velocity = velocity;
    }
}

void ConvertBlockToPickup(GameWorld* world, iv3 voxelP) {
    auto chunkPos = WorldPos::ToChunk(voxelP);
    auto chunk = GetChunk(world, chunkPos.chunk.x, chunkPos.chunk.y, chunkPos.chunk.z);
    auto block = GetBlock(chunk, chunkPos.block.x, chunkPos.block.y, chunkPos.block.z);

    if (block.value != BlockValue::Empty) {
        auto blockInfo = GetBlockInfo(block.value);
        if (blockInfo->DropPickup) {
            blockInfo->DropPickup(&block, world, WorldPos::Make(voxelP));
        }
        auto blockToModify = GetBlockForModification(chunk, chunkPos.block.x, chunkPos.block.y, chunkPos.block.z);
        *blockToModify = BlockValue::Empty;
    }

    if (block.entity) {
        auto entityInfo = GetEntityInfo(block.entity->type);
        if (entityInfo->DropPickup) {
            entityInfo->DropPickup(block.entity, world, WorldPos::Make(voxelP));
        }
        ScheduleEntityForDelete(world, block.entity);
    }
}


bool SetBlockEntityPos(GameWorld* world, BlockEntity* entity, iv3 newP) {
    bool moved = false;
    bool inWorldBounds = CheckWorldBounds(WorldPos::Make(newP));
    if (inWorldBounds) {
        if (newP != entity->p) {
            // TODO: Do not post neighborhood update if entity is not moved
            // Posting on an old location
            PostEntityNeighborhoodUpdate(world, entity);
            auto newChunkP = WorldPos::ToChunk(newP);
            auto oldChunkP = WorldPos::ToChunk(entity->p);
            auto oldChunk = GetChunk(world, oldChunkP.chunk);
            auto newChunk = GetChunk(world, newChunkP.chunk);
            assert(oldChunk);
            // TODO: Just asserting for now. Should do something smart here.
            // Entity may move to the chunk which is generated yet (i.e. outside of a region).
            // So we need handle this situation somehow. Just scheduling chunk gen will not solve this problem
            // because we will need to wait somehow them. Maybe just discard whole frame movement for entity
            // is actually ok since spatial entities won't have complicated behavior and movements. Of the will?
            // see TODO in UpdateEntityResidence
            assert(newChunk);
            auto occupied = OccupyBlock(newChunk, entity, newChunkP.block);

            if (occupied) {
                auto released = ReleaseBlock(oldChunk, entity, oldChunkP.block);
                assert(released);
                EntityStorageUnlink(&oldChunk->entityStorage, entity);
                EntityStorageInsert(&newChunk->entityStorage, entity);
                log_print("[World] Block entity %lu changed it's residence (%ld, %ld, %ld) -> (%ld, %ld, %ld)\n", entity->id, oldChunk->p.x, oldChunk->p.y, oldChunk->p.z, newChunk->p.x, newChunk->p.y, newChunk->p.z);
                entity->p = newP;
                moved = true;
                // Posting on a new location
                PostEntityNeighborhoodUpdate(world, entity);
            }
        }
    }
    return moved;
}

bool BuildBlock(Context* context, GameWorld* world, iv3 p, Item item) {
    bool result = false;
    auto itemInfo = GetItemInfo(&context->entityInfo, (u32)item);
    if (itemInfo->convertsToBlock) {
        auto blockValue = itemInfo->associatedBlock;
        auto chunkPos = WorldPos::ToChunk(p);
        auto chunk = GetChunk(world, chunkPos.chunk.x, chunkPos.chunk.y, chunkPos.chunk.z);
        if (chunk) {
            auto block = GetBlockForModification(chunk, chunkPos.block.x, chunkPos.block.y, chunkPos.block.z);
            assert(block);
            *block = blockValue;
            result = true;
        }
    } else {
        auto entityInfo = GetEntityInfo(&context->entityInfo, itemInfo->associatedEntityTypeID);
        if (entityInfo->typeID != (u32)EntityType::Unknown) {
            Entity* entity = entityInfo->Create(world, WorldPos::Make(p));
            if (entity) {
                result = true;
            }
        }
    }
    return result;
}

void RegisterEntity(GameWorld* world, Entity* entity) {
    timed_scope();
    auto entry = Add(&world->entityHashMap, &entity->id);
    assert(entry);
    *entry = entity;
}

bool UnregisterEntity(GameWorld* world, EntityID id) {
    timed_scope();
    bool result = Delete(&world->entityHashMap, &id);
    return result;
}
