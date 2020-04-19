#include "HashMap.h"

hash_map_template_decl
void Drop(hash_map_template* map) {
    if (map->size > 0) {
        PlatformFree(map->table);
        map->size = 0;
        map->entryCount = 0;
    }
}

hash_map_template_decl
hash_bucket_teamplate* FindEntry(hash_map_template* map, Key* key, bool searchForEmpty) {
    hash_bucket_teamplate* result = nullptr;
    u32 hashMask = map->size - 1;
    u32 hash = HashFunction(key);
    auto firstIndex = hash & hashMask;
    for (u32 offset = 0; offset < map->size; offset++) {
        u32 index = (firstIndex + offset) & hashMask;
        auto entry = map->table + index;
        if (searchForEmpty && (!entry->used)) {
            result = entry;
            break;
        } else if (entry->used && CompareFunction(key, &entry->key)) {
            result = entry;
            break;
        }
    }
    return result;
}

hash_map_template_decl
void Grow(hash_map_template* map) {
    u32 newSize = map->size * hash_map_template::GrowKoef;
    log_print("[Hash map] Growing: old size %lu, old load %.3f, new size %lu new load %.3f\n", map->size, (f32)map->entryCount / (f32)map->size, newSize, (f32)map->entryCount / (f32)newSize);
    auto newMap = hash_map_template::Make(newSize);
    for (u32 i = 0; i < map->size; i++) {
        auto oldBucket = map->table + i;
        if (oldBucket->used) {
            Value* v = Add(&newMap, &oldBucket->key);
            *v = oldBucket->value;
        }
    }
    PlatformFree(map->table, nullptr);
    map->table = newMap.table;
    map->size = newSize;
}


hash_map_template_decl
Value* Add(hash_map_template* map, Key* key) {
    Value* result = nullptr;
    f32 load = (f32)(map->entryCount + 1) / (f32)map->size;
    if (load > hash_map_template::LoadFactor) {
        Grow(map);
    }

    auto entry = FindEntry(map, key, true);
    if (entry) {
        entry->used = true;
        entry->key = *key;
        map->entryCount++;
        result = &entry->value;
    }
    return result;
}

hash_map_template_decl
Value* Get(hash_map_template* map, Key* key) {
    Value* result = nullptr;
    if (key) {
        auto entry = FindEntry(map, key, false);
        if (entry) {
            result = &entry->value;
        }
    }
    return result;
}

hash_map_template_decl
bool Delete(hash_map_template* map, Key* key) {
    bool result = false;
    if (key) {
        auto entry = FindEntry(map, key, false);
        if (entry) {
            assert(map->entryCount);
            entry->used = false;
            map->entryCount--;
            result = true;
        }
    }
    return result;
}
