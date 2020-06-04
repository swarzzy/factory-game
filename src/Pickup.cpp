#include "Pickup.h"
#include "RenderGroup.h"

Entity* CreatePickupEntity(GameWorld* world, WorldPos p) {
    auto entity = AddSpatialEntity<Pickup>(world, p);
    if (entity) {
        entity->type = EntityType::Pickup;
        entity->scale = 0.3f;
        entity->meshScale = V3(0.3f);
    }
    return entity;
}

void PickupUpdateAndRender(Entity* _entity, EntityBehaviorInvoke reason, void* _data) {
    SpatialEntityBehavior(_entity, reason, _data);
    if (reason == EntityBehaviorInvoke::UpdateAndRender) {
        auto data = (EntityUpdateAndRenderData*)_data;
        auto entity = (Pickup*)_entity;
        auto context = GetContext();

        auto info = GetItemInfo(entity->item);

        RenderCommandDrawMesh command{};
        command.transform = Translate(WorldPos::Relative(data->camera->targetWorldPosition, entity->p));
        command.mesh = info->mesh;
        command.material = info->material;
        command.transform = command.transform * Scale(entity->meshScale);
        Push(data->group, &command);
    }
}
