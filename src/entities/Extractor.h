#pragma once

#include "Entity.h"
#include "EntityTraits.h"

attrib (RegisterEntity("kind: block, type: Extractor, name: Extractor"))
struct Extractor : BlockEntity {
    inline static const f32 ExtractTimeout = 1.5f;

    attrib (SerializeTrivially)
    Direction direction;

    attrib (SerializeTrivially)
    f32 extractTimeout;

    attrib (SerializeTrivially)
    u32 bufferItemID;

    attrib (EntityTrait("ItemExchange"))
    ItemExchangeTrait itemExchangeTrait;
};

attrib (EntityFunction("Extractor", "Create"))
Entity* CreateExtractor(GameWorld* world, WorldPos p);

attrib (EntityFunction("Extractor", "Delete"))
void ExtractorDelete(Entity* entity, GameWorld* world);

attrib (EntityFunction("Extractor", "Behavior"))
void ExtractorBehavior(Entity* entity, EntityBehaviorInvoke reason, void* data);

attrib (EntityFunction("Extractor", "DropPickup"))
void ExtractorDropPickup(Entity* entity, GameWorld* world, WorldPos p);

attrib (EntityFunction("Extractor", "UpdateAndRenderUI"))
void ExtractorUpdateAndRenderUI(Entity* entity, EntityUIInvoke reason);

attrib (EntityFunction("Extractor", "Serialize"))
void ExtractorSerialize(Entity* entity, BinaryBlob* out);

attrib (EntityFunction("Extractor", "Deserialize"))
void ExtractorDeserialize(Entity* entity, EntitySerializedData data);

EntityPopItemResult ExtractorPopItem(Entity* entity, Direction dir, u32 itemID, u32 count);
u32 ExtractorPushItem(Entity* entity, Direction dir, u32 itemID, u32 count);
