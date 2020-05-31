#pragma once

#include "Entity.h"
#include "Inventory.h"
#include "World.h"

struct Pickup : SpatialEntity {
    Item item;
    u32 count;
};

Entity* CreatePickupEntity(GameWorld* world, WorldPos p);
void PickupUpdateAndRender(Entity* entity, EntityUpdateInvoke reason, f32 deltaTime, RenderGroup* group, Camera* camera);
