#pragma once

#include "../ext/imgui/imgui.h"
#include "Resource.h"
#include "Renderer.h"

struct Context;
struct Player;
struct Camera;

struct UI {
    Camera* camera; // Only for input mode
    EntityID player;
    bool openPlayerInventory;
    Texture coalIcon;
    Texture containerIcon;
    EntityID entityToOpenInventoryFor;
    bool itemSelected;
    bool selectedInPlayer;
    u32 selectedItemSlotIndex;
    SimRegion* region;
};

void UISelectItem(UI* ui, bool inPlayerInventory, u32 slotIndex) {
    ui->itemSelected = true;
    ui->selectedInPlayer = inPlayerInventory;
    ui->selectedItemSlotIndex =  slotIndex;
}


void InitUI(UI* ui, Player* player, Camera* camera);
void OpenPlayerInventory(UI* ui) {
    ui->openPlayerInventory = true;
}

void ClosePlayerInventory(UI* ui) {
    ui->openPlayerInventory = false;
    if (ui->itemSelected && ui->selectedInPlayer) {
        ui->itemSelected = false;
    }
}

bool OpenInventoryForEntity(UI* ui, Context* context, EntityID id);

void CloseEntityInventory(UI* ui) {
    ui->entityToOpenInventoryFor = {};
    if (ui->itemSelected && !ui->selectedInPlayer) {
        ui->itemSelected = false;
    }
}

void TickUI(UI* ui, Context* context);

// TODO: Both entity classes
void DrawEntityInfo(UI* ui, Entity* blockEntity);
