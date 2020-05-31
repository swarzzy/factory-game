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

void ContainerUpdateAndRender(Entity* _entity, EntityUpdateInvoke reason, f32 deltaTime, RenderGroup* group, Camera* camera) {
    if (reason == EntityUpdateInvoke::UpdateAndRender) {
        auto entity = (Container*)_entity;
        auto context = GetContext();
        RenderCommandDrawMesh command{};
        command.transform = Translate(WorldPos::Relative(camera->targetWorldPosition, WorldPos::Make(entity->p))) * Rotate(entity->meshRotation);
        command.mesh = context->containerMesh;;
        command.material = &context->containerMaterial;
        Push(group, &command);
    }
}


void ContainerDropPickup(Entity* entity, GameWorld* world, WorldPos p) {
    auto pickup = (Pickup*)CreatePickupEntity(world, p);
    pickup->item = Item::Container;
    pickup->count = 1;
}
