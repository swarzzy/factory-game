#include "Entity.h"
#include "World.h"

EntityID GenEntityID(GameWorld* world, EntityKind kind) {
    assert(world->entitySerialCount < (U64::Max - 1));
    u64 mask = kind == EntityKind::Spatial ? 0x8000000000000000ull : 0ull;
    EntityID result = { (++world->entitySerialCount) | mask };
    ClassifyEntity(result);
    return result;
}

void SpatialEntity::Update(f32 deltaTime) {
    v3 frameAcceleration = V3(0.0f, -20.8f, 0.0f);
    v3 movementDelta = 0.5f * frameAcceleration * deltaTime * deltaTime + this->velocity * deltaTime;
    this->velocity += frameAcceleration * deltaTime;
    MoveSpatialEntity(this->world, this, movementDelta, nullptr, nullptr);
}
