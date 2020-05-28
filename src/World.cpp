#include "World.h"
#include "Renderer.h"



Voxel* GetVoxelRaw(Chunk* chunk, u32 x, u32 y, u32 z) {
    Voxel* result = &chunk->voxels[x + Chunk::Size * y + Chunk::Size * Chunk::Size * z];
    return result;
}

const Voxel* GetVoxel(Chunk* chunk, u32 x, u32 y, u32 z) {
    const Voxel* result = nullptr;
    if (x < Chunk::Size && y < Chunk::Size && z < Chunk::Size) {
        result = GetVoxelRaw(chunk, x, y, z);
    }
    return result;
}

const Voxel* GetVoxel(GameWorld* world, i32 x, i32 y, i32 z) {
    const Voxel* result = nullptr;
    auto chunkP = WorldPos::ToChunk(IV3(x, y, z));
    Chunk* chunk = GetChunk(world, chunkP.chunk.x, chunkP.chunk.y, chunkP.chunk.z);
    if (chunk) {
        result = GetVoxel(chunk, chunkP.block.x, chunkP.block.y, chunkP.block.z);
    }
    return result;
}

bool OccupyVoxel(Chunk* chunk, BlockEntity* entity, u32 x, u32 y, u32 z) {
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

bool ReleaseVoxel(Chunk* chunk, BlockEntity* entity, u32 x, u32 y, u32 z) {
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
    chunk->blockEntityStorage.Init(MakeAllocator(PlatformAlloc, PlatformFree, nullptr));
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
    BucketArrayInit(&world->blockEntitiesToDelete, MakeAllocator(PlatformAlloc, PlatformFree, nullptr));
}

// TODO: Are 64 bit literals supported on all compilers?
EntityKind ClassifyEntity(EntityID id) {
    bool spatial = id & 0x8000000000000000ull;
    if (spatial) {
        return EntityKind::Spatial;
    } else {
        return EntityKind::Block;
    }
}

EntityID GenEntityID(GameWorld* world, EntityKind kind) {
    assert(world->entitySerialCount < (U64::Max - 1));
    u64 mask = kind == EntityKind::Spatial ? 0x8000000000000000ull : 0ull;
    EntityID result = { (++world->entitySerialCount) | mask };
    ClassifyEntity(result);
    return result;
}

BlockEntity* AddSpatialEntity(GameWorld* world, iv3 p) {
    BlockEntity* entity = nullptr;
    auto chunkP = WorldPos::ToChunk(p).chunk;
    auto chunk = GetChunk(world, chunkP.x, chunkP.y, chunkP.z);
    if (chunk) {
        entity = chunk->blockEntityStorage.Add();
        if (entity) {
            entity->id = GenEntityID(chunk->world, EntityKind::Spatial);
            entity->entityClass = EntityClass::Spatial;
            entity->p = p;
            if (chunk->region) {
                RegisterEntity(chunk->region, entity);
            }
        }
    }
    return entity;
}

void DeleteSpatialEntity(GameWorld* world, BlockEntity* entity) {
    auto chunk = GetChunk(world, WorldPos::ToChunk(entity->p).chunk);
    assert(chunk);
    UnregisterEntity(chunk->region, entity->id);
    if (entity->inventory) {
        DeleteEntityInventory(entity->inventory);
    }
    chunk->blockEntityStorage.Remove(entity);
}

BlockEntity* GetBlockEntity(GameWorld* world, iv3 p) {
    const Voxel* voxel = GetVoxel(world, p);
    return voxel->entity;
}

void MakeBlockEntityNeighborhoodDirty(GameWorld* world, BlockEntity* entity) {
    iv3 min = entity->p - IV3(1);
    iv3 max = entity->p + IV3(1);

    for (i32 z = min.z; z <= max.z; z++) {
        for (i32 y = min.y; y <= max.y; y++) {
            for (i32 x = min.x; x <= max.x; x++) {
                BlockEntity* neighbor = GetBlockEntity(world, IV3(x, y, z));
                if (neighbor) {
                    neighbor->dirtyNeighborhood = true;
                }
            }
        }
    }
}

BlockEntity* AddBlockEntity(GameWorld* world, iv3 p) {
    BlockEntity* entity = nullptr;
    auto chunkP = WorldPos::ToChunk(p);
    auto chunk = GetChunk(world, chunkP.chunk);
    if (chunk) {
        auto voxel = GetVoxel(chunk, chunkP.block);
        if (voxel) {
            if (!IsVoxelCollider(voxel) && (voxel->entity == nullptr)) {
                entity = chunk->blockEntityStorage.Add();
                if (entity) {
                    entity->id = GenEntityID(chunk->world, EntityKind::Block);
                    entity->entityClass = EntityClass::Block;
                    entity->p = p;
                    auto occupied = OccupyVoxel(chunk, entity, chunkP.block);
                    assert(occupied);
                    MakeBlockEntityNeighborhoodDirty(world, entity);
                    if (chunk->region) {
                        RegisterEntity(chunk->region, entity);
                    }
                }
            }
        }
    }
    return entity;
}

// TODO Propper entity behaviors instead f dirty hack-in's
void BlockEntityDeleteBehavior(GameWorld* world, BlockEntity* entity) {
    switch (entity->type) {
        case BlockEntityType::Pipe: {
            MakeBlockEntityNeighborhoodDirty(world, entity);
        } break;
    default: {} break;
    }
}

void DeleteBlockEntity(GameWorld* world, BlockEntity* entity) {
    // TODO: Get chunk from region for speed?
    auto chunkP = WorldPos::ToChunk(entity->p);
    auto chunk = GetChunk(world, chunkP.chunk);
    assert(chunk);
    BlockEntityDeleteBehavior(world, entity);
    auto released = ReleaseVoxel(chunk, entity, chunkP.block);
    assert(released);
    UnregisterEntity(chunk->region, entity->id);
    if (entity->inventory) {
        DeleteEntityInventory(entity->inventory);
    }
    chunk->blockEntityStorage.Remove(entity);
}

void DeleteBlockEntityAfterThisFrame(GameWorld* world, BlockEntity* entity) {
    auto entry = BucketArrayPush(&world->blockEntitiesToDelete);
    assert(entry);
    *entry = entity;
    entity->deleted = true;
}


bool UpdateEntityResidence(GameWorld* world, BlockEntity* entity) {
    bool changedResidence = false;
    // TODO: Maybe it's not so fast to go through chunk pointer for every entity
    // Maybe we could just have a position from frame start and a position at frame end
    // and compare them?
    auto residenceChunkP = WorldPos::ToChunk(entity->p).chunk;
    auto currentP = WorldPos::ToChunk(entity->p).chunk;
    if (residenceChunkP != currentP) {
        // TODO: Just pass these pointers as agrs?
        auto oldChunk = GetChunk(world, residenceChunkP);
        assert(oldChunk);
        auto newChunk = GetChunk(world, currentP.x, currentP.y, currentP.z);
        // TODO: Just asserting for now. Should do something smart here.
        // Entity may move to the chunk which is generated yet (i.e. outside of a region).
        // So we need handle this situation somehow. Just scheduling chunk gen will not solve this problem
        // because we will need to wait somehow them. Maybe just discard whole frame movement for entity
        // is actually ok since spatial entities won't have complicated behavior and movements. Of the will?
        assert(newChunk);

        // Don't need to update hash map entry since pointer isn't changed
        //UnregisterSpatialEntity(region, entity->id);
        oldChunk->blockEntityStorage.Unlink(entity);
        newChunk->blockEntityStorage.Insert(entity);
        //RegisterSpatialEntity(region, newEntity);
        changedResidence = true;
        log_print("[World] Entity %lu changed it's residence (%ld, %ld, %ld) -> (%ld, %ld, %ld)\n", entity->id, oldChunk->p.x, oldChunk->p.y, oldChunk->p.z, newChunk->p.x, newChunk->p.y, newChunk->p.z);
    }
    return changedResidence;
}

bool IsVoxelCollider(const Voxel* voxel) {
    bool hasColliderTerrain = true;
    bool hasColliderEntity = false;
    if (voxel->entity && (voxel->entity->flags & BlockEntityFlag_Collides)) {
        hasColliderEntity = true;
    }
    if (voxel && (voxel->value == VoxelValue::Empty)) {
        hasColliderTerrain = false;
    }

    return hasColliderTerrain || hasColliderEntity;
}

void ProcessEntityOverlap(GameWorld* world, BlockEntity* entity, BlockEntity* overlapped) {
    switch (entity->type) {
    case BlockEntityType::Player: {
        if (overlapped->type == BlockEntityType::Pickup) {
            assert(entity->inventory);
            auto itemRemainder = EntityInventoryPushItem(entity->inventory, overlapped->pickupItem, overlapped->itemCount);
            if (itemRemainder == 0) {
                DeleteBlockEntityAfterThisFrame(world, overlapped);
            } else {
                overlapped->itemCount = itemRemainder;
            }
            //log_print("Player is overlapping with: %llu %s (%ld, %ld, %ld)\n", overlapped->id, ToString(overlapped->type), overlapped->p.block.x, overlapped->p.block.y, overlapped->p.block.z);
        }
    } break;
    invalid_default();
    }
}

void FindOverlapsFor(GameWorld* world, BlockEntity* entity) {
    WorldPos origin = WorldPos::Make(entity->p, entity->offset);

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
                    foreach (chunk->blockEntityStorage) {
                        if (it->entityClass == EntityClass::Spatial) {
                            if (it->id != entity->id) {
                                v3 relativePos = WorldPos::Relative(WorldPos::Make(entity->p), WorldPos::Make(it->p));
                                v3 testSize = V3(entity->scale);
                                v3 testRadius = colliderSize * 0.5f;
                                v3 testMin = relativePos - testRadius - colliderRadius;
                                v3 testMax = relativePos + testRadius + colliderRadius;
                                v3 bary = GetBarycentric(testMin, testMax, V3(0.0f));
                                // TODO: Inside bary
                                if ((bary.x > 0.0f && bary.x <= 1.0f) &&
                                    (bary.y > 0.0f && bary.y <= 1.0f) &&
                                    (bary.z > 0.0f && bary.z <= 1.0f)) {
                                    ProcessEntityOverlap(world, entity, it);
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

OverlapResolveResult TryResolveOverlaps(GameWorld* world, BlockEntity* entity) {
    assert(entity->entityClass == EntityClass::Spatial);
    WorldPos origin = WorldPos::Make(entity->p);

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
        entity->p = origin.block;
        entity->offset = origin.offset;
    }

    OverlapResolveResult result = { overlaps, resolved };

    return result;
}


void MoveSpatialEntity(GameWorld* world, BlockEntity* entity, v3 delta, Camera* camera, RenderGroup* renderGroup) {

    auto overlapResolveResult = TryResolveOverlaps(world, entity);

    if (overlapResolveResult.wasOverlapped) {
        log_print("Entity was overlapping at frame %d. %s\n", GlobalPlatform.tickCount, overlapResolveResult.resolved ? "Resolved" : "Stuck");
    }

    if (overlapResolveResult.resolved) {
        // TODO: Better ground hit detection
        entity->grounded = false;

        v3 colliderSize = V3(entity->scale);
        v3 colliderRadius = colliderSize * 0.5f;

        WorldPos origin = WorldPos::Make(entity->p, entity->offset);
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
        entity->p = origin.block;
        entity->offset = origin.offset;
        entity->velocity = velocity;
    }
}

void ConvertVoxelToPickup(GameWorld* world, iv3 voxelP) {
    auto chunkPos = WorldPos::ToChunk(voxelP);
    auto chunk = GetChunk(world, chunkPos.chunk.x, chunkPos.chunk.y, chunkPos.chunk.z);
    auto voxel = GetVoxelForModification(chunk, chunkPos.block.x, chunkPos.block.y, chunkPos.block.z);
    if (voxel->value == VoxelValue::CoalOre) {
        RandomSeries series = {};
        for (u32 i = 0; i < 4; i++) {
            auto entity = AddSpatialEntity(world, voxelP);
            if (entity) {
                v3 randomOffset = V3(RandomUnilateral(&series) - 0.5f, RandomUnilateral(&series) - 0.5f, RandomUnilateral(&series) - 0.5f);
                auto worldPos = WorldPos::Make(voxelP, randomOffset);
                entity->p = worldPos.block;
                entity->offset = worldPos.offset;
                    // TODO: SetEntityPos
                entity->scale = 0.2f;
                entity->type = BlockEntityType::Pickup;
                entity->pickupItem = Item::CoalOre;
                entity->itemCount = 1;
            }
        }
    }
    voxel->value = VoxelValue::Empty;
    if (voxel->entity) {
        DeleteBlockEntityAfterThisFrame(world, voxel->entity);
        switch (voxel->entity->type) {
        case BlockEntityType::Container: {
            auto entity = AddSpatialEntity(world, voxelP);
            if (entity) {
                // TODO: SetEntityPos
                entity->scale = 0.2f;
                entity->type = BlockEntityType::Pickup;
                entity->pickupItem = Item::Container;
                entity->itemCount = 1;
            }
        } break;
        case BlockEntityType::Pipe: {
            auto entity = AddSpatialEntity(world, voxelP);
            if (entity) {
                // TODO: SetEntityPos
                entity->scale = 0.2f;
                entity->type = BlockEntityType::Pickup;
                entity->pickupItem = Item::Pipe;
                entity->itemCount = 1;
            }
        } break;
        case BlockEntityType::Barrel: {
            auto entity = AddSpatialEntity(world, voxelP);
            if (entity) {
                // TODO: SetEntityPos
                entity->scale = 0.2f;
                entity->type = BlockEntityType::Pickup;
                entity->pickupItem = Item::Barrel;
                entity->itemCount = 1;
            }
        } break;
        default: {} break;
        }

    }
}

EntityInventory* AllocateEntityInventory(u32 slotCount, u32 slotCapacity) {
    // TODO: Joint allocation
    auto inventory = (EntityInventory*)PlatformAlloc(sizeof(EntityInventory) * slotCount, 0, nullptr);
    assert(inventory);
    auto slots = (InventorySlot*)PlatformAlloc(sizeof(InventorySlot) * slotCount, 0, nullptr);
    ClearArray(slots, slotCount);
    assert(slots);
    inventory->slots = slots;
    inventory->slotCount = slotCount;
    inventory->slotCapacity = slotCapacity;
    return inventory;
}

void DeleteEntityInventory(EntityInventory* inventory) {
    PlatformFree(inventory->slots, nullptr);
    PlatformFree(inventory, nullptr);
}

u32 EntityInventoryPushItem(EntityInventory* inventory, Item item, u32 count) {
    bool fitted = false;
    if (count > 0) {
        for (usize i = 0; i < inventory->slotCount; i++) {
            auto slot = inventory->slots + i;
            if (slot->item == Item::None || slot->item == item) {
                u32 slotFree = inventory->slotCapacity - slot->count;
                u32 amount = count <= slotFree ? count : slotFree;
                count = count - amount;
                slot->item = item;
                slot->count += amount;
                if (!count) break;
            }
        }
    }
    return count;
}

// TODO: Store world poninter in entity or smth?
bool SetBlockEntityPos(GameWorld* world, BlockEntity* entity, iv3 newP) {
    bool moved = false;
    // TODO validate coords
    //assert(newP)
    if (newP != entity->p) {
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
            oldChunk->blockEntityStorage.Unlink(entity);
            newChunk->blockEntityStorage.Insert(entity);
            log_print("[World] Block entity %lu changed it's residence (%ld, %ld, %ld) -> (%ld, %ld, %ld)\n", entity->id, oldChunk->p.x, oldChunk->p.y, oldChunk->p.z, newChunk->p.x, newChunk->p.y, newChunk->p.z);
            entity->p = newP;
            moved = true;
        }
    }
    return moved;
}

bool BuildBlock(Context* context, GameWorld* world, iv3 p, Item item) {
    bool result = false;
    auto blockValue = ItemToBlock(item);
    if (blockValue != VoxelValue::Empty) {
        auto chunkPos = WorldPos::ToChunk(p);
        auto chunk = GetChunk(world, chunkPos.chunk.x, chunkPos.chunk.y, chunkPos.chunk.z);
        auto voxel = GetVoxelForModification(chunk, chunkPos.block.x, chunkPos.block.y, chunkPos.block.z);
        voxel->value = blockValue;
        result = true;
    } else {
        auto type = ItemToBlockEntityType(item);
        if (type != BlockEntityType::Unknown) {
            // TODO: You know what!! Too many switch statements
            switch (type) {
            case BlockEntityType::Container: { result = (bool)CreateContainer(context, world, p); } break;
            case BlockEntityType::Pipe: { result = (bool)CreatePipe(context, world, p); } break;
            case BlockEntityType::Barrel: { result = (bool)CreateBarrel(context, world, p); } break;
            default: {} break;
            }
        }
    }
    return result;
}
