#include "World.h"
#include "Renderer.h"
#include "Pickup.h"
#include "Container.h"

// TODO: Move it out of here
void CoalOreDropPickup(const Voxel* voxel, GameWorld* world, WorldPos p) {
    RandomSeries series = {};
    for (u32 i = 0; i < 4; i++) {
        auto entity = (Pickup*)CreatePickupEntity(world, p);
        if (entity) {
            entity->item = Item::CoalOre;
            entity->count = 1;
            entity->meshScale = V3(1.0f);
            auto spatial = static_cast<SpatialEntity*>(entity);
            v3 randomOffset = V3(RandomUnilateral(&series) - 0.5f, RandomUnilateral(&series) - 0.5f, RandomUnilateral(&series) - 0.5f);
            spatial->p = WorldPos::Make(p.block, randomOffset);
        }
    }
};


Voxel* GetVoxelRaw(Chunk* chunk, u32 x, u32 y, u32 z) {
    Voxel* result = &chunk->voxels[x + Chunk::Size * y + Chunk::Size * Chunk::Size * z];
    return result;
}

const Voxel* GetVoxel(Chunk* chunk, u32 x, u32 y, u32 z) {
    const Voxel* result = nullptr;
    if (x < Chunk::Size && y < Chunk::Size && z < Chunk::Size) {
        result = GetVoxelRaw(chunk, x, y, z);
    } else {
        result = &chunk->nullVoxel;
    }
    return result;
}

const Voxel* GetVoxel(GameWorld* world, i32 x, i32 y, i32 z) {
    const Voxel* result = nullptr;
    auto chunkP = WorldPos::ToChunk(IV3(x, y, z));
    Chunk* chunk = GetChunk(world, chunkP.chunk.x, chunkP.chunk.y, chunkP.chunk.z);
    if (chunk) {
        result = GetVoxel(chunk, chunkP.block.x, chunkP.block.y, chunkP.block.z);
    } else {
        result = &world->nullVoxel;
    }
    return result;
}

bool OccupyVoxel(Chunk* chunk, Entity* entity, u32 x, u32 y, u32 z) {
    bool result = false;
    if (x < Chunk::Size && y < Chunk::Size && z < Chunk::Size) {
        auto voxel = GetVoxelRaw(chunk, x, y, z);
        assert(voxel);
        if (!voxel->entity) {
            voxel->entity = entity;
            result = true;
        }
    }
    return result;
}

bool ReleaseVoxel(Chunk* chunk, Entity* entity, u32 x, u32 y, u32 z) {
    bool result = false;
    if (x < Chunk::Size && y < Chunk::Size && z < Chunk::Size) {
        auto voxel = GetVoxelRaw(chunk, x, y, z);
        assert(voxel);
        // Only entity that lives here allowed to release voxel
        assert(voxel->entity->id == entity->id);
        if (voxel->entity->id == entity->id) {
            voxel->entity = nullptr;
            result = true;
        }
    }
    return result;
}

Voxel* GetVoxelForModification(Chunk* chunk, u32 x, u32 y, u32 z) {
    Voxel* result = nullptr;
    if (x < Chunk::Size && y < Chunk::Size && z < Chunk::Size) {
        result = GetVoxelRaw(chunk, x, y, z);
        chunk->shouldBeRemeshedAfterEdit = true;
    }
    return result;
}

Chunk* AddChunk(GameWorld* world, iv3 coord) {
    auto chunk = (Chunk*)PlatformAlloc(sizeof(Chunk), 0, nullptr);
    ClearMemory(chunk);
    chunk->entityStorage.Init(MakeAllocator(PlatformAlloc, PlatformFree, nullptr));
    chunk->p = coord;
    chunk->priority = ChunkPriority::Low;
    chunk->world = world;
    auto entry = Add(&world->chunkHashMap, &chunk->p);
    assert(entry);
    *entry = chunk;
    return chunk;
}

Chunk* GetChunk(GameWorld* world, i32 x, i32 y, i32 z) {
    Chunk* result = nullptr;
    iv3 key = IV3(x, y, z);
    Chunk** entry = Get(&world->chunkHashMap, &key);
    if (entry) {
        result = *entry;
    }
    return result;
}

void InitWorld(GameWorld* world, Context* context, ChunkMesher* mesher, u32 seed) {
    world->chunkHashMap = HashMap<iv3, Chunk*, ChunkHashFunc, ChunkHashCompFunc>::Make();
    world->context = context;
    world->mesher = mesher;
    world->worldGen.Init(seed);
    BucketArrayInit(&world->entitiesToDelete, MakeAllocator(PlatformAlloc, PlatformFree, nullptr));
}

template <typename T>
T* AddSpatialEntity(GameWorld* world, WorldPos p) {
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
            chunk->entityStorage.Insert(entity);
            entity->id = GenEntityID(chunk->world, EntityKind::Spatial);
            entity->kind = EntityKind::Spatial;
            entity->p = worldPos;
            entity->world = world;
            entity->friction = 10.0f;
            entity->currentChunk = chunk->p;
            if (chunk->region) {
                RegisterEntity(chunk->region, entity);
            }
        }
    }
    return entity;
}

Entity* GetEntity(GameWorld* world, iv3 p) {
    Entity* result = nullptr;
    const Voxel* voxel = GetVoxel(world, p);
    if (voxel) {
        result = voxel->entity;
    }
    return result;
}

// TODO: There is no way now for dirty entities to figure out who caused an neighbor update
// So an entity which if set to dirty can not post neighborhood update itself because it will cause
// spinlock. We probably need to neighborhood update to be event-based or smth if we need dirty entities to
// post neighborhood updates
void PostEntityNeighborhoodUpdate(GameWorld* world, BlockEntity* entity) {
    iv3 min = entity->p - IV3(1);
    iv3 max = entity->p + IV3(1);

    for (i32 z = min.z; z <= max.z; z++) {
        for (i32 y = min.y; y <= max.y; y++) {
            for (i32 x = min.x; x <= max.x; x++) {
                Entity* neighbor = GetEntity(world, IV3(x, y, z));
                if (neighbor && (neighbor != entity) && (neighbor->kind == EntityKind::Block)) {
                    auto e = static_cast<BlockEntity*>(neighbor);
                    e->dirtyNeighborhood = true;
                }
            }
        }
    }
}

template <typename T>
T* AddBlockEntity(GameWorld* world, iv3 p) {
    T* entity = nullptr;
    auto chunkP = WorldPos::ToChunk(p);
    auto chunk = GetChunk(world, chunkP.chunk);
    if (chunk) {
        auto voxel = GetVoxel(chunk, chunkP.block);
        if (voxel) {
            if (!IsVoxelCollider(voxel) && (voxel->entity == nullptr)) {
                entity = (T*)PlatformAlloc(sizeof(T), alignof(T), nullptr);
                if (entity) {
                    ClearMemory(entity);
                    chunk->entityStorage.Insert(entity);
                    entity->id = GenEntityID(chunk->world, EntityKind::Block);
                    entity->kind = EntityKind::Block;
                    entity->p = p;
                    entity->world = world;
                    auto occupied = OccupyVoxel(chunk, entity, chunkP.block);
                    assert(occupied);
                    if (chunk->region) {
                        RegisterEntity(chunk->region, entity);
                    }
                    PostEntityNeighborhoodUpdate(world, entity);
                }
            }
        }
    }
    return entity;
}

void DeleteEntity(GameWorld* world, Entity* entity) {
    auto entityInfo = GetEntityInfo(entity->type);
    if (entityInfo->Delete) {
        entityInfo->Delete(entity, world);
    }

    Chunk* chunk;
    switch (entity->kind) {
    case EntityKind::Block: {
        auto blockEntity = (BlockEntity*)entity;
        PostEntityNeighborhoodUpdate(world, blockEntity);
        auto chunkP = WorldPos::ToChunk(blockEntity->p);
        chunk = GetChunk(world, chunkP.chunk);
        assert(chunk);
        auto released = ReleaseVoxel(chunk, blockEntity, chunkP.block);
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
        assert(chunk);
    } break;
    invalid_default();
    }

    UnregisterEntity(chunk->region, entity->id);
    if (entity->inventory) {
        DeleteEntityInventory(entity->inventory);
    }
    chunk->entityStorage.Unlink(entity);
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

bool UpdateEntityResidence(GameWorld* world, SpatialEntity* entity) {
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

            oldChunk->entityStorage.Unlink(entity);
            newChunk->entityStorage.Insert(entity);
            changedResidence = true;
            entity->currentChunk = newChunk->p;
            log_print("[World] Entity %lu changed it's residence (%ld, %ld, %ld) -> (%ld, %ld, %ld)\n", entity->id, oldChunk->p.x, oldChunk->p.y, oldChunk->p.z, newChunk->p.x, newChunk->p.y, newChunk->p.z);
        }
    }

    return changedResidence;
}

bool IsVoxelCollider(const Voxel* voxel) {
    bool hasColliderTerrain = true;
    bool hasColliderEntity = false;
    if (voxel->entity && (voxel->entity->flags & EntityFlag_Collides)) {
        hasColliderEntity = true;
    }
    if (voxel && (voxel->value == VoxelValue::Empty)) {
        hasColliderTerrain = false;
    }

    return hasColliderTerrain || hasColliderEntity;
}

void FindOverlapsFor(GameWorld* world, SpatialEntity* entity) {
    auto context = GetContext();
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
                    foreach (chunk->entityStorage) {
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
                    }
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
                    auto testVoxel = GetVoxel(world, x, y, z);
                    bool collides = IsVoxelCollider(testVoxel);
                    if (collides) {
                        v3 relOrigin = WorldPos::Relative(WorldPos::Make(IV3(x, y, z)), origin);
                        v3 minCorner = V3(-Voxel::HalfDim);
                        v3 maxCorner = V3(Voxel::HalfDim);
                        // NOTE: Minkowski sum
                        minCorner += colliderSize * -0.5f;
                        maxCorner += colliderSize * 0.5f;
                        v3 bary = GetBarycentric(minCorner, maxCorner, relOrigin);
                        if ((bary.x > 0.0f && bary.x <= 1.0f) &&
                            (bary.y > 0.0f && bary.y <= 1.0f) &&
                            (bary.z > 0.0f && bary.z <= 1.0f)) {

                            overlaps = true;
                            //log_print("PENETRATION DETECTED!!! at frame %llu\n", GlobalPlatform.tickCount);

                            iv3 penetratedVoxel = IV3(x, y, z);
                            bool hasFreeNeighbor = false;
                            f32 closestNeighborDist = F32::Max;
                            iv3 closestNeighborVoxel = IV3(0);
                            v3 closestNeighborRelOrigin = {};
                            for (i32 pz = penetratedVoxel.z - 1; pz <= penetratedVoxel.z + 1; pz++) {
                                for (i32 py = penetratedVoxel.y - 1; py <= penetratedVoxel.y + 1; py++) {
                                    for (i32 px = penetratedVoxel.x - 1; px <= penetratedVoxel.x + 1; px++) {
                                        auto neighborVoxel = GetVoxel(world, px, py, pz);
                                        if (!IsVoxelCollider(neighborVoxel)) {
                                            hasFreeNeighbor = true;
                                            v3 neighborRelOrigin = WorldPos::Relative(WorldPos::Make(IV3(x, y, z)), WorldPos::Make(IV3(px, py, pz)));
                                            f32 dist = LengthSq(neighborRelOrigin - relOrigin);
                                            if (dist < closestNeighborDist) {
                                                closestNeighborDist = dist;
                                                closestNeighborVoxel = IV3(px, py, pz);
                                                closestNeighborRelOrigin = neighborRelOrigin;
                                            }
                                        }
                                    }
                                }
                            }

                            if (hasFreeNeighbor) {
                                // TODO: this is temporary hack
                                // Wee need an actual way to compute this coordinate
                                v3 relNewOrigin = Normalize(closestNeighborRelOrigin + relOrigin) * Voxel::Dim + F32::Eps;
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

    auto overlapResolveResult = TryResolveOverlaps(world, entity);

    if (overlapResolveResult.wasOverlapped) {
        log_print("Entity was overlapping at frame %d. %s\n", GlobalPlatform.tickCount, overlapResolveResult.resolved ? "Resolved" : "Stuck");
    }

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
            // min -= V3(Voxel::HalfDim);
            //max -= V3(Voxel::HalfDim);
            DrawAlignedBoxOutline(renderGroup, min, max, V3(0.0f, 0.0f, 1.0f), 2.0f);
#endif
            for (i32 z = minB.z; z <= maxB.z; z++) {
                for (i32 y = minB.y; y <= maxB.y; y++) {
                    for (i32 x = minB.x; x <= maxB.x; x++) {
                        if (y >= GameWorld::MinHeight && y <= GameWorld::MaxHeight) {
                            // TODO: Cache chunk pointer
                            auto testVoxel = GetVoxel(world, x, y, z);
                            bool collides = IsVoxelCollider(testVoxel);
                            if (collides) {
                                v3 relOrigin = WorldPos::Relative(WorldPos::Make(IV3(x, y, z)), origin);
                                v3 minCorner = V3(-Voxel::HalfDim);
                                v3 maxCorner = V3(Voxel::HalfDim);
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
    auto voxel = GetVoxel(chunk, chunkPos.block.x, chunkPos.block.y, chunkPos.block.z);

    auto context = GetContext();

    if (voxel->value != VoxelValue::Empty) {
        auto blockInfo = GetBlockInfo(voxel->value);
        blockInfo->DropPickup(voxel, world, WorldPos::Make(voxelP));
        auto voxelToModify = GetVoxelForModification(chunk, chunkPos.block.x, chunkPos.block.y, chunkPos.block.z);
        voxelToModify->value = VoxelValue::Empty;
    }

    if (voxel->entity) {
        auto entityInfo = GetEntityInfo(voxel->entity->type);
        entityInfo->DropPickup(voxel->entity, world, WorldPos::Make(voxelP));
        ScheduleEntityForDelete(world, voxel->entity);
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
            auto occupied = OccupyVoxel(newChunk, entity, newChunkP.block);

            if (occupied) {
                auto released = ReleaseVoxel(oldChunk, entity, oldChunkP.block);
                assert(released);
                oldChunk->entityStorage.Unlink(entity);
                newChunk->entityStorage.Insert(entity);
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
            auto voxel = GetVoxelForModification(chunk, chunkPos.block.x, chunkPos.block.y, chunkPos.block.z);
            assert(voxel);
            voxel->value = blockValue;
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
