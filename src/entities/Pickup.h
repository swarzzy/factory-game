#pragma once

#include "Entity.h"
#include "Inventory.h"
#include "World.h"

struct Pickup : SpatialEntity {
    ItemID item;
    u32 count;
};

Pickup* CreatePickup(WorldPos p, ItemID item, u32 count);
Entity* CreatePickupEntity(GameWorld* world, WorldPos p);
void PickupUpdateAndRender(Entity* entity, EntityBehaviorInvoke reason, void* data);

void SerializePickup(Entity* entity, BinaryBlob* output);
void DeserializePickup(Entity* entity, EntitySerializedData data);
