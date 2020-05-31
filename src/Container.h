#pragma once

#include "Entity.h"

struct Container : BlockEntity {
};

Entity* CreateContainerEntity(GameWorld* world, WorldPos p);
void ContainerUpdateAndRender(Entity* entity, EntityUpdateInvoke reason, f32 deltaTime, RenderGroup* group, Camera* camera);
void ContainerDropPickup(Entity* entity, GameWorld* world, WorldPos p);
