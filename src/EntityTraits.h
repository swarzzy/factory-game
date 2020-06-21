#pragma once

struct Player;

typedef u16 TraitID;

enum struct Trait {
    Unknown = 0,
    Belt,
    ItemExchange,
    HandUsable,
    _Count
};

struct EntityPopItemResult {
    u32 itemID;
    u32 count;
};

// NOTE: if item is is 0 then filltering is disabled and entity may pop any item
// NOTE: Returns zero if all items pushed, otherwise returns remainder
typedef u32(ItemExchangeTraitPushItemFn)(Entity* entity, Direction dir, u32 itemID, u32 count);
typedef EntityPopItemResult(ItemExchangeTraitPopItemFn)(Entity* entity, Direction dir, u32 itemID, u32 count);

struct ItemExchangeTrait {
    constant TraitID ID = (TraitID)Trait::ItemExchange;
    ItemExchangeTraitPushItemFn* PushItem;
    ItemExchangeTraitPopItemFn* PopItem;
};

typedef void(HandUsableTraitUseFn)(Entity* entity, Player* caller);

struct HandUsableTrait {
    HandUsableTraitUseFn* Use;
};
