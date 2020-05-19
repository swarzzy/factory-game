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
    auto chunkP = ChunkPosFromWorldPos(IV3(x, y, z));
    Chunk* chunk = GetChunk(world, chunkP.chunk.x, chunkP.chunk.y, chunkP.chunk.z);
    if (chunk) {
        result = GetVoxel(chunk, chunkP.voxel.x, chunkP.voxel.y, chunkP.voxel.z);
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
    chunk->spatialEntityStorage.Init(MakeAllocator(PlatformAlloc, PlatformFree, nullptr));
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

void InitWorld(GameWorld* world, ChunkMesher* mesher, u32 seed) {
    world->chunkHashMap = HashMap<iv3, Chunk*, ChunkHashFunc, ChunkHashCompFunc>::Make();
    world->mesher = mesher;
    world->worldGen.Init(seed);
    BucketArrayInit(&world->spatialEntitiesToDelete, MakeAllocator(PlatformAlloc, PlatformFree, nullptr));
    BucketArrayInit(&world->blockEntitiesToDelete, MakeAllocator(PlatformAlloc, PlatformFree, nullptr));
}

// TODO: Are 64 bit literals supported on all compilers?
EntityKind ClassifyEntity(EntityID id) {
    bool spatial = id.id & 0x8000000000000000ull;
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

SpatialEntity* AddSpatialEntity(GameWorld* world, iv3 p) {
    SpatialEntity* entity = nullptr;
    auto chunkP = ChunkPosFromWorldPos(p).chunk;
    auto chunk = GetChunk(world, chunkP.x, chunkP.y, chunkP.z);
    if (chunk) {
        entity = chunk->spatialEntityStorage.Add();
        if (entity) {
            entity->id = GenEntityID(chunk->world, EntityKind::Spatial);
            entity->p = MakeWorldPos(p);
            entity->residenceChunk = chunk;
            if (chunk->region) {
                RegisterSpatialEntity(chunk->region, entity);
            }
        }
    }
    return entity;
}

void DeleteSpatialEntity(GameWorld* world, SpatialEntity* entity) {
    auto chunk = entity->residenceChunk;
    UnregisterSpatialEntity(chunk->region, entity->id);
    if (entity->inventory) {
        DeleteEntityInventory(entity->inventory);
    }
    chunk->spatialEntityStorage.Remove(entity);
}

void DeleteSpatialEntityAfterThisFrame(GameWorld* world, SpatialEntity* entity) {
    auto entry = BucketArrayPush(&world->spatialEntitiesToDelete);
    assert(entry);
    *entry = entity;
    entity->deleted = true;
}

BlockEntity* AddBlockEntity(GameWorld* world, iv3 p) {
    BlockEntity* entity = nullptr;
    auto chunkP = ChunkPosFromWorldPos(p);
    auto chunk = GetChunk(world, chunkP.chunk);
    if (chunk) {
        auto voxel = GetVoxel(chunk, chunkP.voxel);
        if (voxel) {
            if (!IsVoxelCollider(voxel) && (voxel->entity == nullptr)) {
                entity = chunk->blockEntityStorage.Add();
                if (entity) {
                    entity->id = GenEntityID(chunk->world, EntityKind::Block);
                    entity->p = p;
                    auto occupied = OccupyVoxel(chunk, entity, chunkP.voxel);
                    assert(occupied);
                    if (chunk->region) {
                        RegisterBlockEntity(chunk->region, entity);
                    }
                }
            }
        }
    }
    return entity;
}

void DeleteBlockEntity(GameWorld* world, BlockEntity* entity) {
    // TODO: Get chunk from region for speed?
    auto chunkP = ChunkPosFromWorldPos(entity->p);
    auto chunk = GetChunk(world, chunkP.chunk);
    assert(chunk);
    auto released = ReleaseVoxel(chunk, entity, chunkP.voxel);
    assert(released);
    UnregisterBlockEntity(chunk->region, entity->id);
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


bool UpdateEntityResidence(SpatialEntity* entity) {
    bool changedResidence = false;
    // TODO: Maybe it's not so fast to go through chunk pointer for every entity
    // Maybe we could just have a position from frame start and a position at frame end
    // and compare them?
    auto residenceChunkP = entity->residenceChunk->p;
    auto currentP = ChunkPosFromWorldPos(entity->p.voxel).chunk;
    if (residenceChunkP != currentP) {
        // TODO: Just pass these pointers as agrs?
        auto oldChunk = entity->residenceChunk;
        auto region = oldChunk->region;
        auto world = region->world;
        auto newChunk = GetChunk(world, currentP.x, currentP.y, currentP.z);
        // TODO: Just asserting for now. Should do something smart here.
        // Entity may move to the chunk which is generated yet (i.e. outside of a region).
        // So we need handle this situation somehow. Just scheduling chunk gen will not solve this problem
        // because we will need to wait somehow them. Maybe just discard whole frame movement for entity
        // is actually ok since spatial entities won't have complicated behavior and movements. Of the will?
        assert(newChunk);

        // Don't need to update hash map entry since pointer isn't changed
        //UnregisterSpatialEntity(region, entity->id);
        oldChunk->spatialEntityStorage.Unlink(entity);
        newChunk->spatialEntityStorage.Insert(entity);
        //RegisterSpatialEntity(region, newEntity);
        entity->residenceChunk = newChunk;
        changedResidence = true;
        log_print("[World] Entity %lu changed it's residence (%ld, %ld, %ld) -> (%ld, %ld, %ld)\n", entity->id, oldChunk->p.x, oldChunk->p.y, oldChunk->p.z, newChunk->p.x, newChunk->p.y, newChunk->p.z);
    }
    return changedResidence;
}


WorldPos NormalizeWorldPos(WorldPos p) {
    WorldPos result;

    // NOTE: We are not checking against integer overflowing
    i32 voxelX = (i32)Floor((p.offset.x + Voxel::HalfDim) / Voxel::Dim);
    i32 voxelY = (i32)Floor((p.offset.y + Voxel::HalfDim) / Voxel::Dim);
    i32 voxelZ = (i32)Floor((p.offset.z + Voxel::HalfDim) / Voxel::Dim);

    result.offset.x = p.offset.x - voxelX * Voxel::Dim;
    result.offset.y = p.offset.y - voxelY * Voxel::Dim;
    result.offset.z = p.offset.z - voxelZ * Voxel::Dim;

    result.voxel.x = p.voxel.x + voxelX;
    result.voxel.y = p.voxel.y + voxelY;
    result.voxel.z = p.voxel.z + voxelZ;

    return result;
}

WorldPos Offset(WorldPos p, v3 offset) {
    p.offset += offset;
    auto result = NormalizeWorldPos(p);
    return result;
}

v3 Difference(WorldPos a, WorldPos b) {
    v3 result;
    iv3 voxelDiff = a.voxel - b.voxel;
    v3 offsetDiff = a.offset - b.offset;
    result = Hadamard(V3(voxelDiff), V3(Voxel::Dim)) + offsetDiff;
    return result;
}

v3 RelativePos(WorldPos origin, WorldPos target) {
    v3 result = Difference(target, origin);
    return result;
}

iv3 GetChunkCoord(i32 x, i32 y, i32 z) {
    iv3 result;
    result.x = x >> Chunk::BitShift;
    result.y = y >> Chunk::BitShift;
    result.z = z >> Chunk::BitShift;
    return result;
}

uv3 GetVoxelCoordInChunk(i32 x, i32 y, i32 z) {
    uv3 result;
    result.x = x & Chunk::BitMask;
    result.y = y & Chunk::BitMask;
    result.z = z & Chunk::BitMask;
    return result;
}

ChunkPos ChunkPosFromWorldPos(iv3 tile) {
    ChunkPos result;
    iv3 c = GetChunkCoord(tile.x, tile.y, tile.z);
    uv3 t = GetVoxelCoordInChunk(tile.x, tile.y, tile.z);
    result = ChunkPos{c, t};
    return result;
}

WorldPos WorldPosFromChunkPos(ChunkPos p) {
    WorldPos result = {};
    result.voxel.x = p.chunk.x * Chunk::Size + p.voxel.x;
    result.voxel.y = p.chunk.y * Chunk::Size + p.voxel.y;
    result.voxel.z = p.chunk.z * Chunk::Size + p.voxel.z;
    return result;
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

void ProcessSpatialEntityOverlap(GameWorld* world, SpatialEntity* entity, SpatialEntity* overlapped) {
    switch (entity->type) {
    case SpatialEntityType::Player: {
        if (overlapped->type == SpatialEntityType::Pickup) {
            assert(entity->inventory);
            auto itemRemainder = EntityInventoryPushItem(entity->inventory, overlapped->pickupItem, overlapped->itemCount);
            if (itemRemainder == 0) {
                DeleteSpatialEntityAfterThisFrame(world, overlapped);
            } else {
                overlapped->itemCount = itemRemainder;
            }
            //log_print("Player is overlapping with: %llu %s (%ld, %ld, %ld)\n", overlapped->id, ToString(overlapped->type), overlapped->p.voxel.x, overlapped->p.voxel.y, overlapped->p.voxel.z);
        }
    } break;
    invalid_default();
    }
}

void FindOverlapsFor(GameWorld* world, SpatialEntity* entity) {
    WorldPos origin = entity->p;

    bool overlaps = false;
    bool resolved = true;

    v3 colliderSize = V3(entity->scale);
    v3 colliderRadius = colliderSize * 0.5f;

    iv3 bboxMin = Offset(origin, -colliderRadius).voxel - IV3(1);
    iv3 bboxMax = Offset(origin, colliderRadius).voxel + IV3(1);

    auto minChunk = ChunkPosFromWorldPos(bboxMin).chunk;
    auto maxChunk = ChunkPosFromWorldPos(bboxMax).chunk;

    for (i32 z = minChunk.z; z <= maxChunk.z; z++) {
        for (i32 y = minChunk.y; y <= maxChunk.y; y++) {
            for (i32 x = minChunk.x; x <= maxChunk.x; x++) {
                auto chunk = GetChunk(world, x, y, z);
                if (chunk) {
                    foreach(chunk->spatialEntityStorage) {
                        if (it->id != entity->id) {
                            v3 relativePos = RelativePos(entity->p, it->p);
                            v3 testSize = V3(entity->scale);
                            v3 testRadius = colliderSize * 0.5f;
                            v3 testMin = relativePos - testRadius - colliderRadius;
                            v3 testMax = relativePos + testRadius + colliderRadius;
                            v3 bary = GetBarycentric(testMin, testMax, V3(0.0f));
                            // TODO: Inside bary
                            if ((bary.x > 0.0f && bary.x <= 1.0f) &&
                                (bary.y > 0.0f && bary.y <= 1.0f) &&
                                (bary.z > 0.0f && bary.z <= 1.0f)) {
                                ProcessSpatialEntityOverlap(world, entity, it);
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
    WorldPos origin = entity->p;

    bool overlaps = false;
    bool resolved = true;

    v3 colliderSize = V3(entity->scale);
    v3 colliderRadius = colliderSize * 0.5f;

    iv3 bboxMin = Offset(origin, -colliderRadius).voxel - IV3(1);
    iv3 bboxMax = Offset(origin, colliderRadius).voxel + IV3(1);

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
                        v3 relOrigin = RelativePos(MakeWorldPos(IV3(x, y, z)), origin);
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
                                            v3 neighborRelOrigin = RelativePos(MakeWorldPos(IV3(x, y, z)), MakeWorldPos(IV3(px, py, pz)));
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
                                origin = Offset(MakeWorldPos(IV3(x, y, z)), relNewOrigin);
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

            auto target = Offset(origin, delta);
            iv3 originBoxMin = Offset(origin, -colliderRadius).voxel - IV3(1);
            iv3 originBoxMax = Offset(origin, colliderRadius).voxel + IV3(1);
            iv3 targetBoxMin = Offset(target, -colliderRadius).voxel - IV3(1);
            iv3 targetBoxMax = Offset(target, colliderRadius).voxel + IV3(1);

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
                                v3 relOrigin = RelativePos(MakeWorldPos(IV3(x, y, z)), origin);
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

            origin = Offset(origin, delta * tMin);
            //hitNormal = -hitNormal;

            if (hit) {
                if (hitNormal.y == 1.0f) {
                    entity->grounded = true;
                }
                delta = Difference(target, origin);
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

void ConvertVoxelToPickup(GameWorld* world, iv3 voxelP) {
    auto chunkPos = ChunkPosFromWorldPos(voxelP);
    auto chunk = GetChunk(world, chunkPos.chunk.x, chunkPos.chunk.y, chunkPos.chunk.z);
    auto voxel = GetVoxelForModification(chunk, chunkPos.voxel.x, chunkPos.voxel.y, chunkPos.voxel.z);
    if (voxel->value == VoxelValue::CoalOre) {
        RandomSeries series = {};
        for (u32 i = 0; i < 4; i++) {
            auto entity = AddSpatialEntity(world, voxelP);
            if (entity) {
                v3 randomOffset = V3(RandomUnilateral(&series) - 0.5f, RandomUnilateral(&series) - 0.5f, RandomUnilateral(&series) - 0.5f);
                entity->p = MakeWorldPos(voxelP, randomOffset);
                // TODO: SetEntityPos
                entity->scale = 0.2f;
                entity->type = SpatialEntityType::Pickup;
                entity->pickupItem = Item::CoalOre;
                entity->itemCount = 1;
            }
        }
    }
    voxel->value = VoxelValue::Empty;
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
        auto newChunkP = ChunkPosFromWorldPos(newP);
        auto oldChunkP = ChunkPosFromWorldPos(entity->p);
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
        auto occupied = OccupyVoxel(newChunk, entity, newChunkP.voxel);

        if (occupied) {
            auto released = ReleaseVoxel(oldChunk, entity, oldChunkP.voxel);
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
