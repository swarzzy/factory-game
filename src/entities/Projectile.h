#pragma once

#include "../Entity.h"
#include "../Inventory.h"
#include "../World.h"

struct Projectile : SpatialEntity {
};

Projectile* CreateProjectile(WorldPos p);
Entity* CreateProjectileEntity(GameWorld* world, WorldPos p);
void ProjectileCollisionResponse(SpatialEntity* entity, const CollisionInfo* info);
void ProjectileUpdateAndRender(Entity* entity, EntityBehaviorInvoke reason, void* data);
