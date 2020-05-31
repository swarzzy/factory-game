#pragma once

#include "Entity.h"

struct Player : SpatialEntity {
    f32 height;
    iv3 selectedVoxel;
    EntityID selectedEntity;
    f32 jumpAcceleration;
    f32 runAcceleration;
    SimRegion* region;
    Camera* camera;
    b32 flightMode;

};

Entity* CreatePlayerEntity(GameWorld* world, WorldPos p);
void PlayerUpdateAndRender(Entity* entity, EntityUpdateInvoke reason, f32 deltaTime, RenderGroup* group, Camera* camera);
void PlayerProcessOverlap(GameWorld* world, SpatialEntity* testEntity, SpatialEntity* overlappedEntity);
