#pragma once

#include "Entity.h"
#include "EntityTraits.h"
#include "FlatArray.h"
#include "World.h"

struct Logger;
struct Mesh;
struct Material;

struct EntityInfoEntry {
    CreateEntityFn* Create;
    EntityDeleteFn* Delete;
    EntityBehaviorFn* Behavior;
    EntityDropPickupFn* DropPickup;
    EntityProcessOverlapFn* ProcessOverlap;
    SpatialEntityCollisionResponseFn* CollisionResponse;
    EntityUpdateAndRenderUIFn* UpdateAndRenderUI;
    const char* name;
    EntityKind kind;
    bool hasUI;
    u32 typeID;

    u32 traitCount;
    u16 traits[16];
    u16 traitOffsets[16];
};

struct ItemInfoEntry {
    u32 id;
    // TODO: To entity and zero as default
    b32 convertsToBlock;
    union {
        u32 associatedEntityTypeID;
        BlockValue associatedBlock;
    };
    const char* name;
    Mesh* mesh;
    Material* material;
    Texture* icon;
    f32 beltAlign;
    f32 beltScale;
    ItemUseFn* Use;
};

struct BlockInfoEntry {
    BlockDropPickupFn* DropPickup;
    const char* name;
    u32 id;
    ItemID associatedItem;
};

struct EntityTraitInfoEntry {
    const char* name;
    TraitID id;
};

struct EntityInfo {
    FlatArray<EntityInfoEntry> entityTable;
    FlatArray<ItemInfoEntry> itemTable;
    FlatArray<BlockInfoEntry> blockTable;
    FlatArray<EntityTraitInfoEntry> traitTable;
    EntityInfoEntry nullEntity;
    ItemInfoEntry nullItem;
    BlockInfoEntry nullBlock;
    EntityTraitInfoEntry nullTrait;
};
// TODO: Make entity info global
void EntityInfoInit(EntityInfo* info);

EntityInfoEntry* EntityInfoRegisterEntity(EntityInfo* info, EntityKind kind);
ItemInfoEntry* EntityInfoRegisterItem(EntityInfo* info);
BlockInfoEntry* EntityInfoRegisterBlock(EntityInfo* info);
EntityTraitInfoEntry* EntityInfoRegisterTrait(EntityInfo* info);

#define REGISTER_ENTITY_TRAIT(info, entityType, traitMember, traitID) do { auto result = EntityInfoAddTrait(info, (TraitID)traitID, offset_of(entityType, traitMember)); assert(result); } while (false)
bool EntityInfoAddTrait(EntityInfoEntry* info, u16 traitID, u16 offset);

const EntityInfoEntry* GetEntityInfo(EntityInfo* info, u32 id);
const ItemInfoEntry* GetItemInfo(EntityInfo* info, u32 id);
const BlockInfoEntry* GetBlockInfo(EntityInfo* info, u32 id);
const BlockInfoEntry* GetBlockInfo(EntityInfo* info, u32 id);
const EntityTraitInfoEntry* GetTraitInfo(EntityInfo* info, TraitID id);

const EntityInfoEntry* GetEntityInfo(u32 id);
const ItemInfoEntry* GetItemInfo(u32 id);
const BlockInfoEntry* GetBlockInfo(u32 id);
const EntityTraitInfoEntry* GetTraitInfo(TraitID id);

void* FindEntityTrait(Entity* entity, TraitID traitID);

template <typename T>
inline T* FindEntityTrait(Entity* entity) {
    T* result = (T*)FindEntityTrait(entity, T::ID);
    return result;
}

inline const EntityInfoEntry* GetEntityInfo(EntityType type) { return GetEntityInfo((u32)type); }
inline const ItemInfoEntry* GetItemInfo(Item item) { return GetItemInfo((u32)item); }
inline const BlockInfoEntry* GetBlockInfo(BlockValue block) {return GetBlockInfo((u32)block); }

void EntityInfoPrint(EntityInfo* info, Logger* logger);
