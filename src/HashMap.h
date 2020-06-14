#pragma once

//
// TODO: Itartors
//

//
// NOTE: IMPORTANT: Pointers returned by hash map functions are valid only
// before next write to a map.
//

template<typename Key, typename Value>
struct HashBucket {
    b32 used;
    Key key;
    Value value;
};

typedef u32(HashFunctionFn)(void*);
typedef bool(CompareFunctionFn)(void*, void*);

#define hash_map_template_decl template<typename Key, typename Value, HashFunctionFn* HashFunction, CompareFunctionFn* CompareFunction>
#define hash_map_template HashMap<Key, Value, HashFunction, CompareFunction>
#define hash_map_iter_template HashMapIter<Key, Value, HashFunction, CompareFunction>
#define hash_bucket_teamplate HashBucket<Key, Value>

hash_map_template_decl
struct HashMap {
    static constexpr u32 DefaultSize = 128;
    // Totally random value
    static constexpr f32 LoadFactor = 0.8;
    static constexpr u32 GrowKoef = 2;

    u32 entryCount;
    u32 size = DefaultSize;
    HashBucket<Key, Value>* table;

    static HashMap Make(u32 size = DefaultSize) {
        HashMap map = {};
        map.table = (HashBucket<Key, Value>*)PlatformAllocClear(sizeof(HashBucket<Key, Value>) * size);
        map.size = size;
        return map;
    }

};

template<typename Key, typename Value, HashFunctionFn* HashFunction, CompareFunctionFn* CompareFunction, typename F>
void ForEach(hash_map_template* map, F func) {
    for (usize i = 0; i < map->size; i++) {
        if (map->table[at].used) {
            auto entry = &map->table[i].value;
            func(entry);
        }
    }
}

hash_map_template_decl
void Drop(hash_map_template* map);

hash_map_template_decl
Value* Add(hash_map_template* map, Key* key);

hash_map_template_decl
Value* Get(hash_map_template* map, Key* key);

hash_map_template_decl
bool Delete(hash_map_template* map, Key* key);
