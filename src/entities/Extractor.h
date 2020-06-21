#pragma once

#include "Entity.h"
#include "EntityTraits.h"

struct Extractor : BlockEntity {
    inline static const f32 ExtractTimeout = 1.5f;
    Direction direction;
    f32 extractTimeout;
    u32 bufferItemID;
    ItemExchangeTrait itemExchangeTrait;
};

Entity* CreateExtractor(GameWorld* world, WorldPos p);
void ExtractorDelete(Entity* entity, GameWorld* world);
void ExtractorBehavior(Entity* entity, EntityBehaviorInvoke reason, void* data);
void ExtractorDropPickup(Entity* entity, GameWorld* world, WorldPos p);
void ExtractorUpdateAndRenderUI(Entity* entity, EntityUIInvoke reason);

EntityPopItemResult ExtractorPopItem(Entity* entity, Direction dir, u32 itemID, u32 count);
u32 ExtractorPushItem(Entity* entity, Direction dir, u32 itemID, u32 count);
