#pragma once

#include "Common.h"
#include "Math.h"
#include "Item.h"
#include "Inventory.h"
#include "Block.h"
#include "BinaryBlob.h"

struct Material;
struct GameWorld;
struct RenderGroup;
struct Camera;
struct Entity;
struct GameWorld;
struct Block;

typedef u64 EntityID;

enum struct EntityKind {
    Block, Spatial
};

const char* ToString(EntityKind kind) {
    switch (kind) {
    case EntityKind::Block: { return "Block"; } break;
    case EntityKind::Spatial: { return "Spatial"; } break;
    invalid_default();
    }
    return nullptr;
}

EntityID GenEntityID(GameWorld* world, EntityKind kind);

enum EntityFlags : u32 {
    EntityFlag_Collides = (1 << 0),
    EntityFlag_ProcessOverlaps = (1 << 1),
    EntityFlag_DisableDeleteWhenOutsideOfWorldBounds = (1 << 3),
    // TODO: This probably should be some kind of entity-type associated flag
    // since we do not allow to change it at runtime, because this will break chunk sim propagaton count
    EntityFlag_PropagatesSim = (1 << 4)
};

enum struct EntityType : u32 {
    Unknown = 0,
    Container,
    Pipe,
    Belt,
    Extractor,
    Barrel,
    Tank,
    Pickup,
    Projectile,
    Player,
    _Count
};

struct Entity {
    EntityID id;
    u64 generation; // Frame counter at last UpdateAndRender call
    EntityKind kind;
    EntityType type;
    u32 flags;
    b32 deleted;

    Entity* nextInStorage;
    Entity* prevInStorage;

    GameWorld* world;
};

struct SpatialEntity : Entity {
    WorldPos p;
    b32 grounded;
    v3 velocity;
    f32 scale;
    f32 acceleration;
    f32 friction;
    iv3 currentChunk;
    b32 outsideOfTheWorld;
};

struct BlockEntity : Entity {
    iv3 p;
    // TODO: More data-driven architecture probably?
    b32 dirtyNeighborhood;
    // TODO: Quaternions
    // TODO: Footprints
    //
    iv3* multiBlockEntityFootprint;
};

typedef WorldPos(GetEntityPositionFn)(Entity* entity);

WorldPos GetEntityPosition(Entity* entity);

enum struct EntityUIInvoke: u32 {
    Info, Inventory
};

enum struct EntityBehaviorInvoke : u32 {
    UpdateAndRender, Rotate,
};

struct EntityUpdateAndRenderData {
    f32 deltaTime;
    RenderGroup* group;
    Camera* camera;
};

struct EntityRotateData {
    enum Direction { CW, CCW } direction;
};

typedef void(EntityBehaviorFn)(Entity* entity, EntityBehaviorInvoke reason, void* data);

void SpatialEntityBehavior(Entity* entity, EntityBehaviorInvoke reason, void* data);
void SpatialEntityProcessOverlap(GameWorld* world, SpatialEntity* testEntity, SpatialEntity* overlappedEntity) {};

typedef Entity*(CreateEntityFn)(GameWorld* world, WorldPos p);

struct CollisionInfo {
    WorldPos hitPosition;
    v3 hitNormal;
    Block hitBlock;
};

typedef void(SpatialEntityCollisionResponseFn)(SpatialEntity* entity, const CollisionInfo* info);

template<typename T>
void WriteField(BinaryBlob* blob, const T* field) {
    auto data = blob->Write(sizeof(T));
    memcpy(data, field, sizeof(T));
}

struct EntitySerializedData {
    void* data;
    u32 size;
    u32 at;
};

template<typename T>
void ReadField(EntitySerializedData* data, T* field) {
    auto fieldSize = sizeof(T);
    assert(fieldSize <= (data->size - data->at));
    auto serializedField = (T*)((u8*)data->data + data->at);
    *field = *serializedField;
    data->at += (u32)fieldSize;
}

typedef void(EntitySerializeFn)(Entity* entity, BinaryBlob* output);
typedef void(EntityDeserializeFn)(Entity* entity, EntitySerializedData data);

// TODO: Make one call out of these
typedef void(EntityDeleteFn)(Entity* entity);
typedef void(EntityDropPickupFn)(Entity* entity, GameWorld* world, WorldPos p);
typedef void(EntityProcessOverlapFn)(GameWorld* world, SpatialEntity* testEntity, SpatialEntity* overlappedEntity);
typedef void(EntityUpdateAndRenderUIFn)(Entity* entity, EntityUIInvoke reason);

template <typename F>
void ForEachEntityNeighbor(GameWorld* world, iv3 p, F func);
