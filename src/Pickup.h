#pragma once

#include "Entity.h"
#include "Inventory.h"
#include "World.h"

struct Pickup : SpatialEntity {
    Item item;
    u32 count;
    v3 meshScale;
};

Entity* CreatePickupEntity(GameWorld* world, WorldPos p);
void PickupUpdateAndRender(Entity* entity, EntityBehaviorInvoke reason, void* data);
