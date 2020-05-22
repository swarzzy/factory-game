#pragma once

#include "../ext/imgui/imgui.h"
#include "Resource.h"
#include "Renderer.h"

struct Context;
struct Player;
struct Camera;

struct UI {
    Camera* camera; // Only for input mode
    Player* player;
    bool openPlayerInventory;
    Texture coalIcon;
    Texture containerIcon;
    EntityID entityToOpenInventoryFor;
    bool itemSelected;
    bool selectedInPlayer;
    u32 selectedItemSlotIndex;
};

void UISelectItem(UI* ui, bool inPlayerInventory, u32 slotIndex) {
    ui->itemSelected = true;
    ui->selectedInPlayer = inPlayerInventory;
    ui->selectedItemSlotIndex =  slotIndex;
}


void InitUI(UI* ui, Player* player, Camera* camera) {
    ui->player = player;
    ui->camera = camera;

    ui->coalIcon = LoadTextureFromFile("../res/coal_icon.png", TextureFormat::SRGB8, TextureWrapMode::Default, TextureFilter::None, DynamicRange::LDR);
    assert(ui->coalIcon.base);
    UploadToGPU(&ui->coalIcon);

    ui->containerIcon = LoadTextureFromFile("../res/chest_icon.png", TextureFormat::SRGB8, TextureWrapMode::Default, TextureFilter::None, DynamicRange::LDR);
    assert(ui->containerIcon.base);
    UploadToGPU(&ui->containerIcon);

}

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
