#pragma once

#include "Common.h"
#include "HashMap.h"
#include "BucketArray.h"
#include "WorldGen.h"
#include "Memory.h"
#include "Position.h"
#include "Entity.h"

struct ChunkMesh;
struct ChunkMesher;
struct Camera;
struct RenderGroup;
struct Mesh;
struct Material;
struct Context;

enum struct Direction : u32 {
    NZ = 0,
    PX = 1,
    PZ = 2,
    NX = 3,
    NY = 4,
    PY = 5,
    West = NX,
    East = PX,
    Down = NY,
    Up = PY,
    North = NZ,
    South = PZ
};

iv3 DirectionToIV3(Direction dir) {
    switch (dir) {
    case Direction::North: { return IV3(0, 0, -1); } break;
    case Direction::South: { return IV3(0, 0, 1); } break;
    case Direction::West: { return IV3(-1, 0, 0); } break;
    case Direction::East: { return IV3(1, 0, 0); } break;
    case Direction::Up: { return IV3(0, 1, 0); } break;
    case Direction::Down: { return IV3(0, -1, 0); } break;
        invalid_default();
    }
    return IV3(0);
}

Direction OppositeDirection(Direction dir) {
    switch (dir) {
    case Direction::North: { return Direction::South; } break;
    case Direction::South: { return Direction::North; } break;
    case Direction::West: { return Direction::East; } break;
    case Direction::East: { return Direction::West; } break;
    case Direction::Up: { return Direction::Down; } break;
    case Direction::Down: { return Direction::Up; } break;
    invalid_default();
    }
    return Direction::North;
}

enum struct RotationDir {
    CW, CCW,
};

RotationDir ClassifyTurn(Direction from, Direction to) {
    RotationDir result = RotationDir::CW;
    i32 diff = (i32)to - (i32)from;
    result = diff > 0 ? RotationDir::CW : RotationDir::CCW;
    return result;
}

Direction RotateYCW(Direction from) {
    assert((u32)from < 4);
    u32 result = (u32)from + 1;
    if (result > (u32)Direction::West) result = 0;
    return (Direction)result;
}

f32 AngleY(Direction from, Direction to) {
    i32 i = (i32)to - (i32)from;
    f32 angle = -90.0f * i;
    return angle;
}

enum struct VoxelValue : u32 {
    Empty = 0,
    Stone,
    Grass,
    CoalOre,
    Water,
    _Count
};

const f32 MeterScale = 1.0f;

struct Voxel {
    inline static const f32 Dim = 1.0f * MeterScale;
    inline static const f32 HalfDim = Dim * 0.5f;
    // NOTE: Just go bananas and storing pointer to block entity that lives in this voxel
    // TODO: In the future we probably need some sophisticated structure for
    // fast retrieval of entities by coords without storing 8 BYTE POINTER IN EVERY VOXEL WHICH
    // MAKES EVERY CHUNK AT LEAST 0.25 MB BIGGER. Just static grid subdivision might be enough
    // or there might be an octree in chunk of smth...
    Entity* entity;
    VoxelValue value = VoxelValue::Empty;
};

typedef void(BlockDropPickupFn)(const Voxel* voxel, GameWorld* world, WorldPos p);

void BlockDropPickup(const Voxel* voxel, GameWorld* world, WorldPos p) {};

void CoalOreDropPickup(const Voxel* voxel, GameWorld* world, WorldPos p);

enum struct ChunkState : u32 {
    Complete = 0, Filling, Filled, Meshing, MeshingFinished, WaitsForUpload, MeshUploadingFinished, UploadingMesh
};

enum struct ChunkPriority : u32 {
    Low = 0, High
};

#if 0
enum struct SpatialEntityType : u32 {
    Player, Pickup
};
#endif

struct GameWorld;
struct SimRegion;

// NOTE: Store entities as linked list for now
struct EntityStorage {
    Allocator allocator;
    Entity* first;
    usize count;

    void Init(Allocator allocator) {
        assert(!this->first);
        this->allocator = allocator;
    }

    void Insert(Entity* entity) {
        entity->nextInStorage = this->first;
        if (this->first) this->first->prevInStorage = entity;
        this->first = entity;
        this->count++;
    }

    void Unlink(Entity* entity) {
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

    struct Iterator {
        Entity* current;
        Entity* Begin() {return current; }
        Entity* Get() { return current; }
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

    EntityStorage entityStorage;

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

struct GameWorld {
    static const i32 MinHeight = -(i32)Chunk::Size * 3;
    static const i32 MaxHeight = (i32)Chunk::Size * 3 - 1;
    static const i32 MinHeightChunk = -3;
    static const i32 MaxHeightChunk = 2;
    static const i32 InvalidCoord = I32::Max;
    inline static const iv3 InvalidPos = IV3(InvalidCoord);
    EntityID playerID;
    HashMap<iv3, Chunk*, ChunkHashFunc, ChunkHashCompFunc> chunkHashMap;
    // TODO: Dynamic view distance
    static const u32 ViewDistance = 4;
    Context* context;
    WorldGen worldGen;
    ChunkMesher* mesher;
    Chunk* firstActive;

    u64 entitySerialCount;

    // Be carefull with pointers
    // Be carefull with pointers
    BucketArray<Entity*, 16> blockEntitiesToDelete;
};

void InitWorld(GameWorld* world, Context* context, ChunkMesher* mesher, u32 seed);
Voxel* GetVoxelRaw(Chunk* chunk, u32 x, u32 y, u32 z);
const Voxel* GetVoxel(Chunk* chunk, u32 x, u32 y, u32 z);
inline const Voxel* GetVoxel(Chunk* chunk, uv3 p) { return GetVoxel(chunk, p.x, p.y, p.z); }
const Voxel* GetVoxel(GameWorld* world, i32 x, i32 y, i32 z);
inline const Voxel* GetVoxel(GameWorld* world, iv3 p) { return GetVoxel(world, p.x, p.y, p.z); }
Voxel* GetVoxelForModification(Chunk* chunk, u32 x, u32 y, u32 z);
Voxel* GetVoxelForModification(Chunk* chunk, u32 x, u32 y, u32 z);

bool OccupyVoxel(Chunk* chunk, Entity* entity, u32 x, u32 y, u32 z);
inline bool OccupyVoxel(Chunk* chunk, Entity* entity, uv3 p) { return OccupyVoxel(chunk, entity, p.x, p.y, p.z); }
bool ReleaseVoxel(Chunk* chunk, Entity* entity, u32 x, u32 y, u32 z);
inline bool ReleaseVoxel(Chunk* chunk, Entity* entity, uv3 p) { return ReleaseVoxel(chunk, entity, p.x, p.y, p.z); }

bool IsVoxelCollider(const Voxel* voxel);

Chunk* AddChunk(GameWorld* world, iv3 coord);
Chunk* GetChunk(GameWorld* world, i32 x, i32 y, i32 z);
inline Chunk* GetChunk(GameWorld* world, iv3 chunkP) { return GetChunk(world, chunkP.x, chunkP.y, chunkP.z); }


template <typename T>
T* AddSpatialEntity(GameWorld* world, WorldPos p);
void DeleteSpatialEntity(GameWorld* world, SpatialEntity* entity);
void DeleteSpatialEntityAfterThisFrame(GameWorld* world, SpatialEntity* entity);

EntityKind ClassifyEntity(EntityID id);

template <typename T>
T* AddBlockEntity(GameWorld* world, iv3 p);
void DeleteBlockEntity(GameWorld* world, BlockEntity* entity);
void DeleteBlockEntityAfterThisFrame(GameWorld* world, BlockEntity* entity);

void MoveSpatialEntity(GameWorld* world, SpatialEntity* entity, v3 delta, Camera* camera, RenderGroup* renderGroup);

bool UpdateEntityResidence(GameWorld* world, SpatialEntity* entity);

void ConvertBlockToPickup(GameWorld* world, iv3 voxelP);

void FindOverlapsFor(GameWorld* world, SpatialEntity* entity);

bool SetBlockEntityPos(GameWorld* world, BlockEntity* entity, iv3 newP);

bool BuildBlock(Context* context, GameWorld* world, iv3 p, Item item);

Entity* GetEntity(GameWorld* world, iv3 p);
