#pragma once

#include "Entity.h"

struct Player : SpatialEntity {
    SpatialEntity base;
    f32 height;
    iv3 selectedBlock;
    EntityID selectedEntity;
    f32 jumpAcceleration;
    f32 runAcceleration;
    Camera* camera;
    b32 flightMode;
    EntityInventory* toolbelt;
    EntityInventory* inventory;
    u32 toolbeltSelectIndex;
};

Entity* CreatePlayerEntity(GameWorld* world, WorldPos p);
void DeletePlayer(Entity* entity);
void PlayerUpdateAndRender(Entity* entity, EntityBehaviorInvoke reason, void* data);
void PlayerProcessOverlap(GameWorld* world, SpatialEntity* testEntity, SpatialEntity* overlappedEntity);
void PlayerUpdateAndRenderUI(Entity* entity, EntityUIInvoke reason);
