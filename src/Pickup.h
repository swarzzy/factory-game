#pragma once

#include "Entity.h"
#include "Inventory.h"
#include "World.h"

struct PickupEntity : SpatialEntity {
    Item item;
    u32 count;

    virtual void Render(RenderGroup* group, Camera* camera) override;
};

Entity* CreatePickupEntity(GameWorld* world, WorldPos p, Item item, u32 count) {
    auto entity = AddSpatialEntity<PickupEntity>(world, p);
    if (entity) {
        entity->type = EntityType::Pickup;
        entity->scale = 0.2f;
        entity->item = item;
        entity->count = count;
    }
    return entity;
}