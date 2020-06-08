#pragma once

#include "Common.h"

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
    _Count
};

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
