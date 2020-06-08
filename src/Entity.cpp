#include "Entity.h"
#include "World.h"

EntityID GenEntityID(GameWorld* world, EntityKind kind) {
    assert(world->entitySerialCount < (U64::Max - 1));
    EntityID result = ++world->entitySerialCount;
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

NeighborIterator NeighborIterator::Begin(iv3 p) {
    NeighborIterator iter {};
    iter.p = p;
    return iter;
}

bool NeighborIterator::Ended(NeighborIterator* iter) {
    return (iter->at == array_count(NeighborIterator::Offsets));
}

void NeighborIterator::Advance(NeighborIterator* iter) {
    if (iter->at < array_count(NeighborIterator::Offsets)) {
        iter->at++;
    }
}

const Voxel* NeighborIterator::Get(NeighborIterator* iter) {
    static const Voxel* result = nullptr;
    if (!Ended(iter)) {
        auto world = GetWorld();
        result = GetVoxel(world, iter->p + NeighborIterator::Offsets[iter->at]);
    }
    return result;
}

Direction NeighborIterator::GetCurrentDirection(NeighborIterator* iter) {
    assert(iter->at < array_count(NeighborIterator::Directions));
    return Directions[iter->at];
}
