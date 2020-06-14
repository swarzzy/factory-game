#pragma once

#include "Common.h"

struct BlockEntity;

enum struct BlockValue : u32 {
    Empty = 0,
    Stone,
    Grass,
    CoalOre,
    Water,
    _Count
};

struct Block {
    BlockValue value;
    BlockEntity* entity;
};

typedef void(BlockDropPickupFn)(const Block* voxel, GameWorld* world, WorldPos p);

void BlockDropPickup(const Block* voxel, GameWorld* world, WorldPos p);
void CoalOreDropPickup(const Block* voxel, GameWorld* world, WorldPos p);

bool IsBlockCollider(const Block* block);
