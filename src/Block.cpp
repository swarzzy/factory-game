#include "Block.h"

void CoalOreDropPickup(const Block* voxel, GameWorld* world, WorldPos p) {
    RandomSeries series = {};
    for (u32 i = 0; i < 4; i++) {
        auto entity = CreatePickup(p, (u32)Item::CoalOre, 1);
        if (entity) {
            auto spatial = static_cast<SpatialEntity*>(entity);
            v3 randomOffset = V3(RandomUnilateral(&series) - 0.5f, RandomUnilateral(&series) - 0.5f, RandomUnilateral(&series) - 0.5f);
            spatial->p = WorldPos::Make(p.block, randomOffset);
        }
    }
};

void BlockDropPickup(const Block* voxel, GameWorld* world, WorldPos p) {
    auto info = GetBlockInfo(voxel->value);
    if (info->associatedItem != 0) {
        auto entity = CreatePickup(p, info->associatedItem, 1);
    }
};

bool IsBlockCollider(const Block* voxel) {
    bool hasColliderTerrain = true;
    bool hasColliderEntity = false;
    if (voxel->entity && (voxel->entity->flags & EntityFlag_Collides)) {
        hasColliderEntity = true;
    }
    if (voxel && (voxel->value == BlockValue::Empty)) {
        hasColliderTerrain = false;
    }

    return hasColliderTerrain || hasColliderEntity;
}
