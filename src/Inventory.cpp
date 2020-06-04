#include "Inventory.h"

EntityInventory* AllocateEntityInventory(u32 slotCount, u32 slotCapacity) {
    // TODO: Joint allocation
    auto inventory = (EntityInventory*)PlatformAlloc(sizeof(EntityInventory) * slotCount, 0, nullptr);
    assert(inventory);
    auto slots = (InventorySlot*)PlatformAlloc(sizeof(InventorySlot) * slotCount, 0, nullptr);
    ClearArray(slots, slotCount);
    assert(slots);
    inventory->slots = slots;
    inventory->slotCount = slotCount;
    inventory->slotCapacity = slotCapacity;
    return inventory;
}

void DeleteEntityInventory(EntityInventory* inventory) {
    PlatformFree(inventory->slots, nullptr);
    PlatformFree(inventory, nullptr);
}

u32 EntityInventoryPushItem(EntityInventory* inventory, Item item, u32 count) {
    bool fitted = false;
    if (count > 0) {
        for (usize i = 0; i < inventory->slotCount; i++) {
            auto slot = inventory->slots + i;
            if (slot->item == Item::None || slot->item == item) {
                u32 slotFree = inventory->slotCapacity - slot->count;
                u32 amount = count <= slotFree ? count : slotFree;
                count = count - amount;
                slot->item = item;
                slot->count += amount;
                if (!count) break;
            }
        }
    }
    return count;
}

Item EntityInventoryPopItem(EntityInventory* inventory) {
    Item item = Item::None;
    for (usize i = 0; i < inventory->slotCount; i++) {
        auto slot = inventory->slots + i;
        if (slot->item != Item::None) {
            assert(slot->count);
            slot->count--;
            item = slot->item;
            if (slot->count == 0) {
                slot->item = Item::None;
            }
            break;
        }
    }
    return item;
}

Item EntityInventoryPopItem(EntityInventory* inventory, u32 slotIndex) {
    Item item = Item::None;
    if (slotIndex < inventory->slotCount) {
        auto slot = inventory->slots + slotIndex;
        if (slot->item != Item::None) {
            assert(slot->count);
            slot->count--;
            item = slot->item;
            if (slot->count == 0) {
                slot->item = Item::None;
            }
        }
    }
    return item;
}
