#include "World.h"
#include "flux_renderer.h"

void DebugFillChunk(Chunk* chunk) {
    if (chunk->p.y == 0) {
        for (u32 bz = 0; bz < Chunk::Size; bz++) {
            for (u32 by = 0; by < Chunk::Size; by++) {
                for (u32 bx = 0; bx < Chunk::Size; bx++) {
                    auto block = GetVoxel(chunk, bx, by, bz);
                    if (block) {
                        if (by == 0) {
                            block->value = VoxelValue::Grass;
                        }
                    }
                }
            }
        }
    } else if (chunk->p.y < 0) {
        for (u32 bz = 0; bz < Chunk::Size; bz++) {
            for (u32 by = 0; by < Chunk::Size; by++) {
                for (u32 bx = 0; bx < Chunk::Size; bx++) {
                    auto block = GetVoxel(chunk, bx, by, bz);
                    if (block) {
                        block->value = VoxelValue::Stone;
                    }
                }
            }
        }
    }
}


Voxel* GetVoxelRaw(Chunk* chunk, u32 x, u32 y, u32 z) {
    Voxel* result = &chunk->voxels[x + Chunk::Size * y + Chunk::Size * Chunk::Size * z];
    return result;
}

Voxel* GetVoxel(Chunk* chunk, u32 x, u32 y, u32 z) {
    Voxel* result = nullptr;
    if (x < Chunk::Size && y < Chunk::Size && z < Chunk::Size) {
        result = GetVoxelRaw(chunk, x, y, z);
    }
    return result;
}

Chunk* AddChunk(GameWorld* world, iv3 coord) {
    auto chunk = (Chunk*)PlatformAlloc(sizeof(Chunk));
    *chunk = {};
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
    i32 voxelX = (i32)((p.offset.x + Voxel::HalfDim) / Voxel::Dim);
    i32 voxelY = (i32)((p.offset.y + Voxel::HalfDim) / Voxel::Dim);
    i32 voxelZ = (i32)((p.offset.z + Voxel::HalfDim) / Voxel::Dim);

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

v3 RelativePos(WorldPos origin, WorldPos target) {
    v3 result;
    iv3 voxelDiff = target.voxel - origin.voxel;
    v3 offsetDiff = target.offset - origin.offset;
    result = Hadamard(V3(voxelDiff), V3(Voxel::Dim)) + offsetDiff;
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
