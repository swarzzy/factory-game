#include "Extractor.h"

Entity* CreateExtractor(GameWorld* world, WorldPos p) {
    Extractor* extractor = AddBlockEntity<Extractor>(world, p.block);
    if (extractor) {
        extractor->flags = EntityFlag_Collides;
        extractor->type = EntityType::Extractor;
        extractor->dirtyNeighborhood = true;
        extractor->direction = Direction::North;
        extractor->itemExchangeTrait.PushItem = ExtractorPushItem;
        extractor->itemExchangeTrait.PopItem = ExtractorPopItem;
    }
    return extractor;
}

void ExtractoryDelete(Entity* entity, GameWorld* world) {
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
    if (extractor->bufferItemID == 0) {
        auto from = extractor->p + Dir::ToIV3(Dir::Opposite(extractor->direction));
        auto fromEntity = GetEntity(extractor->world, from);
        if (extractor->extractTimeout <= 0.0f) {
            if (fromEntity) {
                auto beltTrait = FindEntityTrait<BeltTrait>(fromEntity);
                if (beltTrait) {
                    auto item = beltTrait->GrabItem(fromEntity, extractor->direction);
                    extractor->bufferItemID = item;
                }
                auto exchangeTrait = FindEntityTrait<ItemExchangeTrait>(fromEntity);
                if (exchangeTrait) {
                    auto popResult = exchangeTrait->PopItem(fromEntity, extractor->direction, 0, 1);
                    if (popResult.itemID != 0) {
                        extractor->bufferItemID = popResult.itemID;
                    }
                }
                extractor->extractTimeout = Extractor::ExtractTimeout;
            }
        }
    }

    if (extractor->bufferItemID != 0) {
        auto to = extractor->p + Dir::ToIV3(extractor->direction);
        auto toEntity = GetEntity(extractor->world, to);
        if (toEntity) {
            auto beltTrait = FindEntityTrait<BeltTrait>(toEntity);
            if (beltTrait) {
                if (beltTrait->InsertItem(toEntity, extractor->direction, extractor->bufferItemID, 1.0f)) {
                    extractor->bufferItemID = 0;
                }
            } else {
                auto itemExchangeTrait = FindEntityTrait<ItemExchangeTrait>(toEntity);
                if (itemExchangeTrait) {
                    if(itemExchangeTrait->PushItem(toEntity, extractor->direction, extractor->bufferItemID, 1) == 0) {
                        extractor->bufferItemID = 0;
                    }
                }
            }
        }
    }

    RenderCommandDrawMesh command {};
    command.transform = Translate(WorldPos::Relative(data->camera->targetWorldPosition, WorldPos::Make(extractor->p))) * M4x4(RotateY(Dir::AngleDegY(Direction::North, extractor->direction)));
    command.mesh = context->extractorMesh;
    command.material = &context->extractorMaterial;
    Push(data->group, &command);
}

void ExtractorRotate(Extractor* belt, void* _data) {
    auto data = (EntityRotateData*)_data;
    belt->direction = Dir::RotateYCW(belt->direction);
    PostEntityNeighborhoodUpdate(belt->world, belt);
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
    auto itemInfo = GetItemInfo(extractor->bufferItemID);
    ImGui::Text("buffer: %s", itemInfo->name);
}

EntityPopItemResult ExtractorPopItem(Entity* entity, Direction dir, u32 itemID, u32 count) {
    auto extractor = (Extractor*)entity;
    EntityPopItemResult result {};
    if (dir == extractor->direction) {
        if (extractor->bufferItemID) {
            if (itemID == 0 || extractor->bufferItemID == itemID) {
                result.itemID = extractor->bufferItemID;
                result.count = 1;
                extractor->bufferItemID = 0;
            }
        }
    }
    return result;
}

u32 ExtractorPushItem(Entity* entity, Direction dir, u32 itemID, u32 count) {
    auto extractor = (Extractor*)entity;
    if (dir == Dir::Opposite(extractor->direction)) {
        if (count > 0) {
            if (!extractor->bufferItemID) {
                extractor->bufferItemID = itemID;
                count--;
            }
        }
    }
    return count;
}
