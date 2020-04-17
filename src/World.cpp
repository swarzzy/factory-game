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

Voxel* GetVoxelForModification(Chunk* chunk, u32 x, u32 y, u32 z) {
    Voxel* result = nullptr;
    if (x < Chunk::Size && y < Chunk::Size && z < Chunk::Size) {
        result = GetVoxelRaw(chunk, x, y, z);
        chunk->dirty = true;
    }
    return result;
}


Chunk* AddChunk(GameWorld* world, iv3 coord) {
    auto chunk = (Chunk*)PlatformAlloc(sizeof(Chunk), 0, nullptr);
    memset(chunk, 0 , sizeof(Chunk));
    chunk->p = coord;
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
    bool collides = true;
    if (voxel && voxel->value == VoxelValue::Empty) {
        collides = false;
    }
    return collides;
}


WorldPos DoMovement(GameWorld* world, WorldPos origin, v3 delta, v3* velocity, bool* hitGround, Camera* camera, RenderGroup* renderGroup) {
    *hitGround = false;

    v3 colliderSize = V3(Voxel::Dim * 0.95f);
    v3 colliderRadius = colliderSize * 0.5f;

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
                    // TODO: Cache chunk pointer
                    auto testVoxel = GetVoxel(world, x, y, z);
                    bool collides = IsVoxelCollider(testVoxel);
                    if (collides) {
                        v3 relOrigin = RelativePos(WorldPos::Make(IV3(x, y, z)), origin);
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
                            //printf("hit something at frame %llu\n", GlobalPlatform.tickCount);
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

        origin = Offset(origin, delta * tMin);
        //hitNormal = -hitNormal;

        if (hit) {
            if (hitNormal.y == 1.0f) {
                *hitGround = true;
            }
            delta = Difference(target, origin);
            *velocity -= Dot(*velocity, hitNormal) * hitNormal;
            delta -= Dot(delta, hitNormal) * hitNormal;
        } else {
            break;
        }
    }

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

                auto testVoxel = GetVoxel(world, x, y, z);
                bool collides = IsVoxelCollider(testVoxel);
                if (collides) {
                    v3 relOrigin = RelativePos(WorldPos::Make(IV3(x, y, z)), origin);
                    v3 minCorner = V3(-Voxel::HalfDim);
                    v3 maxCorner = V3(Voxel::HalfDim);
                    // NOTE: Minkowski sum
                    minCorner += colliderSize * -0.5f;
                    maxCorner += colliderSize * 0.5f;
                    v3 bary = GetBarycentric(minCorner, maxCorner, relOrigin);
                    if ((bary.x > 0.0f && bary.x <= 1.0f) &&
                        (bary.y > 0.0f && bary.y <= 1.0f) &&
                        (bary.z > 0.0f && bary.z <= 1.0f)) {
                        printf("PENETRATION DETECTED!!! at frame %llu\n", GlobalPlatform.tickCount);

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
                                        v3 neighborRelOrigin = RelativePos(WorldPos::Make(IV3(x, y, z)), WorldPos::Make(IV3(px, py, pz)));
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
                            origin = Offset(WorldPos::Make(IV3(x, y, z)), relNewOrigin);
                        }
                    }
                }
            }
        }
    }

    return origin;
}
