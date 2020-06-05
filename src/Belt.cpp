#include "Belt.h"


Entity* CreateBelt(GameWorld* world, WorldPos p) {
    Belt* belt = AddBlockEntity<Belt>(world, p.block);
    if (belt) {
        belt->type = EntityType::Belt;
        belt->dirtyNeighborhood = true;
        belt->direction = Direction::North;
        belt->beltTrait.InsertItem = BeltInsertItem;
        belt->beltTrait.GrabItem = BeltGrabItem;
    }
    return belt;
}

void BeltDelete(Entity* entity, GameWorld* world) {
}

void BeltDropPickup(Entity* entity, GameWorld* world, WorldPos p) {
    auto belt = (Belt*)entity;
    auto pickup = (Pickup*)CreatePickupEntity(world, p);
    pickup->item = Item::Belt;
    pickup->count = 1;
    for (usize i = 0; i < array_count(belt->items); i++) {
        if (belt->items[i] != 0) {
            auto pickup = (Pickup*)CreatePickupEntity(world, p);
            pickup->item = (Item)belt->items[i];
            pickup->count = 1;
        }
    }
}

void BeltUpdateAndRender(Belt* belt, void* _data) {
    auto data = (EntityUpdateAndRenderData*)_data;

    bool lateClearLastItemSlot = false;
    for (usize i = 0; i < Belt::Capacity; i++) {
        if (belt->items[i] != 0) {
            f32 max = (i + 1) * (1.0f / array_count(belt->items));
            f32 maxPrev = (i) * (1.0f / array_count(belt->items));
            belt->itemPositions[i] = belt->itemPositions[i] + data->deltaTime * Belt::Speed;
            if (belt->itemHorzPositions[i] != 0.0f) {
                belt->itemHorzPositions[i] -= data->deltaTime * Belt::HorzSpeed;
                if (belt->itemHorzPositions[i] < 0.0f) {
                    belt->itemHorzPositions[i] = 0.0f;
                }
            }
            if (belt->itemPositions[i] > max) {
                bool passedForward = false;
                if (i < Belt::Capacity - 1) {
                    if (belt->items[i + 1] == 0) {
                        belt->itemPositions[i + 1] = belt->itemPositions[i];
                        belt->items[i + 1] = belt->items[i];
                        belt->items[i] = 0;
                        i++;
                    } else {
                        belt->itemPositions[i] = max;
                    }
                } else {
                    auto to = belt->p + Dir::ToIV3(belt->direction);
                    auto toEntity = GetEntity(belt->world, to);
                    if (toEntity) {
                        auto beltTrait = FindEntityTrait<BeltTrait>(toEntity);
                        if (beltTrait) {
                            assert(beltTrait->InsertItem);
                            bool passed = beltTrait->InsertItem(toEntity, belt->direction, belt->items[i], belt->itemPositions[i]);
                            if (passed) {
                                passedForward = true;
                                if (belt->generation > toEntity->generation) {
                                    belt->items[Belt::LastSlot] = 0;
                                } else {
                                    lateClearLastItemSlot = true;
                                }
                            }
                        }
                    }
                }
                if (!passedForward) {
                    belt->itemPositions[i] = max;
                }
            }
        }
    }

    auto context = GetContext();
    RenderCommandDrawMesh command {};
    command.transform = Translate(WorldPos::Relative(data->camera->targetWorldPosition, WorldPos::Make(belt->p))) * M4x4(RotateY(Dir::AngleDegY(Direction::North, belt->direction)));
    command.mesh = context->beltStraightMesh;
    command.material = &context->beltMaterial;
    Push(data->group, &command);

    for (usize i = 0; i < Belt::Capacity; i++) {
        if (belt->items[i] != 0) {
            v3 dir = V3(Dir::ToIV3(belt->direction));
            v3 horzDir = V3(Dir::ToIV3(belt->itemTurnDirections[i]));
            v3 itemBegin = V3(Dir::ToIV3(Dir::Opposite(belt->direction))) * 0.5f;
            v3 itemOffset = itemBegin + dir * belt->itemPositions[i] - V3(0.0f, Belt::ItemSink, 0.0f) + horzDir * belt->itemHorzPositions[i] * 0.5f;
            auto info = GetItemInfo(belt->items[i]);
            RenderCommandDrawMesh command {};
            command.transform = Translate(WorldPos::Relative(data->camera->targetWorldPosition, WorldPos::Make(belt->p, itemOffset)));
            command.mesh = info->mesh;
            command.material = info->material;
            Push(data->group, &command);
        }
    }

    if (lateClearLastItemSlot) {
        belt->items[Belt::Capacity - 1] = 0;
    }

}

void BeltRotate(Belt* belt, void* _data) {
    auto data = (EntityRotateData*)_data;
    belt->direction = Dir::RotateYCW(belt->direction);
    PostEntityNeighborhoodUpdate(belt->world, belt);
}

void BeltBehavior(Entity* entity, EntityBehaviorInvoke reason, void* data) {
    auto belt = (Belt*)entity;
    switch (reason) {
    case EntityBehaviorInvoke::UpdateAndRender: { BeltUpdateAndRender(belt, data); } break;
    case EntityBehaviorInvoke::Rotate: { BeltRotate(belt, data); } break;
    default: {} break;
    }
}

bool BeltInsertItem(Entity* entity, Direction dir, u32 itemID, f32 callerItemPos) {
    bool result = false;
    auto belt = (Belt*)entity;

    u32 index = U32::Max;
    f32 horzPos = 0.0f;
    Direction turnDir = dir;
    if (dir == belt->direction) {
        if (belt->items[Belt::FirstSlot] == 0) {
            index = Belt::FirstSlot;
        }
    } else if (dir != Dir::Opposite(belt->direction)) {
        u32 midSlot = Belt::Capacity / 2 + 1;
        if (belt->items[midSlot] == 0) {
            index = midSlot;
            horzPos = 1.0f;
            turnDir = Dir::Opposite(dir);
        }
    }
    if (index != U32::Max) {
        belt->items[index] = itemID;
        belt->itemPositions[index] = callerItemPos - 1.0f + (index * (1.0f / Belt::Capacity));
        belt->itemHorzPositions[index] = horzPos;
        belt->itemTurnDirections[index] = turnDir;
        result = true;
    }
    return result;
}

u32 BeltGrabItem(Entity* entity, Direction dir) {
    u32 result = 0;
    auto belt = (Belt*)entity;
    if (belt->direction == dir) {
        if (belt->items[Belt::LastSlot] && belt->itemPositions[Belt::LastSlot] == 1.0f) {
            result = belt->items[Belt::LastSlot];
            belt->items[Belt::LastSlot] = 0;
        }
    }
    return result;
}
