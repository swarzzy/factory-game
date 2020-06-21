#pragma once

#include "Common.h"

struct Entity;

typedef u32 ItemID;

enum struct Item : u32 {
    None = 0,
    Container,
    Stone,
    Grass,
    CoalOre,
    Pipe,
    Belt,
    Extractor,
    Barrel,
    Tank,
    Water,
    CoalOreBlock,
    Grenade,
    _Count
};

struct ItemUseResult {
    b32 used;
    b32 destroyAfterUse;
};

typedef ItemUseResult(ItemUseFn)(ItemID id, Entity* user);

enum struct Liquid : u32 {
    Water
};

const char* ToString(Liquid liquid) {
    switch (liquid) {
    case Liquid::Water: { return "Water"; }
    invalid_default();
    }
    return nullptr;
}
