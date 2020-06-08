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
    FlatArrayInit(&info->traitTable, MakeAllocator(PlatformAlloc, PlatformFree, nullptr));
    info->nullEntity.name = "null";
    info->nullEntity.Create = CreateNullEntity;
    info->nullEntity.kind = EntityKind::Block;

    info->nullItem.name = "null";
    info->nullItem.convertsToBlock = true;
    info->nullItem.associatedBlock = VoxelValue::Empty;

    info->nullBlock.DropPickup = BlockDropPickup;
    info->nullBlock.name = "null";

    info->nullTrait.name = "null";
};

EntityInfoEntry* EntityInfoRegisterEntity(EntityInfo* info, EntityKind kind) {
    EntityInfoEntry* entry = FlatArrayPush(&info->entityTable);
    ClearMemory(entry);
    entry->typeID = info->entityTable.count;
    entry->Create = CreateNullEntity;
    entry->DropPickup = nullptr;
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
    entry->beltAlign = Globals::DefaultBeltItemMeshAlign;
    entry->beltScale = Globals::PickupScale;
    // TODO: Setting cube mesh as default for now
    auto context = GetContext();
    entry->mesh = context->cubeMesh;
    return entry;
}

BlockInfoEntry* EntityInfoRegisterBlock(EntityInfo* info) {
    BlockInfoEntry* entry = FlatArrayPush(&info->blockTable);
    ClearMemory(entry);
    entry->id = info->blockTable.count;
    entry->DropPickup = BlockDropPickup;
    return entry;
}

EntityTraitInfoEntry* EntityInfoRegisterTrait(EntityInfo* info) {
    EntityTraitInfoEntry* entry = FlatArrayPush(&info->traitTable);
    ClearMemory(entry);
    entry->id = info->traitTable.count;
    return entry;
}

bool EntityInfoAddTrait(EntityInfoEntry* info, TraitID traitID, u16 offset) {
    bool result = false;
    if (info->traitCount < array_count(info->traits)) {
        info->traits[info->traitCount] = traitID;
        info->traitOffsets[info->traitCount] = offset;
        info->traitCount++;
        result = true;
    }
    return result;
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

const EntityTraitInfoEntry* GetTraitInfo(EntityInfo* info, TraitID id) {
    const EntityTraitInfoEntry* result;
    if (id > 0 && id <= info->traitTable.count) {
        result = FlatArrayAtUnchecked(&info->traitTable, id - 1);
    } else {
        result = &info->nullTrait;
    }
    return result;
}

const EntityInfoEntry* GetEntityInfo(u32 type) {
    return GetEntityInfo(&(GetContext()->entityInfo), type);
}

const ItemInfoEntry* GetItemInfo(u32 item) {
    return GetItemInfo(&(GetContext()->entityInfo), item);
}

const BlockInfoEntry* GetBlockInfo(u32 block) {
    return GetBlockInfo(&(GetContext()->entityInfo), block);
}

const EntityTraitInfoEntry* GetTraitInfo(TraitID id) {
    return GetTraitInfo(&(GetContext()->entityInfo), id);
}

void* FindEntityTrait(Entity* entity, TraitID traitID) {
    void* result = nullptr;
    auto info = GetEntityInfo(entity->type);
    if (info) {
        // TODO: SIMD probing
        for (usize i = 0; i < array_count(info->traits); i++) {
            if (info->traits[i] == traitID) {
                result = (void*)((uptr)entity + (uptr)info->traitOffsets[i]);
                break;
            }
        }
    }
    return result;
}

void EntityInfoPrint(EntityInfo* info, Logger* logger) {
    LogMessage(logger, "\nEntity info: \n\n");
    LogMessage(logger, "Entities:\n\n");
    for (usize i = 0; i < info->entityTable.count; i++) {
        auto entry = FlatArrayAt(&info->entityTable, i);
        if (entry) {
            LogMessage(logger, "Type ID: %lu\nKind: %s\nName: %s\nCreate: 0x%llx\nDelete: 0x%llx\nBehavior: 0x%llx\nDropPickup: 0x%llx\nProcessOverlap: 0x%llx\n",
                       entry->typeID, ToString(entry->kind), entry->name, (u64)entry->Create, (u64)entry->Delete, (u64)entry->Behavior, (u64)entry->DropPickup, (u64)entry->ProcessOverlap);
            if (entry->traitCount) {
                LogMessage(logger, "Traits (count: %lu):\n", (u32)entry->traitCount);
                for (usize i = 0; i < entry->traitCount; i++) {
                    auto traitInfo = GetTraitInfo(entry->traits[i]);
                    LogMessage(logger, "[%lu] %s (offset: %lu)\n", (u32)traitInfo->id, traitInfo->name, entry->traitOffsets[i]);
                }
            }
            LogMessage(logger, "\n");
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
