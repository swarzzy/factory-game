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

EntityID GenEntityID(GameWorld* world, EntityKind kind);

enum EntityFlags : u32 {
    EntityFlag_Collides = (1 << 0)
};

enum struct EntityType : u32 {
    Unknown = 0,
    Container,
    Pipe,
    Barrel,
    Tank,
    Pickup,
    Player,
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

    virtual void Render(RenderGroup* group, Camera* camera) {};
    virtual void Update(f32 deltaTime) {};
};

struct SpatialEntity : public Entity {
    WorldPos p;
    b32 grounded;
    v3 velocity;
    f32 scale;
    f32 acceleration;
    f32 friction;
    virtual void Update(f32 deltaTime) override;
};

struct BlockEntity : public Entity {
    iv3 p;
    // TODO: More data-driven architecture probably?
    b32 dirtyNeighborhood;
    // TODO: Quaternions
    // TODO: Footprints
    //
    iv3* multiBlockEntityFootprint;

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

};

template<typename T>
T* AllocateEntity(Allocator allocator);
