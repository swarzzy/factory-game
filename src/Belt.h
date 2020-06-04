#pragma once

#include "Entity.h"
#include "World.h"
#include "EntityTraits.h"

struct Belt : BlockEntity {
    constant u32 Capacity = 5;
    constant f32 Speed = 0.4f;
    constant f32 HorzSpeed = Speed * 5.0f;
    constant f32 ItemSink = 0.4f;
    constant f32 ExtractTimeout = 2.0f;
    constant u32 FirstSlot = 0;
    constant u32 LastSlot = Capacity - 1;
    Direction direction;
    u32 items[Capacity];
    f32 itemPositions[Capacity];
    f32 itemHorzPositions[Capacity];
    Direction itemTurnDirections[Capacity];
    f32 extractTimeout;
    BeltTrait beltTrait;
};

Entity* CreateBelt(GameWorld* world, WorldPos p);
void BeltDelete(Entity* entity, GameWorld* world);
void BeltBehavior(Entity* entity, EntityBehaviorInvoke reason, void* data);
void BeltDropPickup(Entity* entity, GameWorld* world, WorldPos p);

bool BeltInsertItem(Entity* entity, Direction dir, u32 itemID, f32 callerItemPos);
u32 BeltGrabItem(Entity* entity, Direction dir);
