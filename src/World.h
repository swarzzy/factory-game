#pragma once

#include "Common.h"
#include "HashMap.h"
#include "BucketArray.h"
#include "WorldGen.h"
#include "Memory.h"

struct ChunkMesh;
struct ChunkMesher;
struct Camera;
struct RenderGroup;
struct Mesh;
struct Material;
struct Context;

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

struct EntityID {
    u64 id;
};

inline bool operator==(EntityID a, EntityID b) {
    return a.id == b.id;
}

inline bool operator!=(EntityID a, EntityID b) {
    return a.id != b.id;
}

enum struct VoxelValue : u32 {
    Empty = 0,
    Stone,
    Grass,
    CoalOre,
    Water
};

const f32 MeterScale = 1.0f;

struct BlockEntity* entity;

struct Voxel {
    inline static const f32 Dim = 1.0f * MeterScale;
    inline static const f32 HalfDim = Dim * 0.5f;
    // NOTE: Just go bananas and storing pointer to block entity that lives in this voxel
    // TODO: In the future we probably need some sophisticated structure for
    // fast retrieval of entities by coords without storing 8 BYTE POINTER IN EVERY VOXEL WHICH
    // MAKES EVERY CHUNK AT LEAST 0.25 MB BIGGER. Just static grid subdivision might be enough
    // or there might be an octree in chunk of smth...
    BlockEntity* entity;
    VoxelValue value = VoxelValue::Empty;
};

enum struct ChunkState : u32 {
    Complete = 0, Filling, Filled, Meshing, MeshingFinished, WaitsForUpload, MeshUploadingFinished, UploadingMesh
};

enum struct ChunkPriority : u32 {
    Low = 0, High
};

enum struct SpatialEntityType : u32 {
    Player, Pickup
};

enum struct Item : u32 {
    None = 0,
    Container,
    Stone,
    Grass,
    CoalOre,
    Pipe,
    Barrel
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

VoxelValue ItemToBlock(Item item) {
    switch (item) {
    case Item::None: { return VoxelValue::Empty; } break;
    case Item::Container: { return VoxelValue::Empty; } break;
    case Item::Stone: { return VoxelValue::Stone; } break;
    case Item::Grass: { return VoxelValue::Grass; } break;
    case Item::CoalOre: { return VoxelValue::CoalOre; } break;
    }
    return VoxelValue::Empty;
}


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
    case Item::Container: { return "Container"; }
    case Item::Pipe: { return "Pipe"; }
    case Item::Barrel: { return "Barrel"; }
    case Item::Grass: { return "Grass"; }
    case Item::Stone: { return "Stone"; }
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

    struct Iterator {
        EntityInventory* inventory;
        usize at;
        InventorySlot* Begin() { return inventory->slots + at; }
        InventorySlot* Get() { return inventory->slots + at; }
        void Advance() { at++; }
        bool End() { return at >= inventory->slotCount; }
    };

    inline Iterator GetIterator() { return Iterator { this }; }
};

enum struct EntityKind {
    Block, Spatial
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
    // TODO: Remove this. We already know resident of which chunk we are by position
    Chunk* residenceChunk;

    SpatialEntity* nextInStorage;
    SpatialEntity* prevInStorage;
};

enum struct BlockEntityType : u32 {
    Unknown = 0, Container, Pipe, Barrel
};

BlockEntityType ItemToBlockEntityType(Item item) {
    switch (item) {
    case Item::Container: { return BlockEntityType::Container; } break;
    case Item::Pipe: { return BlockEntityType::Pipe; } break;
    case Item::Barrel: { return BlockEntityType::Barrel; } break;
    }
    return BlockEntityType::Unknown;
}

const char* ToString(BlockEntityType type) {
    switch (type) {
    case BlockEntityType::Container: { return "Container"; } break;
    case BlockEntityType::Pipe: { return "Pipe"; } break;
    case BlockEntityType::Barrel: { return "Barrel"; } break;
    invalid_default();
    }
    return nullptr;
}

enum BlockEntityFlags : u32 {
    BlockEntityFlag_Collides = (1 << 0)
};

struct BlockEntity {
    EntityID id;
    BlockEntityType type;
    u32 flags;
    EntityInventory* inventory;
    b32 deleted;
    // TODO: More data-driven architecture probably?
    b32 dirtyNeighborhood;
    iv3 p; // should be moved only through SetBlockEntityPos
    // TODO: Quaternions
    v3 meshRotation;
    // TODO: Footprints
    Mesh* mesh;
    Material* material;

    // Pipe stuff
    // TODO: use these for update
    b32 nxConnected;
    b32 pxConnected;
    b32 nyConnected;
    b32 pyConnected;
    b32 nzConnected;
    b32 pzConnected;

    inline static const f32 MaxPipeCapacity = 2.0f;
    inline static const f32 MaxBarrelCapacity = 200.0f;
    inline static const f32 PipePressureDrop = 0.0001f;
    b32 source;
    b32 filled;
    Liquid liquid;
    f32 amount;
    f32 pressure;

    BlockEntity* nextInStorage;
    BlockEntity* prevInStorage;
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

// NOTE: Store entities as linked list for now
struct BlockEntityStorage {
    Allocator allocator;
    BlockEntity* first;
    usize count;

    void Init(Allocator allocator) {
        assert(!this->first);
        this->allocator = allocator;
    }

    void Insert(BlockEntity* entity) {
        entity->nextInStorage = this->first;
        if (this->first) this->first->prevInStorage = entity;
        this->first = entity;
        this->count++;
    }

    BlockEntity* Add() {
        BlockEntity* result = nullptr;
        auto entity = (BlockEntity*)this->allocator.Alloc(sizeof(BlockEntity), 0);
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


    void Unlink(BlockEntity* entity) {
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

    void Remove(BlockEntity* entity) {
        Unlink(entity);
        this->allocator.Dealloc(entity);
    }

    struct Iterator {
        BlockEntity* current;
        BlockEntity* Begin() {return current; }
        BlockEntity* Get() { return current; }
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
    BlockEntityStorage blockEntityStorage;

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
    EntityID selectedEntity;
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
    Context* context;
    WorldGen worldGen;
    ChunkMesher* mesher;
    Chunk* firstActive;

    u64 entitySerialCount;

    // Be carefull with pointers
    BucketArray<SpatialEntity*, 16> spatialEntitiesToDelete;
    // Be carefull with pointers
    BucketArray<BlockEntity*, 16> blockEntitiesToDelete;
};

void InitWorld(GameWorld* world, Context* context, ChunkMesher* mesher, u32 seed);
Voxel* GetVoxelRaw(Chunk* chunk, u32 x, u32 y, u32 z);
const Voxel* GetVoxel(Chunk* chunk, u32 x, u32 y, u32 z);
inline const Voxel* GetVoxel(Chunk* chunk, uv3 p) { return GetVoxel(chunk, p.x, p.y, p.z); }
const Voxel* GetVoxel(GameWorld* world, i32 x, i32 y, i32 z);
inline const Voxel* GetVoxel(GameWorld* world, iv3 p) { return GetVoxel(world, p.x, p.y, p.z); }
Voxel* GetVoxelForModification(Chunk* chunk, u32 x, u32 y, u32 z);
Voxel* GetVoxelForModification(Chunk* chunk, u32 x, u32 y, u32 z);

bool OccupyVoxel(Chunk* chunk, BlockEntity* entity, u32 x, u32 y, u32 z);
inline bool OccupyVoxel(Chunk* chunk, BlockEntity* entity, uv3 p) { return OccupyVoxel(chunk, entity, p.x, p.y, p.z); }
bool ReleaseVoxel(Chunk* chunk, BlockEntity* entity, u32 x, u32 y, u32 z);
inline bool ReleaseVoxel(Chunk* chunk, BlockEntity* entity, uv3 p) { return ReleaseVoxel(chunk, entity, p.x, p.y, p.z); }

bool IsVoxelCollider(const Voxel* voxel);

Chunk* AddChunk(GameWorld* world, iv3 coord);
Chunk* GetChunk(GameWorld* world, i32 x, i32 y, i32 z);
inline Chunk* GetChunk(GameWorld* world, iv3 chunkP) { return GetChunk(world, chunkP.x, chunkP.y, chunkP.z); }


SpatialEntity* AddSpatialEntity(GameWorld* world, iv3 p);
void DeleteSpatialEntity(GameWorld* world, SpatialEntity* entity);
void DeleteSpatialEntityAfterThisFrame(GameWorld* world, SpatialEntity* entity);

EntityKind ClassifyEntity(EntityID id);

BlockEntity* AddBlockEntity(GameWorld* world, iv3 p);
void DeleteBlockEntity(GameWorld* world, BlockEntity* entity);
void DeleteBlockEntityAfterThisFrame(GameWorld* world, BlockEntity* entity);

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

bool SetBlockEntityPos(GameWorld* world, BlockEntity* entity, iv3 newP);

bool BuildBlock(Context* context, GameWorld* world, iv3 p, Item item);

BlockEntity* GetBlockEntity(GameWorld* world, iv3 p);
