#include "Pickup.h"
#include "RenderGroup.h"

Entity* CreatePickupEntity(GameWorld* world, WorldPos p) {
    auto entity = AddSpatialEntity<Pickup>(world, p);
    if (entity) {
        entity->type = EntityType::Pickup;
        entity->scale = 0.2f;
    }
    return entity;
}

void PickupUpdateAndRender(Entity* _entity, EntityUpdateInvoke reason, f32 deltaTime, RenderGroup* group, Camera* camera) {
    SpatialEntityUpdateAndRender(_entity, reason, deltaTime, group, camera);
    if (reason == EntityUpdateInvoke::UpdateAndRender) {
        auto entity = (Pickup*)_entity;
        auto context = GetContext();

        RenderCommandDrawMesh command{};
        command.transform = Translate(WorldPos::Relative(camera->targetWorldPosition, entity->p));
        switch (entity->item) {
        case Item::CoalOre: {
            command.mesh = context->coalOreMesh;
            command.material = &context->coalOreMaterial;
        } break;
        case Item::Container: {
            command.mesh = context->containerMesh;
            command.material = &context->containerMaterial;
            command.transform = command.transform * Scale(V3(0.2));
        } break;
        case Item::Pipe: {
            command.mesh = context->pipeStraightMesh;
            command.material = &context->pipeMaterial;
            command.transform = command.transform * Scale(V3(0.2));
        } break;

            invalid_default();
        }
        Push(group, &command);
    }
}
