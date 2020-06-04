#pragma once

#include "Common.h"

// TODO: Convert Item to u32 id's

struct InventorySlot {
    Item item;
    u32 count;
};

struct EntityInventory {
    u32 slotCapacity;
    u32 slotCount;
    InventorySlot* slots;

    struct Iterator {
        EntityInventory* inventory;
        usize at;
        InventorySlot* Begin() { return inventory->slots + at; }
        InventorySlot* Get() { return inventory->slots + at; }
        void Advance() { at++; }
        bool End() { return at >= inventory->slotCount; }
    };

    inline Iterator GetIterator() { return Iterator { this }; }
};

struct InventoryPopItemResult {
    Item item;
    u32 count;
};

// NOTE: Returns a count of items that isn't fitted
u32 EntityInventoryPushItem(EntityInventory* inventory, Item item, u32 count);
InventoryPopItemResult EntityInventoryPopItem(EntityInventory* inventory, Item filter, u32 slot);
Item EntityInventoryPopItem(EntityInventory* inventory, u32 slotIndex);
EntityInventory* AllocateEntityInventory(u32 slotCount, u32 slotCapacity);
void DeleteEntityInventory(EntityInventory* inventory);
