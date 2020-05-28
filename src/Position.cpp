#include "Position.h"
#include "World.h"

WorldPos WorldPos::Normalize(WorldPos p) {
    WorldPos result;

    // NOTE: We are not checking against integer overflowing
    i32 voxelX = (i32)Floor((p.offset.x + Voxel::HalfDim) / Voxel::Dim);
    i32 voxelY = (i32)Floor((p.offset.y + Voxel::HalfDim) / Voxel::Dim);
    i32 voxelZ = (i32)Floor((p.offset.z + Voxel::HalfDim) / Voxel::Dim);

    result.offset.x = p.offset.x - voxelX * Voxel::Dim;
    result.offset.y = p.offset.y - voxelY * Voxel::Dim;
    result.offset.z = p.offset.z - voxelZ * Voxel::Dim;

    result.block.x = p.block.x + voxelX;
    result.block.y = p.block.y + voxelY;
    result.block.z = p.block.z + voxelZ;

    return result;
}

WorldPos WorldPos::Offset(WorldPos p, v3 offset) {
    p.offset += offset;
    auto result = WorldPos::Normalize(p);
    return result;
}

v3 WorldPos::Difference(WorldPos a, WorldPos b) {
    v3 result;
    iv3 voxelDiff = a.block - b.block;
    v3 offsetDiff = a.offset - b.offset;
    result = Hadamard(V3(voxelDiff), V3(Voxel::Dim)) + offsetDiff;
    return result;
}

v3 WorldPos::Relative(WorldPos origin, WorldPos target) {
    v3 result = Difference(target, origin);
    return result;
}

iv3 WorldPos::ChunkCoord(iv3 block) {
    iv3 result;
    result.x = block.x >> Chunk::BitShift;
    result.y = block.y >> Chunk::BitShift;
    result.z = block.z >> Chunk::BitShift;
    return result;
}

uv3 WorldPos::BlockInChunk(iv3 block) {
    uv3 result;
    result.x = block.x & Chunk::BitMask;
    result.y = block.y & Chunk::BitMask;
    result.z = block.z & Chunk::BitMask;
    return result;
}

ChunkPos WorldPos::ToChunk(iv3 block) {
    ChunkPos result;
    iv3 c = WorldPos::ChunkCoord(block);
    uv3 t = WorldPos::BlockInChunk(block);
    result = ChunkPos{c, t};
    return result;
}

ChunkPos WorldPos::ToChunk(WorldPos p) {
    return ToChunk(p.block);
}


WorldPos ChunkPos::ToWorld(ChunkPos p) {
    WorldPos result = {};
    result.block.x = p.chunk.x * Chunk::Size + p.block.x;
    result.block.y = p.chunk.y * Chunk::Size + p.block.y;
    result.block.z = p.chunk.z * Chunk::Size + p.block.z;
    return result;
}




