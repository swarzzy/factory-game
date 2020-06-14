#include "UI.h"
#include "Player.h"

#include "Inventory.h"

void UIDrawEntityInfo(UI* ui, Entity* entity) {
    ImGuiIO& io = ImGui::GetIO();
    ImVec2 windowPos = ImVec2(io.DisplaySize.x / 2, 0);
    ImVec2 windowPivot = ImVec2(0.5f, 0.0f);
    ImGui::SetNextWindowPos(windowPos, ImGuiCond_Always, windowPivot);
    ImGui::SetNextWindowBgAlpha(0.35f);
    bool open = true;
    if (ImGui::Begin("entity info", &open, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoNav)) {
        auto entityInfo = GetEntityInfo(entity->type);
        ImGui::Text("%s", entityInfo->name);
        ImGui::Text("id: %llu", entity->id);
        ImGui::Text("kind: %s", ToString(entity->kind));
        ImGui::Text("has inventory: %s", entity->inventory ? "true" : "false");
        switch (entity->kind) {
        case EntityKind::Block: {
            auto blockEntity = static_cast<BlockEntity*>(entity);
            ImGui::Text("pos: (%ld, %ld, %ld)", blockEntity->p.x, blockEntity->p.y, blockEntity->p.z);
            if (entityInfo->UpdateAndRenderUI) {
                entityInfo->UpdateAndRenderUI(entity, EntityUIInvoke::Info);
            }
        } break;
        case EntityKind::Spatial: {
            auto spatialEntity = static_cast<SpatialEntity*>(entity);
            ImGui::Text("pos block: (%ld, %ld, %ld)", spatialEntity->p.block.x, spatialEntity->p.block.y, spatialEntity->p.block.z);
            ImGui::Text("pos offset: (%f, %f, %f)", spatialEntity->p.offset.x, spatialEntity->p.offset.y, spatialEntity->p.offset.z);
            if (entityInfo->UpdateAndRenderUI) {
                entityInfo->UpdateAndRenderUI(entity, EntityUIInvoke::Info);
            }
        } break;
        default: {} break;
        }
    }
    ImGui::End();
}

void UIDrawBlockInfo(UI* ui, const Block* block) {
    ImGuiIO& io = ImGui::GetIO();
    ImVec2 windowPos = ImVec2(io.DisplaySize.x / 2, 0);
    ImVec2 windowPivot = ImVec2(0.5f, 0.0f);
    ImGui::SetNextWindowPos(windowPos, ImGuiCond_Always, windowPivot);
    ImGui::SetNextWindowBgAlpha(0.35f);
    bool open = true;
    if (ImGui::Begin("entity info", &open, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoNav)) {
        auto info = GetBlockInfo(block->value);
        ImGui::Text("block: %s", info->name);
        if (block->entity) {
            auto entityInfo = GetEntityInfo(block->entity->type);
            ImGui::Text("living entity: %s (id:%llu)", entityInfo->name, (u64)block->entity->id);
        } else {
            ImGui::Text("living entity: null");
        }
    }
    ImGui::End();
}

bool UIOpenForEntity(UI* ui, EntityID id) {
    bool opened = false;
    for (usize i = 0; i < array_count(ui->entityList); i++) {
        if (ui->entityList[i] == 0) {
            ui->entityList[i] = id;
            opened = true;
            break;
        }
    }
    return opened;
}

void UICloseAll(UI* ui) {
    for (usize i = 0; i < array_count(ui->entityList); i++) {
        ui->entityList[i] = 0;
    }
    UIClearDragAndDrop(ui);
}

bool UIHasOpen(UI* ui) {
    bool hasOpen = false;
    for (usize i = 0; i < array_count(ui->entityList); i++) {
        if (ui->entityList[i]) {
            hasOpen = true;
            break;
        }
    }
    return hasOpen;
}

void UIDragItem(UI* ui, EntityID id, EntityInventory* inventory, u32 index) {
    auto world = GetWorld();
    if (GetEntity(world, id) && inventory && (index < inventory->slotCount)) {
        ui->dragEntityID = id;
        ui->dragEntityInventory = inventory;
        ui->dragItemIndex = index;
    }
}

void UIClearDragAndDrop(UI* ui) {
    ui->dragEntityID = 0;
    ui->dragEntityInventory = nullptr;
    ui->dragItemIndex = U32::Max;
}

void UIDropItem(UI* ui, EntityInventory* dropInventory, u32 index) {
    auto world = GetWorld();
    if (GetEntity(world, ui->dragEntityID) && (ui->dragItemIndex < ui->dragEntityInventory->slotCount)) {
        if (index < dropInventory->slotCount) {
            auto dropSlot = dropInventory->slots + index;
            if (dropSlot->item == Item::None) {
                assert(dropSlot->count == 0);
                auto dragSlot = ui->dragEntityInventory->slots + ui->dragItemIndex;
                u32 free = dropInventory->slotCapacity - dropSlot->count;
                u32 remainder = 0;
                u32 transferCount = dragSlot->count;
                if (free < dragSlot->count) {
                    transferCount = dropInventory->slotCapacity;
                    remainder = dragSlot->count - free;
                }
                dropSlot->item = dragSlot->item;
                dropSlot->count = transferCount;
                dragSlot->count = remainder;
                if (!remainder) {
                    dragSlot->item = Item::None;
                }
                UIClearDragAndDrop(ui);
            }
        }
    }
}

void UIUpdateAndRender(UI* ui) {
    auto world = GetWorld();
    auto camera = world->camera;
    bool showingEntities = false;
    float windowOffset = 50.0f;
    for (usize i = 0; i < array_count(ui->entityList); i++) {
        auto id = ui->entityList[i];
        if (id) {
            if (!showingEntities) {
                if (camera->inputMode == GameInputMode::Game) {
                    camera->inputMode = GameInputMode::InGameUI;
                }
            }
            showingEntities = true;
            auto entity = GetEntity(world, id);
            if (entity) {
                auto info = GetEntityInfo(entity->type);
                if (info->UpdateAndRenderUI) {
                    ImGui::SetNextWindowSize({560.0f, 400.0f});
                    ImGui::SetNextWindowPos({windowOffset, 150.0f});
                    windowOffset += 560.0f + 50.0f;
                    auto windowFlags = 0; //ImGuiWindowFlags_NoResize; //ImGuiWindowFlags_AlwaysAutoResize;
                    char buffer[64];
                    sprintf_s(buffer, 64, "%s##%llu", info->name, (u64)entity->id);
                    bool open = true;
                    if (ImGui::Begin(buffer, &open, windowFlags)) {
                        info->UpdateAndRenderUI(entity, EntityUIInvoke::Inventory);
                    }
                    ImGui::End();
                }
            } else {
                ui->entityList[i] = 0;
                UIClearDragAndDrop(ui);
            }
        }
    }
    if (!showingEntities) {
        if (camera->inputMode == GameInputMode::InGameUI) {
            camera->inputMode = GameInputMode::Game;
        }
    }
}

void UIDrawInventory(UI* ui, Entity* entity, EntityInventory* inventory) {
    if (ImGui::BeginChild("inventory")) {
        u32 i = 0;
        ForEach(inventory, [&](auto* it) {
            if (!(i % 6 == 0)) {
                ImGui::SameLine();
            }
            ImGui::PushID(it);
            bool selected;
            if (it->item == Item::None) {
                // TODO: id
                ImGui::PushID(i + 123423);
                if (ImGui::Button("", ImVec2(50, 50))) {
                    UIDropItem(ui, inventory, i);
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
                    UIDragItem(ui, entity->id, inventory, i);
                }
            }

            auto posAfter = ImGui::GetCursorPos();
            ImGui::PushID(i);
            ImGui::SameLine();
            ImGui::Text("%3lu", it->count);
            ImGui::PopID();
            ImGui::PopID();
            i++;
        });
    }
    ImGui::EndChild();
}

void UIInit(UI* ui) {
    ui->dragItemIndex = U32::Max;
}
