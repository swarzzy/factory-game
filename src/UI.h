#pragma once

#include "../ext/imgui/imgui.h"
#include "Resource.h"
#include "Renderer.h"

struct Context;
struct Player;
struct Camera;
struct EntityInventory;

struct UI {
    EntityID entityList[2];
    EntityID dragEntityID;
    EntityInventory* dragEntityInventory;
    u32 dragItemIndex;
};

void UIDragItem(UI* ui, EntityID id, EntityInventory* inventory, u32 index);
void UIDropItem(UI* ui, EntityInventory* dropInventory, u32 index);
void UIClearDragAndDrop(UI* ui);

void UIDrawEntityInfo(UI* ui, Entity* entity);
void UIDrawBlockInfo(UI* ui, const Block* block);

bool UIOpenForEntity(UI* ui, EntityID id);
void UICloseAll(UI* ui);
bool UIHasOpen(UI* ui);

void UIDrawInventory(UI* ui, Entity* entity, EntityInventory* inventory);

void UIUpdateAndRender(UI* ui);

void UIInit(UI* ui);
