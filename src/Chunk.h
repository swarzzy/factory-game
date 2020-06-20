#pragma once

#include "Block.h"
#include "Globals.h"

struct ChunkPool;
struct Entity;

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
};

void EntityStorageInsert(EntityStorage* storage, Entity* entity);
void EntityStorageUnlink(EntityStorage* storage, Entity* entity);

template <typename F>
void ForEach(EntityStorage* storage, F func);

struct Chunk {
    static const u32 BitShift = 5;
    static const u32 BitMask = (1 << BitShift) - 1;
    static const u32 Size = 1 << BitShift;

    volatile u64 lastSaveTick;
    volatile ChunkState state;
    volatile u32 saving;

    b32 locked;
    b32 filled;
    b32 primaryMeshValid;
    b32 secondaryMeshValid;
    b32 remeshingAfterEdit;
    ChunkPriority priority;
    b32 shouldBeRemeshedAfterEdit;

    u64 lastModificationTick;
    b32 active;
    b32 visible;

    // TODO: Too many pointers here
    Chunk* nextActive;
    Chunk* prevActive;
    Chunk* nextRendered;
    Chunk* prevRendered;
    Chunk* nextInEvictionList;

    Chunk* nextInFreeList;

    iv3 p;
    ChunkMesh* primaryMesh;
    ChunkMesh* secondaryMesh;
    u32 primaryMeshPoolIndex;
    u32 secondaryMeshPoolIndex;

    u32 simPropagationCount;

    EntityStorage entityStorage;

    // TODO: Is separating block values and living entities actually a good idea?
    BlockValue blocks[Size * Size * Size];
    // NOTE: Just go bananas and storing pointer to block entity that lives in this voxel
    // TODO: In the future we probably need some sophisticated structure for
    // fast retrieval of entities by coords without storing 8 BYTE POINTER IN EVERY VOXEL WHICH
    // MAKES EVERY CHUNK AT LEAST 0.25 MB BIGGER. Just static grid subdivision might be enough
    // or there might be an octree in chunk of smth...
    // TODO: Make this BlockEntity*
    BlockEntity* livingEntities[Size * Size * Size];
    BlockValue nullBlockValue;
};

BlockValue* GetBlockValueRaw(Chunk* chunk, u32 x, u32 y, u32 z);

// Returns null block value if a block isn't exist
BlockValue GetBlockValue(Chunk* chunk, u32 x, u32 y, u32 z);
inline BlockValue GetBlockValue(Chunk* chunk, uv3 p) { return GetBlockValue(chunk, p.x, p.y, p.z); }

BlockEntity* GetBlockEntity(Chunk* chunk, u32 x, u32 y, u32 z);
inline BlockEntity* GetBlockEntity(Chunk* chunk, uv3 p) { return GetBlockEntity(chunk, p.x, p.y, p.z); }

Block GetBlock(Chunk* chunk, u32 x, u32 y, u32 z);
inline Block GetBlock(Chunk* chunk, uv3 p) { return GetBlock(chunk, p.x, p.y, p.z); }


BlockValue* GetBlockForModification(Chunk* chunk, u32 x, u32 y, u32 z);
BlockValue* GetBlockForModification(Chunk* chunk, u32 x, u32 y, u32 z);

bool OccupyBlock(Chunk* chunk, BlockEntity* entity, u32 x, u32 y, u32 z);
inline bool OccupyBlock(Chunk* chunk, BlockEntity* entity, uv3 p) { return OccupyBlock(chunk, entity, p.x, p.y, p.z); }
bool ReleaseBlock(Chunk* chunk, BlockEntity* entity, u32 x, u32 y, u32 z);
inline bool ReleaseBlock(Chunk* chunk, BlockEntity* entity, uv3 p) { return ReleaseBlock(chunk, entity, p.x, p.y, p.z); }

const char* ToString(ChunkState state);
const char* ToString(ChunkPriority state);
