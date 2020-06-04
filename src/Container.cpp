#include "Container.h"
#include "RenderGroup.h"

Entity* CreateContainerEntity(GameWorld* world, WorldPos p) {
    Container* entity = AddBlockEntity<Container>(world, p.block);
    if (entity) {
        entity->type = EntityType::Container;
        entity->inventory = AllocateEntityInventory(64, 128);
        entity->flags |= EntityFlag_Collides;
    }
    return entity;
}

void ContainerUpdateAndRender(Entity* _entity, EntityBehaviorInvoke reason, void* _data) {
    if (reason == EntityBehaviorInvoke::UpdateAndRender) {
        auto data = (EntityUpdateAndRenderData*)_data;
        auto entity = (Container*)_entity;
        auto context = GetContext();
        RenderCommandDrawMesh command{};
        command.transform = Translate(WorldPos::Relative(data->camera->targetWorldPosition, WorldPos::Make(entity->p))) * Rotate(entity->meshRotation);
        command.mesh = context->containerMesh;;
        command.material = &context->containerMaterial;
        Push(data->group, &command);
    }
}


void ContainerDropPickup(Entity* entity, GameWorld* world, WorldPos p) {
    auto pickup = (Pickup*)CreatePickupEntity(world, p);
    pickup->item = Item::Container;
    pickup->count = 1;
}

void ContainerUpdateAndRenderUI(Entity* entity, EntityUIInvoke reason) {
    if (reason == EntityUIInvoke::Inventory) {
        auto context = GetContext();
        UIDrawInventory(&context->ui, entity, entity->inventory);
    }
}
