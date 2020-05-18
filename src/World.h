#pragma once

#include "HashMap.h"
#include "BucketArray.h"
#include "WorldGen.h"
#include "Memory.h"

struct ChunkMesh;
struct ChunkMesher;
struct Camera;
struct RenderGroup;

struct WorldPos {
    iv3 voxel;
    v3 offset;
};

inline WorldPos MakeWorldPos(iv3 voxel) { return WorldPos{voxel, V3(0.0f)}; }
inline WorldPos MakeWorldPos(i32 x, i32 y, i32 z) { return WorldPos{IV3(x, y, z), V3(0.0f)}; }
inline WorldPos MakeWorldPos(iv3 voxel, v3 offset) { return WorldPos{voxel, offset}; }

struct ChunkPos {
    iv3 chunk;
    uv3 voxel;
};

inline ChunkPos MakeChunkPos(iv3 chunk, uv3 voxel) { return ChunkPos{chunk, voxel}; }

enum struct VoxelValue : u32 {
    Empty = 0,
    Stone,
    Grass,
    CoalOre
};

const f32 MeterScale = 1.0f;

struct Voxel {
    inline static const f32 Dim = 1.0f * MeterScale;
    inline static const f32 HalfDim = Dim * 0.5f;
    VoxelValue value = VoxelValue::Empty;
};

enum struct ChunkState : u32 {
    Complete = 0, Filling, Filled, Meshing, MeshingFinished, WaitsForUpload, MeshUploadingFinished, UploadingMesh
};

enum struct ChunkPriority : u32 {
    Low = 0, High
};

struct EntityID {
    u64 id;
};

inline bool operator==(EntityID a, EntityID b) {
    return a.id == b.id;
}

inline bool operator!=(EntityID a, EntityID b) {
    return a.id != b.id;
}

enum struct SpatialEntityType : u32 {
    Player, Pickup
};

enum struct Item : u32 {
    None = 0, CoalOre
};

// TODO: Generate these
constexpr const char* ToString(SpatialEntityType e) {
    switch (e) {
    case SpatialEntityType::Player: { return "Player"; }
    case SpatialEntityType::Pickup: { return "Pickup"; }
    invalid_default();
    }
    return nullptr;
}

constexpr const char* ToString(Item e) {
    switch (e) {
    case Item::CoalOre: { return "CoalOre"; }
        invalid_default();
    }
    return nullptr;
}

struct InventorySlot {
    Item item;
    u32 count;
};

struct EntityInventory {
    u32 slotCapacity;
    u32 slotCount;
    InventorySlot* slots;
};

// NOTE: Returns a count of items that isn't fitted
u32 EntityInventoryPushItem(EntityInventory* inventory, Item item, u32 count);
EntityInventory* AllocateEntityInventory(u32 slotCount, u32 slotCapacity);
void DeleteEntityInventory(EntityInventory* inventory);

struct SpatialEntity {
    EntityID id;
    SpatialEntityType type;
    Item pickupItem;
    u32 itemCount;
    EntityInventory* inventory;
    // NOTE: Set this flag when deleting entity. It means that entity will be deleted at the end of a frame
    // And should not be simulated anymore
    b32 deleted;
    WorldPos p;
    // TODO: Propperly support entity size
    f32 scale;
    v3 velocity;
    f32 acceleration;
    f32 friction;
    b32 grounded;
    // TODO: When we go to chunk swapping and saving/loading we probably
    // won't be able to store direct pointers to chunks
    Chunk* residenceChunk;

    SpatialEntity* nextInStorage;
    SpatialEntity* prevInStorage;
};

struct GameWorld;
struct SimRegion;

// NOTE: Store entities as linked list for now
struct SpatialEntityStorage {
    Allocator allocator;
    SpatialEntity* first;
    usize count;

    void Init(Allocator allocator) {
        assert(!this->first);
        this->allocator = allocator;
    }

    void Insert(SpatialEntity* entity) {
        entity->nextInStorage = this->first;
        if (this->first) this->first->prevInStorage = entity;
        this->first = entity;
        this->count++;
    }

    SpatialEntity* Add() {
        SpatialEntity* result = nullptr;
        auto entity = (SpatialEntity*)this->allocator.Alloc(sizeof(SpatialEntity), 0);
        if (entity) {
            ClearMemory(entity);
            entity->nextInStorage = this->first;
            if (this->first) this->first->prevInStorage = entity;
            this->first = entity;
            this->count++;
            result = entity;
        }
        return result;
    }


    void Unlink(SpatialEntity* entity) {
        auto prev = entity->prevInStorage;
        auto next = entity->nextInStorage;
        if (prev) {
            prev->nextInStorage = next;
        } else {
            this->first = next;
        }
        if (next) {
            next->prevInStorage = prev;
        }
        assert(this->count);
        this->count--;
        entity->prevInStorage = nullptr;
        entity->nextInStorage = nullptr;
    }

    void Remove(SpatialEntity* entity) {
        Unlink(entity);
        this->allocator.Dealloc(entity);
    }

    struct Iterator {
        SpatialEntity* current;
        SpatialEntity* Begin() {return current; }
        SpatialEntity* Get() { return current; }
        void Advance() { current = current->nextInStorage; }
        bool End() { return current == nullptr; }
    };

    inline Iterator GetIterator() { return Iterator { this->first }; }
};

struct Chunk {
    static const u32 BitShift = 5;
    static const u32 BitMask = (1 << BitShift) - 1;
    static const u32 Size = 1 << BitShift;

    volatile ChunkState state;
    b32 locked;
    b32 filled;
    b32 primaryMeshValid;
    b32 secondaryMeshValid;
    b32 remeshingAfterEdit;
    ChunkPriority priority;
    b32 shouldBeRemeshedAfterEdit;

    SimRegion* region;
    b32 modified;
    b32 active;
    Chunk* nextActive;
    Chunk* prevActive;
    iv3 p;
    ChunkMesh* primaryMesh;
    ChunkMesh* secondaryMesh;
    u32 primaryMeshPoolIndex;
    u32 secondaryMeshPoolIndex;

    SpatialEntityStorage spatialEntityStorage;

    GameWorld* world;
    Voxel voxels[Size * Size * Size];
};

// TODO: We need more complicated structure of chunk hash table
// which is able to handle very large number of entries

u32 ChunkHashFunc(void* value) {
    iv3* p = (iv3*)value;
    // TODO: Reasonable hashing
    u32 hash = p->x * 12342 + p->y * 23423 + p->z * 13;
    return hash;
}

bool ChunkHashCompFunc(void* a, void* b) {
    iv3* p1 = (iv3*)a;
    iv3* p2 = (iv3*)b;
    bool result = (p1->x == p2->x) && (p1->y == p2->y) && (p1->z == p2->z);
    return result;
}

struct Player {
    SimRegion* region;
    EntityID entityID;
    b32 flightMode;
    f32 height;
    iv3 selectedVoxel;
    f32 jumpAcceleration;
    f32 runAcceleration;
};

struct GameWorld {
    static const i32 MinHeight = -(i32)Chunk::Size * 3;
    static const i32 MaxHeight = (i32)Chunk::Size * 3 - 1;
    static const i32 MinHeightChunk = -3;
    static const i32 MaxHeightChunk = 2;
    static const i32 InvalidCoord = I32::Max;
    inline static const iv3 InvalidPos = IV3(InvalidCoord);
    Player player;
    HashMap<iv3, Chunk*, ChunkHashFunc, ChunkHashCompFunc> chunkHashMap;
    // TODO: Dynamic view distance
    static const u32 ViewDistance = 4;
    WorldGen worldGen;
    ChunkMesher* mesher;
    Chunk* firstActive;

    u64 entitySerialCount;

    // Be carefull with pointers
    BucketArray<SpatialEntity*, 16> spatialEntitiesToDelete;
};

void InitWorld(GameWorld* world, ChunkMesher* mesher, u32 seed);
Voxel* GetVoxelRaw(Chunk* chunk, u32 x, u32 y, u32 z);
const Voxel* GetVoxel(Chunk* chunk, u32 x, u32 y, u32 z);
const Voxel* GetVoxel(GameWorld* world, i32 x, i32 y, i32 z);
Voxel* GetVoxelForModification(Chunk* chunk, u32 x, u32 y, u32 z);

Chunk* AddChunk(GameWorld* world, iv3 coord);
Chunk* GetChunk(GameWorld* world, i32 x, i32 y, i32 z);

SpatialEntity* AddSpatialEntity(GameWorld* world, iv3 p);
void DeleteSpatialEntity(GameWorld* world, SpatialEntity* entity);
void DeleteSpatialEntityAfterThisFrame(GameWorld* world, SpatialEntity* entity);
WorldPos NormalizeWorldPos(WorldPos p);

WorldPos Offset(WorldPos p, v3 offset);

v3 Difference(WorldPos a, WorldPos b);
v3 RelativePos(WorldPos origin, WorldPos target);

iv3 GetChunkCoord(i32 x, i32 y, i32 z);

uv3 GetVoxelCoordInChunk(i32 x, i32 y, i32 z);

ChunkPos ChunkPosFromWorldPos(iv3 tile);
WorldPos WorldPosFromChunkPos(ChunkPos p);

void MoveSpatialEntity(GameWorld* world, SpatialEntity* entity, v3 delta, Camera* camera, RenderGroup* renderGroup);

bool UpdateEntityResidence(SpatialEntity* entity);

void ConvertVoxelToPickup(GameWorld* world, iv3 voxelP);

void FindOverlapsFor(GameWorld* world, SpatialEntity* entity);
