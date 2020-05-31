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
    EntityFlag_ProcessOverlaps = (2 << 0),
};

enum struct EntityType : u32 {
    Unknown = 0,
    Container,
    Pipe,
    Barrel,
    Tank,
    Pickup,
    Player,
    _Count
};

struct Entity {
    EntityID id;
    EntityKind kind;
    EntityType type;
    u32 flags;
    EntityInventory* inventory;
    b32 deleted;
    //iv3 p; // should be moved only through SetBlockEntityPos
    v3 meshRotation;
    Mesh* mesh;
    Material* material;

    Entity* nextInStorage;
    Entity* prevInStorage;

    // TODO: just make it global variable
    GameWorld* world;

    // TODO: Maybe we just use entity info instead of virtual functions.
    // That way we will independent from C++ specific feature which will simplify C scripting API implementation,
    // and entities will becode POD again. Further more, we could just use straight old composition instead of
    // inheritance. By the way, it looks like inlining of virtual functions isn't a thing actually
    virtual void Render(RenderGroup* group, Camera* camera) {};
    virtual void Update(f32 deltaTime) {};
};

struct SpatialEntity : Entity {
    WorldPos p;
    b32 grounded;
    v3 velocity;
    f32 scale;
    f32 acceleration;
    f32 friction;
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

enum struct EntityUpdateInvoke : u32 {
    UpdateAndRender, NeighborhoodChanged,
};

void EntityDropPickup(Entity* entity, GameWorld* world, WorldPos p) {};
void SpatialEntityUpdateAndRender(Entity* entity, EntityUpdateInvoke reason, f32 deltaTime, RenderGroup* group, Camera* camera);
void SpatialEntityProcessOverlap(GameWorld* world, SpatialEntity* testEntity, SpatialEntity* overlappedEntity) {};

typedef Entity*(CreateEntityFn)(GameWorld* world, WorldPos p);

// TODO: Make one call out of these
typedef void(EntityDeleteFn)(Entity* entity, GameWorld* world);
typedef void(EntityUpdateAndRenderFn)(Entity* entity, EntityUpdateInvoke reason, f32 deltaTime, RenderGroup* group, Camera* camera);
typedef void(EntityDropPickupFn)(Entity* entity, GameWorld* world, WorldPos p);
typedef void(EntityProcessOverlapFn)(GameWorld* world, SpatialEntity* testEntity, SpatialEntity* overlappedEntity);
