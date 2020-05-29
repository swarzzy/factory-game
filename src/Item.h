#pragma once

#include "Common.h"

enum struct Item : u32 {
    None = 0,
    Container,
    Stone,
    Grass,
    CoalOre,
    Pipe,
    Barrel,
    Tank
};

enum struct Liquid : u32 {
    Water
};

constexpr const char* ToString(Item e) {
    switch (e) {
    case Item::CoalOre: { return "CoalOre"; }
    case Item::Container: { return "Container"; }
    case Item::Pipe: { return "Pipe"; }
    case Item::Barrel: { return "Barrel"; }
    case Item::Grass: { return "Grass"; }
    case Item::Stone: { return "Stone"; }
    case Item::Tank: { return "Tank"; }
        invalid_default();
    }
    return nullptr;
}

const char* ToString(Liquid liquid) {
    switch (liquid) {
    case Liquid::Water: { return "Water"; }
    invalid_default();
    }
    return nullptr;
}

