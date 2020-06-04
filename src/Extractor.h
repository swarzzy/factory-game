#pragma once

#include "Entity.h"


struct Extractor : BlockEntity {
    inline static const f32 ExtractTimeout = 1.5f;
    Direction direction;
    f32 extractTimeout;
    Item buffer;
};

Entity* CreateExtractor(GameWorld* world, WorldPos p);
void ExtractorDelete(Entity* entity, GameWorld* world);
void ExtractorBehavior(Entity* entity, EntityBehaviorInvoke reason, void* data);
void ExtractorDropPickup(Entity* entity, GameWorld* world, WorldPos p);
void ExtractorUpdateAndRenderUI(Entity* entity, EntityUIInvoke reason);
