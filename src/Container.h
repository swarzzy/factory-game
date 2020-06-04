#pragma once

#include "Entity.h"
#include "EntityTraits.h"

struct Container : BlockEntity {
    ItemExchangeTrait itemExchangeTrait;
};

Entity* CreateContainerEntity(GameWorld* world, WorldPos p);
void ContainerUpdateAndRender(Entity* entity, EntityBehaviorInvoke reason, void* data);
void ContainerDropPickup(Entity* entity, GameWorld* world, WorldPos p);
void ContainerUpdateAndRenderUI(Entity* entity, EntityUIInvoke reason);

EntityPopItemResult ContainerPopItem(Entity* entity, Direction dir, u32 itemID, u32 count);
u32 ContainerPushItem(Entity* entity, Direction dir, u32 itemID, u32 count);
