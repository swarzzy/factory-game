#include "Extractor.h"

Entity* CreateExtractor(GameWorld* world, WorldPos p) {
    Extractor* extractor = AddBlockEntity<Extractor>(world, p.block);
    if (extractor) {
        extractor->flags = EntityFlag_Collides;
        extractor->type = EntityType::Extractor;
        MakeEntityNeighborhoodDirty(world, extractor);
        extractor->dirtyNeighborhood = true;
        extractor->direction = Direction::North;
    }
    return extractor;
}

void ExtractoryDelete(Entity* entity, GameWorld* world) {
    MakeEntityNeighborhoodDirty(world, (BlockEntity*)entity);
}

void ExtractorDropPickup(Entity* entity, GameWorld* world, WorldPos p) {
    auto extractor = (Extractor*)entity;
    auto pickup = (Pickup*)CreatePickupEntity(world, p);
    pickup->item = Item::Extractor;
    pickup->count = 1;
}

void ExtractorUpdateAndRender(Extractor* extractor, void* _data) {
    auto data = (EntityUpdateAndRenderData*)_data;
    auto context = GetContext();

    extractor->extractTimeout = Clamp(extractor->extractTimeout - data->deltaTime, 0.0f, Extractor::ExtractTimeout);
    if (extractor->buffer == Item::None) {
        auto from = extractor->p + DirectionToIV3(OppositeDirection(extractor->direction));
        auto fromEntity = GetEntity(extractor->world, from);
        // TODO: Some _extractable_ trait instead of checking inventory
        if (extractor->extractTimeout <= 0.0f) {
            if (fromEntity && fromEntity->inventory) {
                Item item = EntityInventoryPopItem(fromEntity->inventory);
                if (item != Item::None) {
                    extractor->buffer = item;
                }
                extractor->extractTimeout = Extractor::ExtractTimeout;
            } else if (fromEntity && fromEntity->type == EntityType::Belt) {
                auto belt = (Belt*)fromEntity;
                if (belt->direction == extractor->direction) {
                    if (belt->items[Belt::Capacity - 1] != Item::None) {
                        extractor->buffer = belt->items[Belt::Capacity - 1];
                        belt->items[Belt::Capacity - 1] = Item::None;
                        belt->itemPositions[Belt::Capacity - 1] = 0.0f;
                    }
                }
            }
        }
    }

    if (extractor->buffer != Item::None) {
        auto to = extractor->p + DirectionToIV3(extractor->direction);
        auto toEntity = GetEntity(extractor->world, to);
        // TODO: Some kind if _accept_inserts_ trait so we can insert not only to belts?
        if (toEntity && toEntity->type == EntityType::Belt) {
            auto belt = (Belt*)toEntity;
            if (belt->direction == extractor->direction) {
                if (belt->items[0] == Item::None) {
                    belt->items[0] = extractor->buffer;
                    belt->itemPositions[0] = 0.0f;
                    extractor->buffer = Item::None;
                }
            }
        } else if (toEntity && toEntity->inventory) {
            u32 remainder = EntityInventoryPushItem(toEntity->inventory, extractor->buffer, 1);
            if (!remainder) {
                extractor->buffer = Item::None;
            }
        }
    }

    RenderCommandDrawMesh command {};
    command.transform = Translate(WorldPos::Relative(data->camera->targetWorldPosition, WorldPos::Make(extractor->p))) * M4x4(RotateY(AngleY(Direction::North, extractor->direction)));
    command.mesh = context->extractorMesh;
    command.material = &context->extractorMaterial;
    Push(data->group, &command);
}

void ExtractorRotate(Extractor* belt, void* _data) {
    auto data = (EntityRotateData*)_data;
    belt->direction = RotateYCW(belt->direction);
    MakeEntityNeighborhoodDirty(belt->world, belt);
}

void ExtractorBehavior(Entity* entity, EntityBehaviorInvoke reason, void* data) {
    auto extractor = (Extractor*)entity;
    switch (reason) {
    case EntityBehaviorInvoke::UpdateAndRender: { ExtractorUpdateAndRender(extractor, data); } break;
    case EntityBehaviorInvoke::Rotate: { ExtractorRotate(extractor, data); } break;
    default: {} break;
    }
}

void ExtractorUpdateAndRenderUI(Entity* entity, EntityUIInvoke reason) {
    auto extractor = (Extractor*)entity;
    auto itemInfo = GetItemInfo(extractor->buffer);
    ImGui::Text("buffer: %s", itemInfo->name);
}
