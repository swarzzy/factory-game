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

    virtual void Render(RenderGroup* group, Camera* camera) override;

    virtual void Update(f32 deltaTime) override;
};

Entity* CreatePlayerEntity(GameWorld* world, WorldPos p, SimRegion* region, Camera* camera);