#include "Position.h"
#include "World.h"

WorldPos WorldPos::Normalize(WorldPos p) {
    WorldPos result;

    // NOTE: We are not checking against integer overflowing
    i32 voxelX = (i32)Floor((p.offset.x + Globals::BlockHalfDim) / Globals::BlockDim);
    i32 voxelY = (i32)Floor((p.offset.y + Globals::BlockHalfDim) / Globals::BlockDim);
    i32 voxelZ = (i32)Floor((p.offset.z + Globals::BlockHalfDim) / Globals::BlockDim);

    result.offset.x = p.offset.x - voxelX * Globals::BlockDim;
    result.offset.y = p.offset.y - voxelY * Globals::BlockDim;
    result.offset.z = p.offset.z - voxelZ * Globals::BlockDim;

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
    result = Hadamard(V3(voxelDiff), V3(Globals::BlockDim)) + offsetDiff;
    return result;
}

v3 WorldPos::Relative(WorldPos origin, WorldPos target) {
    v3 result = Difference(target, origin);
    return result;
}

iv3 WorldPos::ChunkCoord(iv3 block) {
    iv3 result;
    result.x = block.x >> Globals::ChunkBitShift;
    result.y = block.y >> Globals::ChunkBitShift;
    result.z = block.z >> Globals::ChunkBitShift;
    return result;
}

uv3 WorldPos::BlockInChunk(iv3 block) {
    uv3 result;
    result.x = block.x & Globals::ChunkBitMask;
    result.y = block.y & Globals::ChunkBitMask;
    result.z = block.z & Globals::ChunkBitMask;
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
    result.block.x = p.chunk.x * Globals::ChunkSize + p.block.x;
    result.block.y = p.chunk.y * Globals::ChunkSize + p.block.y;
    result.block.z = p.chunk.z * Globals::ChunkSize + p.block.z;
    return result;
}

Direction Dir::Opposite(Direction dir) {
    switch (dir) {
    case Direction::North: { return Direction::South; } break;
    case Direction::South: { return Direction::North; } break;
    case Direction::West: { return Direction::East; } break;
    case Direction::East: { return Direction::West; } break;
    case Direction::Up: { return Direction::Down; } break;
    case Direction::Down: { return Direction::Up; } break;
        invalid_default();
    }
    return Direction::North;
}

iv3 Dir::ToIV3(Direction dir) {
    switch (dir) {
    case Direction::North: { return IV3(0, 0, -1); } break;
    case Direction::South: { return IV3(0, 0, 1); } break;
    case Direction::West: { return IV3(-1, 0, 0); } break;
    case Direction::East: { return IV3(1, 0, 0); } break;
    case Direction::Up: { return IV3(0, 1, 0); } break;
    case Direction::Down: { return IV3(0, -1, 0); } break;
        invalid_default();
    }
    return IV3(0);
}

Winding Dir::ClassifyTurnY(Direction from, Direction to) {
    Winding result = Winding::CW;
    i32 diff = (i32)to - (i32)from;
    result = diff > 0 ? Winding::CW : Winding::CCW;
    return result;
}

Direction Dir::RotateYCW(Direction from) {
    assert((u32)from < 4);
    u32 result = (u32)from + 1;
    if (result > (u32)Direction::West) result = 0;
    return (Direction)result;
}

f32 Dir::AngleDegY(Direction from, Direction to) {
    i32 i = (i32)to - (i32)from;
    f32 angle = -90.0f * i;
    return angle;
}
