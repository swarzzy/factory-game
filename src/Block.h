#pragma once

#include "Common.h"

struct Block;

enum struct BlockValue : u32 {
    Empty = 0,
    Stone,
    Grass,
    CoalOre,
    Water,
    _Count
};

typedef void(BlockDropPickupFn)(const Block* voxel, GameWorld* world, WorldPos p);

void BlockDropPickup(const Block* voxel, GameWorld* world, WorldPos p);
void CoalOreDropPickup(const Block* voxel, GameWorld* world, WorldPos p);

bool IsBlockCollider(const Block* voxel);
