#include "UI.h"

bool OpenInventoryForEntity(UI* ui, Context* context, EntityID id) {
    bool opened = false;
    auto entity = GetBlockEntity(&context->playerRegion, id);
    if (entity && entity->inventory) {
        ui->entityToOpenInventoryFor = id;
        opened = true;
    }
    return opened;
}

bool UIMoveItemSlot(EntityInventory* from, EntityInventory* to, u32 slotIndexFrom, u32 slotIndexTo) {
    bool moved = false;
    if (from->slotCount > slotIndexFrom && to->slotCount > slotIndexTo) {
        InventorySlot* slotFrom = from->slots + slotIndexFrom;
        InventorySlot* slotTo = to->slots + slotIndexTo;
        InventorySlot tmp = *slotFrom;
        *slotFrom = *slotTo;
        *slotTo = tmp;
        moved = true;
    }
    return moved;
}

void TickEntityUI(UI* ui, Context* context, BlockEntity* entity) {
    if (entity->inventory) {
        ImGui::SetNextWindowSize({560, 400});
        //ImGui::SetNextWindowPos({300, 200});
        auto windowFlags = 0; //ImGuiWindowFlags_NoResize; //ImGuiWindowFlags_AlwaysAutoResize;
        bool open = true;
        if (ImGui::Begin("Entity inventory", &open, windowFlags)) {
            if (!open) {
                CloseEntityInventory(ui);
            }
            if (ImGui::BeginChild("inventory")) {
                auto inventory = entity->inventory;
                u32 i = 0;
                foreach (*inventory) {
                    if (!(i % 6 == 0)) {
                        ImGui::SameLine();
                    }
                    ImGui::PushID(it);
                    bool selected = false;
                    switch (it->item) {
                    case Item::CoalOre: {
                        selected = ImGui::ImageButton((void*)(uptr)ui->coalIcon.gpuHandle, ImVec2(50, 50), ImVec2(0,0), ImVec2(1,1), 0);
                    } break;
                    case Item::Container: {
                        selected = ImGui::ImageButton((void*)(uptr)ui->containerIcon.gpuHandle, ImVec2(50, 50), ImVec2(0,0), ImVec2(1,1), 0);
                    } break;
                    case Item::None: {
                        if (ImGui::Button("", ImVec2(50, 50))) {
                            if (ui->itemSelected) {
                                if (ui->selectedInPlayer) {
                                    SpatialEntity* player = GetSpatialEntity(&context->playerRegion, ui->player->entityID);
                                    assert(player);
                                    UIMoveItemSlot(player->inventory, inventory, ui->selectedItemSlotIndex, i);
                                } else {
                                    UIMoveItemSlot(inventory, inventory, ui->selectedItemSlotIndex, i);
                                }
                            }
                        }
                    } break;
                    default: {
                        selected = ImGui::Button(ToString(it->item), ImVec2(50, 50));
                    } break;
                    }
                    if (selected) {
                        UISelectItem(ui, false, i);
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
        TickEntityUI(ui, context, blockEntity);
    }

    if (ui->openPlayerInventory) {
        ImGui::SetNextWindowSize({560, 400});
        //ImGui::SetNextWindowPos({300, 200});
        auto windowFlags = 0; //ImGuiWindowFlags_NoResize; //ImGuiWindowFlags_AlwaysAutoResize;
        if (ImGui::Begin("Player inventory", &ui->openPlayerInventory, windowFlags)) {
            if (ImGui::BeginChild("inventory")) {
                auto entity = GetSpatialEntity(ui->player->region, ui->player->entityID);
                assert(entity);
                auto inventory = entity->inventory;
                u32 i = 0;
                foreach (*inventory) {
                    if (!(i % 6 == 0)) {
                        ImGui::SameLine();
                    }
                    ImGui::PushID(it);
                    bool selected = false;
                    switch (it->item) {
                    case Item::CoalOre: {
                        selected = ImGui::ImageButton((void*)(uptr)ui->coalIcon.gpuHandle, ImVec2(50, 50), ImVec2(0,0), ImVec2(1,1), 0);
                    } break;
                    case Item::Container: {
                        selected = ImGui::ImageButton((void*)(uptr)ui->containerIcon.gpuHandle, ImVec2(50, 50), ImVec2(0,0), ImVec2(1,1), 0);
                    } break;
                    case Item::None: {
                        if (ImGui::Button("", ImVec2(50, 50))) {
                            if (ui->itemSelected) {
                                if (!ui->selectedInPlayer) {
                                    BlockEntity* entity = GetBlockEntity(&context->playerRegion, ui->entityToOpenInventoryFor);
                                    assert(entity);
                                    UIMoveItemSlot(entity->inventory, inventory, ui->selectedItemSlotIndex, i);
                                } else {
                                    UIMoveItemSlot(inventory, inventory, ui->selectedItemSlotIndex, i);
                                }
                            }
                        }
                    } break;
                    default: {
                        selected = ImGui::Button(ToString(it->item), ImVec2(50, 50));
                    } break;
                    }
                    if (selected) {
                        UISelectItem(ui, true, i);
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
