#include "Inventory.h"

template <typename F>
void ForEach(EntityInventory* inventory, F func) {
    for (usize i = 0; i < inventory->slotCount; i++) {
        auto slot = inventory->slots + i;
        func (slot);
    }
}

EntityInventory* AllocateEntityInventory(u32 slotCount, u32 slotCapacity) {
    // TODO: Joint allocation
    auto inventory = (EntityInventory*)Platform.Allocate(sizeof(EntityInventory) * slotCount, 0, nullptr);
    assert(inventory);
    auto slots = (InventorySlot*)Platform.Allocate(sizeof(InventorySlot) * slotCount, 0, nullptr);
    ClearArray(slots, slotCount);
    assert(slots);
    inventory->slots = slots;
    inventory->slotCount = slotCount;
    inventory->slotCapacity = slotCapacity;
    return inventory;
}

void DeleteEntityInventory(EntityInventory* inventory) {
    Platform.Deallocate(inventory->slots, nullptr);
    Platform.Deallocate(inventory, nullptr);
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

InventoryPopItemResult EntityInventoryPopItem(EntityInventory* inventory, Item filter, u32 count) {
    InventoryPopItemResult result {};
    for (usize i = 0; i < inventory->slotCount; i++) {
        auto it = inventory->slots + i;
        if (it->count && (filter == Item::None || filter == it->item)) {
            result.item = it->item;
            if (it->count <= count) {
                assert(it->count);
                result.count = it->count;
                it->item = Item::None;
                it->count = 0;
            } else {
                result.count = count;
                it->count -= count;
            }
            break;
        }
    }
    return result;
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

void EntityInventorySerialize(EntityInventory* inventory, BinaryBlob* out) {
    WriteField(out, &inventory->slotCapacity);
    WriteField(out, &inventory->slotCount);
    for (u32 i = 0; i < inventory->slotCount; i++) {
        auto slot = inventory->slots + i;
        WriteField(out, &slot->item);
        WriteField(out, &slot->count);
    }
}

EntityInventory* EntityInventoryDeserialize(EntitySerializedData data) {
    EntityInventory* inventory;
    u32 slotCapacity;
    u32 slotCount;
    ReadField(&data, &slotCapacity);
    ReadField(&data, &slotCount);
    inventory = AllocateEntityInventory(slotCount, slotCapacity);
    if (inventory) {
        for (u32 i = 0; i < inventory->slotCount; i++) {
            auto slot = inventory->slots + i;
            ReadField(&data, &slot->item);
            ReadField(&data, &slot->count);
        }
    }
    return inventory;
}
