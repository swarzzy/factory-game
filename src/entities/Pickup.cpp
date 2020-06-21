#include "Pickup.h"
#include "RenderGroup.h"

Pickup* CreatePickup(WorldPos p, ItemID item, u32 count) {
    auto world = GetWorld();
    auto info = GetItemInfo(item);
    auto pickup = (Pickup*)CreatePickupEntity(world, p);
    if (pickup) {
        pickup->item = item;
        pickup->count = count;
    }
    return pickup;
}

Entity* CreatePickupEntity(GameWorld* world, WorldPos p) {
    auto entity = AddSpatialEntity<Pickup>(world, p);
    if (entity) {
        entity->type = EntityType::Pickup;
        entity->scale = Globals::PickupScale;
    }
    return entity;
}

void PickupUpdateAndRender(Entity* _entity, EntityBehaviorInvoke reason, void* _data) {
    SpatialEntityBehavior(_entity, reason, _data);
    if (reason == EntityBehaviorInvoke::UpdateAndRender) {
        auto data = (EntityUpdateAndRenderData*)_data;
        auto entity = (Pickup*)_entity;

        auto info = GetItemInfo(entity->item);

        RenderCommandDrawMesh command{};
        command.transform = Translate(WorldPos::Relative(data->camera->targetWorldPosition, entity->p));
        command.mesh = info->mesh;
        command.material = info->material;
        command.transform = command.transform * Scale(V3(entity->scale));
        Push(data->group, &command);

        if (Globals::DrawCollisionVolumes) {
            f32 radius = entity->scale * 0.5f;
            v3 min = WorldPos::Relative(data->camera->targetWorldPosition, entity->p) - radius;
            v3 max = WorldPos::Relative(data->camera->targetWorldPosition, entity->p) + radius;
            DrawAlignedBoxOutline(data->group, min, max, V3(1.0f, 1.0f, 0.0f), 0.3f);
        }
    }
}
