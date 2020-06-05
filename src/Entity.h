#pragma once

#include "Common.h"
#include "Math.h"
#include "Item.h"
#include "Inventory.h"

struct Material;
struct GameWorld;
struct RenderGroup;
struct Camera;
struct Entity;
struct GameWorld;

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
    EntityFlag_DisableDeleteWhenOutsideOfWorldBounds = (1 << 3)
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
    Player,
    _Count
};

struct Entity {
    EntityID id;
    u64 generation; // UpdateAndRender call count
    EntityKind kind;
    EntityType type;
    u32 flags;
    EntityInventory* inventory;
    b32 deleted;
    v3 meshRotation;

    Entity* nextInStorage;
    Entity* prevInStorage;
    // TODO: just make it global variable
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

enum struct EntityUIInvoke: u32 {
    Info, Inventory
};

enum struct EntityBehaviorInvoke : u32 {
    UpdateAndRender, Rotate
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

void EntityDropPickup(Entity* entity, GameWorld* world, WorldPos p) {};
void SpatialEntityBehavior(Entity* entity, EntityBehaviorInvoke reason, void* data);
void SpatialEntityProcessOverlap(GameWorld* world, SpatialEntity* testEntity, SpatialEntity* overlappedEntity) {};

typedef Entity*(CreateEntityFn)(GameWorld* world, WorldPos p);

// TODO: Make one call out of these
typedef void(EntityDeleteFn)(Entity* entity, GameWorld* world);
typedef void(EntityDropPickupFn)(Entity* entity, GameWorld* world, WorldPos p);
typedef void(EntityProcessOverlapFn)(GameWorld* world, SpatialEntity* testEntity, SpatialEntity* overlappedEntity);
typedef void(EntityUpdateAndRenderUIFn)(Entity* entity, EntityUIInvoke reason);
