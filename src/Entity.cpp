#include "Entity.h"
#include "World.h"

EntityID GenEntityID(GameWorld* world, EntityKind kind) {
    assert(world->entitySerialCount < (U64::Max - 1));
    EntityID result = ++world->entitySerialCount;
    return result;
}

void SpatialEntityBehavior(Entity* _entity, EntityBehaviorInvoke reason, void* _data) {
    if (reason == EntityBehaviorInvoke::UpdateAndRender) {
        auto world = GetWorld();
        auto data = (EntityUpdateAndRenderData*)_data;
        auto entity = (SpatialEntity*)_entity;
        v3 frameAcceleration = V3(0.0f, -20.8f, 0.0f);
        v3 drag = entity->velocity * entity->friction;
        frameAcceleration -= drag;
        v3 movementDelta = 0.5f * frameAcceleration * data->deltaTime * data->deltaTime + entity->velocity * data->deltaTime;
        entity->velocity += frameAcceleration * data->deltaTime;
        MoveSpatialEntity(world, entity, movementDelta, nullptr, nullptr);
    }
}

WorldPos GetEntityPosition(Entity* entity) {
    WorldPos result = WorldPos::Make(GameWorld::InvalidPos);
    switch (entity->kind) {
    case EntityKind::Block: { result = WorldPos::Make(((BlockEntity*)entity)->p); } break;
    case EntityKind::Spatial: { result = ((SpatialEntity*)entity)->p; } break;
        invalid_default();
    }
    return result;
}

template <typename F>
void ForEachEntityNeighbor(GameWorld* world, iv3 p, F func) {
    static const iv3 Offsets[] = {
        { -1,  0,  0 },
        {  1,  0,  0 },
        {  0, -1,  0 },
        {  0,  1,  0 },
        {  0,  0, -1 },
        {  0,  0,  1 },
    };

    static const Direction Directions[] = {
        Direction::West,
        Direction::East,
        Direction::Down,
        Direction::Up,
        Direction::North,
        Direction::South,
    };

    for (usize i = 0; i < array_count(Offsets); i++) {
        auto block = GetBlock(world, p + Offsets[i]);
        func(block, Directions[i]);
    }
}
