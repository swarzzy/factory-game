#pragma once

#include "Block.h"
#include "Globals.h"

struct ChunkPool;
struct Entity;

struct Block {
    inline static const f32 Dim = 1.0f * Globals::MeterScale;
    inline static const f32 HalfDim = Dim * 0.5f;
    // NOTE: Just go bananas and storing pointer to block entity that lives in this voxel
    // TODO: In the future we probably need some sophisticated structure for
    // fast retrieval of entities by coords without storing 8 BYTE POINTER IN EVERY VOXEL WHICH
    // MAKES EVERY CHUNK AT LEAST 0.25 MB BIGGER. Just static grid subdivision might be enough
    // or there might be an octree in chunk of smth...
    // TODO: Make this BlockEntity*
    Entity* entity;
    BlockValue value = BlockValue::Empty;
};

enum struct ChunkState : u32 {
    Complete = 0,
    Filling,
    Filled,
    Meshing,
    MeshingFinished,
    WaitsForUpload,
    MeshUploadingFinished,
    UploadingMesh
};

enum struct ChunkPriority : u32 {
    Low = 0, High
};

// NOTE: Storing entities as linked list for now
struct EntityStorage {
    Entity* first;
    usize count;

    struct Iterator {
        Entity* current;
        Entity* Begin() {return current; }
        Entity* Get() { return current; }
        void Advance() { current = current->nextInStorage; }
        bool End() { return current == nullptr; }
    };

    inline Iterator GetIterator() { return Iterator { this->first }; }
};

void EntityStorageInsert(EntityStorage* storage, Entity* entity);
void EntityStorageUnlink(EntityStorage* storage, Entity* entity);

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

    b32 modified;
    b32 active;
    b32 visible;

    Chunk* nextActive;
    Chunk* prevActive;
    Chunk* nextRendered;
    Chunk* prevRendered;

    iv3 p;
    ChunkMesh* primaryMesh;
    ChunkMesh* secondaryMesh;
    u32 primaryMeshPoolIndex;
    u32 secondaryMeshPoolIndex;

    u32 simPropagationCount;

    EntityStorage entityStorage;

    Block voxels[Size * Size * Size];
    Block nullBlock;
};

Block* GetBlockRaw(Chunk* chunk, u32 x, u32 y, u32 z);

// These getters always return a valid pointer (pointer to null voxel if actual voxel isn't found)
const Block* GetBlock(Chunk* chunk, u32 x, u32 y, u32 z);
inline const Block* GetBlock(Chunk* chunk, uv3 p) { return GetBlock(chunk, p.x, p.y, p.z); }

Block* GetBlockForModification(Chunk* chunk, u32 x, u32 y, u32 z);
Block* GetBlockForModification(Chunk* chunk, u32 x, u32 y, u32 z);

bool OccupyBlock(Chunk* chunk, BlockEntity* entity, u32 x, u32 y, u32 z);
inline bool OccupyBlock(Chunk* chunk, BlockEntity* entity, uv3 p) { return OccupyBlock(chunk, entity, p.x, p.y, p.z); }
bool ReleaseBlock(Chunk* chunk, BlockEntity* entity, u32 x, u32 y, u32 z);
inline bool ReleaseBlock(Chunk* chunk, BlockEntity* entity, uv3 p) { return ReleaseBlock(chunk, entity, p.x, p.y, p.z); }

const char* ToString(ChunkState state);
const char* ToString(ChunkPriority state);
