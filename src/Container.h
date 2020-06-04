#pragma once

#include "Entity.h"

struct Container : BlockEntity {
};

Entity* CreateContainerEntity(GameWorld* world, WorldPos p);
void ContainerUpdateAndRender(Entity* entity, EntityBehaviorInvoke reason, void* data);
void ContainerDropPickup(Entity* entity, GameWorld* world, WorldPos p);
void ContainerUpdateAndRenderUI(Entity* entity, EntityUIInvoke reason);
