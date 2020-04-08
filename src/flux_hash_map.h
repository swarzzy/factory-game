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

hash_map_template_decl
void Drop(hash_map_template* map);

hash_map_template_decl
struct HashMapIter
{
    hash_map_template* map;
    u32 at;

    inline hash_map_iter_template& operator++() {
        do {
            if (at == map->size) {
                break;
            }
            at++;
        } while (!(map->table[at].used));

        return *this;
    }

    bool operator!=(hash_map_iter_template const& other) const {
        assert(this->map == other.map);
        return at != other.at;
    }

    Value& operator*() {
        return map->table[at].value;
    }
};

hash_map_template_decl
inline hash_map_iter_template begin(hash_map_template& map) {
    hash_map_iter_template iter = {};
    u32 at = 0;
    while (!(map.table[at].used)) {
        at++;
    }
    iter.map = &map;
    iter.at = at;
    return iter;
}

hash_map_template_decl
inline hash_map_iter_template end(hash_map_template& map) {
    hash_map_iter_template iter = {};
    iter.map = &map;
    iter.at = map.size;
    return iter;
}

hash_map_template_decl
Value* Add(hash_map_template* map, Key* key);

hash_map_template_decl
Value* Get(hash_map_template* map, Key* key);

hash_map_template_decl
bool Delete(hash_map_template* map, Key* key);
