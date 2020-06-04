#include "EntityInfo.h"

#include "Platform.h"
#include "Console.h"

Entity* CreateNullEntity(GameWorld* world, WorldPos p) {
    return nullptr;
};

void EntityInfoInit(EntityInfo* info) {
    FlatArrayInit(&info->entityTable, MakeAllocator(PlatformAlloc, PlatformFree, nullptr));
    FlatArrayInit(&info->itemTable, MakeAllocator(PlatformAlloc, PlatformFree, nullptr));
    FlatArrayInit(&info->blockTable, MakeAllocator(PlatformAlloc, PlatformFree, nullptr));
    info->nullEntity.name = "null";
    info->nullEntity.Create = CreateNullEntity;
    info->nullEntity.kind = EntityKind::Block;
    info->nullItem.name = "null";
    info->nullItem.convertsToBlock = true;
    info->nullItem.associatedBlock = VoxelValue::Empty;
    info->nullBlock.DropPickup = BlockDropPickup;
    info->nullBlock.name = "null";
};

EntityInfoEntry* EntityInfoRegisterEntity(EntityInfo* info, EntityKind kind) {
    EntityInfoEntry* entry = FlatArrayPush(&info->entityTable);
    ClearMemory(entry);
    entry->typeID = info->entityTable.count;
    entry->Create = CreateNullEntity;
    entry->DropPickup = EntityDropPickup;
    entry->kind = kind;
    switch (kind) {
    case EntityKind::Spatial: {
        entry->ProcessOverlap = SpatialEntityProcessOverlap;
        entry->Behavior = SpatialEntityBehavior;
    } break;
    case EntityKind::Block: {
        entry->ProcessOverlap = nullptr;
        entry->Behavior = nullptr;
    } break;
    invalid_default();
    }
    return entry;
}

ItemInfoEntry* EntityInfoRegisterItem(EntityInfo* info) {
    ItemInfoEntry* entry = FlatArrayPush(&info->itemTable);
    ClearMemory(entry);
    entry->id = info->itemTable.count;
    return entry;
}

BlockInfoEntry* EntityInfoRegisterBlock(EntityInfo* info) {
    BlockInfoEntry* entry = FlatArrayPush(&info->blockTable);
    ClearMemory(entry);
    entry->id = info->blockTable.count;
    entry->DropPickup = BlockDropPickup;
    return entry;
}

const EntityInfoEntry* GetEntityInfo(EntityInfo* info, u32 typeID) {
    const EntityInfoEntry* result;
    if (typeID > 0 && typeID <= info->entityTable.count) {
        result = FlatArrayAtUnchecked(&info->entityTable, typeID - 1);
    } else {
        result = &info->nullEntity;
    }
    return result;
}

const ItemInfoEntry* GetItemInfo(EntityInfo* info, u32 typeID) {
    const ItemInfoEntry* result;
    if (typeID > 0 && typeID <= info->itemTable.count) {
        result = FlatArrayAtUnchecked(&info->itemTable, typeID - 1);
    } else {
        result = &info->nullItem;
    }
    return result;
}

const BlockInfoEntry* GetBlockInfo(EntityInfo* info, u32 typeID) {
    const BlockInfoEntry* result;
    if (typeID > 0 && typeID <= info->blockTable.count) {
        result = FlatArrayAtUnchecked(&info->blockTable, typeID - 1);
    } else {
        result = &info->nullBlock;
    }
    return result;
}

const EntityInfoEntry* GetEntityInfo(EntityType type) {
    return GetEntityInfo(&(GetContext()->entityInfo), type);
}

const ItemInfoEntry* GetItemInfo(Item item) {
    return GetItemInfo(&(GetContext()->entityInfo), item);
}

const BlockInfoEntry* GetBlockInfo(VoxelValue block) {
    return GetBlockInfo(&(GetContext()->entityInfo), block);
}

void EntityInfoPrint(EntityInfo* info, Logger* logger) {
    LogMessage(logger, "\nEntity info: \n\n");
    LogMessage(logger, "Entities:\n\n");
    for (usize i = 0; i < info->entityTable.count; i++) {
        auto entry = FlatArrayAt(&info->entityTable, i);
        if (entry) {
            LogMessage(logger, "Type ID: %lu\nKind: %s\nName: %s\nCreate: 0x%llx\nDelete: 0x%llx\nBehavior: 0x%llx\nDropPickup: 0x%llx\nProcessOverlap: 0x%llx\n\n",
                       entry->typeID, ToString(entry->kind), entry->name, (u64)entry->Create, (u64)entry->Delete, (u64)entry->Behavior, (u64)entry->DropPickup, (u64)entry->ProcessOverlap);
        }
    }
    LogMessage(logger, "Items:\n\n");
    for (usize i = 0; i < info->itemTable.count; i++) {
        auto entry = FlatArrayAt(&info->itemTable, i);
        if (entry) {
            LogMessage(logger, "Type ID: %lu\nConverts to block: %s\nName: %s\n%s: %llx\n\n",
                       entry->id, entry->convertsToBlock ? "true" : "false", entry->name, entry->convertsToBlock ? "Associated block ID" : "Associated entity type ID", entry->convertsToBlock ? (u32)entry->associatedBlock : entry->associatedEntityTypeID);
        }
    }
    LogMessage(logger, "Blocks:\n\n");
    for (usize i = 0; i < info->blockTable.count; i++) {
        auto entry = FlatArrayAt(&info->blockTable, i);
        if (entry) {
            LogMessage(logger, "Type ID: %lu\nName: %s\nDropPickup: 0x%llx\n\n",
                       entry->id, entry->name, entry->DropPickup);
        }
    }
}
