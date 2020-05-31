#pragma once

#include "Entity.h"
#include "FlatArray.h"
#include "World.h"

struct Logger;

struct EntityInfoEntry {
    CreateEntityFn* Create;
    EntityDeleteFn* Delete;
    EntityUpdateAndRenderFn* UpdateAndRender;
    EntityDropPickupFn* DropPickup;
    EntityProcessOverlapFn* ProcessOverlap;
    const char* name;
    EntityKind kind;
    u32 typeID;
};

struct ItemInfoEntry {
    u32 id;
    // TODO: To entity and zero as default
    b32 convertsToBlock;
    union {
        u32 associatedEntityTypeID;
        VoxelValue associatedBlock;
    };
    const char* name;
};

struct BlockInfoEntry {
    BlockDropPickupFn* DropPickup;
    const char* name;
    u32 id;
};

struct EntityInfo {
    FlatArray<EntityInfoEntry> entityTable;
    FlatArray<ItemInfoEntry> itemTable;
    FlatArray<BlockInfoEntry> blockTable;
    EntityInfoEntry nullEntity;
    ItemInfoEntry nullItem;
    BlockInfoEntry nullBlock;
};
// TODO: Make entity info global
void EntityInfoInit(EntityInfo* info);
EntityInfoEntry* EntityInfoRegisterEntity(EntityInfo* info, EntityKind kind);
ItemInfoEntry* EntityInfoRegisterItem(EntityInfo* info);
BlockInfoEntry* EntityInfoRegisterBlock(EntityInfo* info);
const EntityInfoEntry* GetEntityInfo(EntityInfo* info, u32 typeID);
const ItemInfoEntry* GetItemInfo(EntityInfo* info, u32 typeID);
const BlockInfoEntry* GetBlockInfo(EntityInfo* info, u32 typeID);
inline const EntityInfoEntry* GetEntityInfo(EntityInfo* info, EntityType type) { return GetEntityInfo(info, (u32)type); }
inline const ItemInfoEntry* GetItemInfo(EntityInfo* info, Item item) { return GetItemInfo(info, (u32)item); }
inline const BlockInfoEntry* GetBlockInfo(EntityInfo* info, VoxelValue block) { return GetBlockInfo(info, (u32)block); }

const EntityInfoEntry* GetEntityInfo(EntityType type);
const ItemInfoEntry* GetItemInfo(Item item);
const BlockInfoEntry* GetBlockInfo(VoxelValue block);

void EntityInfoPrint(EntityInfo* info, Logger* logger);
