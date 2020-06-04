#include "Entity.h"
#include "World.h"

EntityID GenEntityID(GameWorld* world, EntityKind kind) {
    assert(world->entitySerialCount < (U64::Max - 1));
    u64 mask = kind == EntityKind::Spatial ? 0x8000000000000000ull : 0ull;
    EntityID result = { (++world->entitySerialCount) | mask };
    ClassifyEntity(result);
    return result;
}

void SpatialEntityBehavior(Entity* _entity, EntityBehaviorInvoke reason, void* _data) {
    if (reason == EntityBehaviorInvoke::UpdateAndRender) {
        auto data = (EntityUpdateAndRenderData*)_data;
        auto entity = (SpatialEntity*)_entity;
        v3 frameAcceleration = V3(0.0f, -20.8f, 0.0f);
        v3 drag = entity->velocity * entity->friction;
        frameAcceleration -= drag;
        v3 movementDelta = 0.5f * frameAcceleration * data->deltaTime * data->deltaTime + entity->velocity * data->deltaTime;
        entity->velocity += frameAcceleration * data->deltaTime;
        MoveSpatialEntity(entity->world, entity, movementDelta, nullptr, nullptr);
    }
}
