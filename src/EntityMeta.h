#pragma once
#include "Entity.h"

struct EntityInfoEntry {

};

struct EntityInfo {
    // TODO: dynarray
    u32 nextEntryIndex;
    EntityInfo info [256];
};

void EntityInfoInit(EntityMetaInfo* info) {

};

u32 MetaInfoRegisterEntry(EntityKind kind)