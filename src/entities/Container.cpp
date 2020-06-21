#include "Container.h"
#include "RenderGroup.h"

Entity* CreateContainerEntity(GameWorld* world, WorldPos p) {
    Container* entity = AddBlockEntity<Container>(world, p.block);
    if (entity) {
        entity->type = EntityType::Container;
        entity->inventory = AllocateEntityInventory(64, 128);
        entity->flags |= EntityFlag_Collides;
        entity->itemExchangeTrait.PushItem = ContainerPushItem;
        entity->itemExchangeTrait.PopItem = ContainerPopItem;
    }
    return entity;
}

void DeleteContainer(Entity* entity) {
    auto container = (Container*)entity;
    DeleteEntityInventory(container->inventory);
}

void ContainerUpdateAndRender(Entity* _entity, EntityBehaviorInvoke reason, void* _data) {
    if (reason == EntityBehaviorInvoke::UpdateAndRender) {
        auto data = (EntityUpdateAndRenderData*)_data;
        auto entity = (Container*)_entity;
        auto context = GetContext();
        RenderCommandDrawMesh command{};
        command.transform = Translate(WorldPos::Relative(data->camera->targetWorldPosition, WorldPos::Make(entity->p)));
        command.mesh = context->containerMesh;;
        command.material = &context->containerMaterial;
        Push(data->group, &command);
    }
}


void ContainerDropPickup(Entity* entity, GameWorld* world, WorldPos p) {
    auto pickup = CreatePickup(p, (ItemID)Item::Container, 1);
}

void ContainerUpdateAndRenderUI(Entity* entity, EntityUIInvoke reason) {
    if (reason == EntityUIInvoke::Inventory) {
        auto context = GetContext();
        UIDrawInventory(&context->ui, entity, ((Container*)entity)->inventory);
    }
}

EntityPopItemResult ContainerPopItem(Entity* entity, Direction dir, u32 itemID, u32 count) {
    auto container = (Container*)entity;
    EntityPopItemResult result {};
    auto popResult = EntityInventoryPopItem(container->inventory, (Item)itemID, count);
    result.itemID = (u32)popResult.item;
    result.count = popResult.count;
    return result;
}

u32 ContainerPushItem(Entity* entity, Direction dir, u32 itemID, u32 count) {
    auto container = (Container*)entity;
    u32 result = EntityInventoryPushItem(container->inventory, (Item)itemID, count);
    return result;
}
