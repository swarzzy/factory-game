#include "UI.h"

bool OpenInventoryForEntity(UI* ui, Context* context, EntityID id) {
    bool opened = false;
    auto entity = GetBlockEntity(&context->playerRegion, id);
    if (entity) {
        ui->entityToOpenInventoryFor = id;
        opened = true;
    }
    return opened;
}

void TickEntityUI(UI* ui, BlockEntity* entity) {
    if (entity->inventory) {
        ImGui::SetNextWindowSize({640, 400});
        ImGui::SetNextWindowPos({300, 200});
        auto windowFlags = ImGuiWindowFlags_NoResize; //ImGuiWindowFlags_AlwaysAutoResize;
        bool open = true;
        if (ImGui::Begin("Entity inventory", &open, windowFlags)) {
            if (!open) {
                CloseEntityInventory(ui);
            }
            if (ImGui::BeginChild("inventory")) {
                auto inventory = entity->inventory;
                u32 i = 0;
                foreach (*inventory) {
                    if (!(i % 7 == 0)) {
                        ImGui::SameLine();
                    }
                    ImGui::PushID(it);
                    switch (it->item) {
                    case Item::CoalOre: {
                        ImGui::ImageButton((void*)(uptr)ui->coalIcon.gpuHandle, ImVec2(50, 50), ImVec2(0,0), ImVec2(1,1), 0);
                    } break;
                    case Item::Container: {
                        ImGui::ImageButton((void*)(uptr)ui->containerIcon.gpuHandle, ImVec2(50, 50), ImVec2(0,0), ImVec2(1,1), 0);
                    } break;
                    case Item::None: {
                        ImGui::Button("", ImVec2(50, 50));
                    } break;
                    default: {
                        ImGui::Button(ToString(it->item), ImVec2(50, 50));
                    } break;
                    }
                    auto posAfter = ImGui::GetCursorPos();
                    ImGui::PushID(i);
                    ImGui::SameLine();
                    ImGui::Text("%3lu", it->count);
                    ImGui::PopID();
                    ImGui::PopID();
                    i++;
                }
            }
            ImGui::EndChild();
        }
        ImGui::End();
    }
}

void TickUI(UI* ui, Context* context) {
    BlockEntity* blockEntity = nullptr;
    if (ui->entityToOpenInventoryFor != EntityID {}) {
        auto entity = GetBlockEntity(&context->playerRegion, ui->entityToOpenInventoryFor);
        if (entity) {
            blockEntity = entity;
        } else {
            CloseEntityInventory(ui);
        }
    }

    auto camera = &context->camera;

    if (ui->openPlayerInventory || blockEntity) {
        if (camera->inputMode == GameInputMode::Game) {
            camera->inputMode = GameInputMode::InGameUI;
        }
    } else {
        if (camera->inputMode == GameInputMode::InGameUI) {
            camera->inputMode = GameInputMode::Game;
        }
    }

    if (blockEntity) {
        TickEntityUI(ui, blockEntity);
    }

    if (ui->openPlayerInventory) {
        ImGui::SetNextWindowSize({640, 400});
        ImGui::SetNextWindowPos({300, 200});
        auto windowFlags = ImGuiWindowFlags_NoResize; //ImGuiWindowFlags_AlwaysAutoResize;
        if (ImGui::Begin("Player inventory", &ui->openPlayerInventory, windowFlags)) {
            if (ImGui::BeginChild("inventory")) {
                auto entity = GetSpatialEntity(ui->player->region, ui->player->entityID);
                assert(entity);
                auto inventory = entity->inventory;
                u32 i = 0;
                foreach (*inventory) {
                    if (!(i % 7 == 0)) {
                        ImGui::SameLine();
                    }
                    ImGui::PushID(it);
                    switch (it->item) {
                    case Item::CoalOre: {
                        ImGui::ImageButton((void*)(uptr)ui->coalIcon.gpuHandle, ImVec2(50, 50), ImVec2(0,0), ImVec2(1,1), 0);
                    } break;
                    case Item::Container: {
                        ImGui::ImageButton((void*)(uptr)ui->containerIcon.gpuHandle, ImVec2(50, 50), ImVec2(0,0), ImVec2(1,1), 0);
                    } break;
                    case Item::None: {
                        ImGui::Button("", ImVec2(50, 50));
                    } break;
                    default: {
                        ImGui::Button(ToString(it->item), ImVec2(50, 50));
                    } break;
                    }
                    auto posAfter = ImGui::GetCursorPos();
                    ImGui::PushID(i);
                    ImGui::SameLine();
                    ImGui::Text("%3lu", it->count);
                    ImGui::PopID();
                    ImGui::PopID();
                    i++;
                }
            }
            ImGui::EndChild();
        }
        ImGui::End();
    }
}
