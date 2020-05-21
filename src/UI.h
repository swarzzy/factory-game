#pragma once

#include "../ext/imgui/imgui.h"
#include "Resource.h"

struct Player;
struct Camera;

struct UI {
    Camera* camera; // Only for input mode
    Player* player;
    bool openPlayerInventory;
    Texture coalIcon;
    Texture containerIcon;
    EntityID entityToOpenInventoryFor;
};

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

bool OpenInventoryForEntity(UI* ui, Context* context, EntityID id);

void CloseEntityInventory(UI* ui) {
    ui->entityToOpenInventoryFor = {};
}

void TickUI(UI* ui, Context* context);
