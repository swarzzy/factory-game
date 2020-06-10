#include "Player.h"

Entity* CreatePlayerEntity(GameWorld* world, WorldPos p) {
    Player* entity = AddSpatialEntity<Player>(world, p);
    if (entity) {
        entity->type = EntityType::Player;
        entity->flags |= EntityFlag_ProcessOverlaps | EntityFlag_DisableDeleteWhenOutsideOfWorldBounds;
        entity->scale = 0.95f;
        entity->acceleration = 70.0f;
        entity->friction = 10.0f;
        entity->height = 1.8f;
        entity->selectedBlock = GameWorld::InvalidPos;
        entity->jumpAcceleration = 420.0f;
        entity->runAcceleration = 140.0f;
        entity->inventory = AllocateEntityInventory(16, 128);
        entity->toolbelt = AllocateEntityInventory(8, 128);
    }
    return entity;
}

// TODO: Maybe move toolbelt drawind logic to UI
void PlayerDrawToolbelt(Player* player) {
    auto platform = GetPlatform();

    if (platform->input.scrollFrameOffset != 0) {
        player->toolbeltSelectIndex = (u32)Clamp((i32)player->toolbeltSelectIndex - platform->input.scrollFrameOffset, (i32)0, (i32)player->toolbelt->slotCount - 1);
    }

    // TODO: Remove this hack and draw player UI somewhere else
    auto ui = &_GlobalContext->ui;
    ImGuiIO& io = ImGui::GetIO();
    ImVec2 windowPos = ImVec2(io.DisplaySize.x / 2, io.DisplaySize.y);
    ImVec2 windowPivot = ImVec2(0.5f, 1.0f);
    ImGui::SetNextWindowPos(windowPos, ImGuiCond_Always, windowPivot);
    ImGui::SetNextWindowBgAlpha(0.35f);
    bool open = true;
    if (ImGui::Begin("player toolbelt", &open, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoNav)) {
        u32 i = 0;
        foreach (*player->toolbelt) {
            ImGui::SameLine();
            ImGui::PushID(it);
            bool selected;
            if (it->item == Item::None) {
                // TODO: id
                ImGui::PushID(i + 123423);
                if (ImGui::Button("", ImVec2(50, 50))) {
                    UIDropItem(ui, player->toolbelt, i);
                }
                ImGui::PopID();
            } else {
                auto info = GetItemInfo(it->item);
                if (info->icon) {
                    selected = ImGui::ImageButton((void*)(uptr)info->icon->gpuHandle, ImVec2(50, 50), ImVec2(0,0), ImVec2(1,1), 0);
                } else {
                    // nocheckin
                    // TODO: imgui ids
                    ImGui::PushID(i + 123423);
                    selected = ImGui::Button(info->name, ImVec2(50, 50));
                    ImGui::PopID();
                }
                if (selected) {
                    UIDragItem(ui, player->id, player->toolbelt, i);
                }
            }

            auto posAfter = ImGui::GetCursorPos();
            ImGui::PushID(i);
            ImGui::SameLine();
            ImVec4 color = {1.0f, 1.0f, 1.0f, 1.0f};
            if (player->toolbeltSelectIndex == i) {
                color = {1.0f, 0.0f, 0.0f, 1.0f};
            }
            ImGui::TextColored(color, "%3lu", it->count);
            ImGui::PopID();
            ImGui::PopID();
            i++;
        }
    }
    ImGui::End();
}

void PlayerUpdateAndRender(Entity* _entity, EntityBehaviorInvoke reason, void* _data) {
    if (reason == EntityBehaviorInvoke::UpdateAndRender) {
        auto data = (EntityUpdateAndRenderData*)_data;
        auto entity = (Player*)_entity;
        auto oldP = entity->p;
        v3 frameAcceleration = {};


        {
            auto world = GetWorld();
            auto playerChunk = GetChunk(world, WorldPos::ToChunk(entity->p.block).chunk);
            DEBUG_OVERLAY_TRACE(playerChunk->simPropagationCount);
        }

        f32 playerAcceleration;
        v3 drag = entity->velocity * entity->friction;

        auto z = Normalize(V3(data->camera->front.x, 0.0f, data->camera->front.z));
        auto x = Normalize(Cross(V3(0.0f, 1.0f, 0.0f), z));
        auto y = V3(0.0f, 1.0f, 0.0f);

        if (entity->camera->inputMode == GameInputMode::Game || entity->camera->inputMode == GameInputMode::InGameUI) {

            if (KeyHeld(Key::W)) {
                frameAcceleration -= z;
            }
            if (KeyHeld(Key::S)) {
                frameAcceleration += z;
            }
            if (KeyHeld(Key::A)) {
                frameAcceleration -= x;
            }
            if (KeyHeld(Key::D)) {
                frameAcceleration += x;
            }

            if ((KeyHeld(Key::Space))) {
                //frameAcceleration += y;
            }

            if (KeyPressed(Key::Y)) {
                entity->flightMode = !entity->flightMode;
            }

            if (entity->flightMode) {
                if (KeyHeld(Key::Space)) {
                    frameAcceleration += y;
                }
                if (KeyHeld(Key::Ctrl)) {
                    frameAcceleration -= y;
                }
            } else {
                drag.y = 0.0f;
            }

            if ((KeyHeld(Key::Shift))) {
                playerAcceleration = entity->runAcceleration;
            } else {
                playerAcceleration = entity->acceleration;
            }
            frameAcceleration *= playerAcceleration;
        }

        // TODO: Physically correct friction
        frameAcceleration -= drag;

        if (!entity->flightMode) {
            if (data->camera->inputMode == GameInputMode::Game || data->camera->inputMode == GameInputMode::InGameUI) {
                if (KeyPressed(Key::Space) && entity->grounded) {
                    frameAcceleration += y * entity->jumpAcceleration * (1.0f / data->deltaTime) / 60.0f;
                }
            }

            frameAcceleration.y += -20.8f;
        }


        v3 movementDelta = 0.5f * frameAcceleration * data->deltaTime * data->deltaTime + entity->velocity * data->deltaTime;

        entity->velocity += frameAcceleration * data->deltaTime;
        DEBUG_OVERLAY_TRACE(entity->velocity);
        bool hitGround = false;
        MoveSpatialEntity(entity->world, entity, movementDelta, data->camera, nullptr);

        if (WorldPos::ToChunk(entity->p).chunk != WorldPos::ToChunk(oldP).chunk) {
            MoveRegion(&entity->world->chunkPool.playerRegion, WorldPos::ToChunk(entity->p).chunk);
        }

        PlayerDrawToolbelt(entity);

        auto context = GetContext();
        if (entity->camera->mode != CameraMode::Gameplay) {
            RenderCommandDrawMesh command{};
            command.transform = Translate(WorldPos::Relative(data->camera->targetWorldPosition, entity->p));
            command.mesh = context->cubeMesh;
            command.material = &context->playerMaterial;
            Push(data->group, &command);
        }
    }
}

void PlayerProcessOverlap(GameWorld* world, SpatialEntity* testEntity, SpatialEntity* overlappedEntity) {
    if (overlappedEntity->type == EntityType::Pickup) {
        assert(testEntity->type == EntityType::Player);
        auto player = (Player*)testEntity;
        auto pickup = static_cast<Pickup*>(overlappedEntity);
        assert(testEntity->inventory);
        auto itemRemainder = EntityInventoryPushItem(player->toolbelt, (Item)pickup->item, pickup->count);
        if (itemRemainder) {
            itemRemainder = EntityInventoryPushItem(testEntity->inventory, (Item)pickup->item, pickup->count);
        }
        if (itemRemainder == 0) {
            ScheduleEntityForDelete(world, overlappedEntity);
        } else {
            pickup->count = itemRemainder;
        }
    }
}

void PlayerUpdateAndRenderUI(Entity* entity, EntityUIInvoke reason) {
    if (reason == EntityUIInvoke::Inventory) {
        auto context = GetContext();
        UIDrawInventory(&context->ui, entity, entity->inventory);
    }
}
