#include "Belt.h"

void OrientBelt(Belt* belt) {
    i32 directions[4] {};

    NeighborIterator iter = NeighborIterator::Begin(belt->p);
    while (!NeighborIterator::Ended(&iter)) {
        auto voxel = NeighborIterator::Get(&iter);
        Direction dir = NeighborIterator::GetCurrentDirection(&iter);
        if (voxel) {
            if (voxel->entity) {
                // TODO: Actually use  trait
                auto neighborBelt = FindEntityTrait<BeltTrait>(voxel->entity);
                if (neighborBelt) {
                    if (neighborBelt->direction == dir || Dir::Opposite(neighborBelt->direction) == dir) {
                        belt->belt.direction = neighborBelt->direction;
                        break;
                    } else {
                        belt->belt.direction = dir;
                        break;
                    }
                }
            }
        }
        NeighborIterator::Advance(&iter);
    }

    i32 max = -1;
    i32 maxIndex = -1;

    for (usize i = 0; i < array_count(directions); i++) {
        if (directions[i] == max) {
            maxIndex = -1;
            break;
        }
        if (directions[i] > max) {
            max = directions[i];
            maxIndex = i;
        }
    }

    if (maxIndex != -1) {
        belt->belt.direction = (Direction)directions[maxIndex];
    }


}

Entity* CreateBelt(GameWorld* world, WorldPos p) {
    Belt* belt = AddBlockEntity<Belt>(world, p.block);
    if (belt) {
        belt->type = EntityType::Belt;
        belt->dirtyNeighborhood = true;
        belt->belt.direction = Direction::North;
        belt->belt.InsertItem = BeltInsertItem;
        belt->belt.GrabItem = BeltGrabItem;
        OrientBelt(belt);
    }
    return belt;
}

void BeltDelete(Entity* entity, GameWorld* world) {
}

void BeltDropPickup(Entity* entity, GameWorld* world, WorldPos p) {
    auto belt = (Belt*)entity;
    auto pickup = CreatePickup(p, (u32)Item::Belt, 1);
    for (usize i = 0; i < array_count(belt->belt.items); i++) {
        if (belt->belt.items[i] != 0) {
            auto pickup = CreatePickup(p, belt->belt.items[i], 1);
        }
    }
}

void BeltRotate(Belt* belt, void* _data) {
    auto data = (EntityRotateData*)_data;
    belt->belt.direction = Dir::RotateYCW(belt->belt.direction);
    PostEntityNeighborhoodUpdate(belt->world, belt);
}

void BeltUpdateAndRender(Belt* belt, void* _data) {
    auto data = (EntityUpdateAndRenderData*)_data;

    if (belt->dirtyNeighborhood) {
        //OrientBelt(belt);
        belt->dirtyNeighborhood = false;
    }

    auto trait = &belt->belt;

    bool lateClearLastItemSlot = false;
    for (usize i = 0; i < BeltTrait::Capacity; i++) {
        if (trait->items[i] != 0) {
            f32 max = (i + 1) * (1.0f / array_count(trait->items));
            f32 maxPrev = (i) * (1.0f / array_count(trait->items));
            trait->itemPositions[i] = trait->itemPositions[i] + data->deltaTime * BeltTrait::Speed;
            if (trait->itemHorzPositions[i] != 0.0f) {
                trait->itemHorzPositions[i] -= data->deltaTime * BeltTrait::HorzSpeed;
                if (trait->itemHorzPositions[i] < 0.0f) {
                    trait->itemHorzPositions[i] = 0.0f;
                }
            }
            if (trait->itemPositions[i] > max) {
                bool passedForward = false;
                if (i < BeltTrait::Capacity - 1) {
                    if (trait->items[i + 1] == 0) {
                        trait->itemPositions[i + 1] = trait->itemPositions[i];
                        trait->items[i + 1] = trait->items[i];
                        trait->items[i] = 0;
                        i++;
                    } else {
                        trait->itemPositions[i] = max;
                    }
                } else {
                    auto to = belt->p + Dir::ToIV3(trait->direction);
                    auto toEntity = GetEntity(belt->world, to);
                    if (toEntity) {
                        auto toBelt = FindEntityTrait<BeltTrait>(toEntity);
                        if (toBelt) {
                            assert(toBelt->InsertItem);
                            bool passed = toBelt->InsertItem(toEntity, trait->direction, trait->items[i], trait->itemPositions[i]);
                            if (passed) {
                                passedForward = true;
                                if (belt->generation > toEntity->generation) {
                                    trait->items[BeltTrait::LastSlot] = 0;
                                } else {
                                    lateClearLastItemSlot = true;
                                }
                            }
                        }
                    }
                }
                if (!passedForward) {
                    trait->itemPositions[i] = max;
                }
            }
        }
    }

    auto context = GetContext();
    RenderCommandDrawMesh command {};
    command.transform = Translate(WorldPos::Relative(data->camera->targetWorldPosition, WorldPos::Make(belt->p))) * M4x4(RotateY(Dir::AngleDegY(Direction::North, belt->belt.direction)));
    command.mesh = context->beltStraightMesh;
    command.material = &context->beltMaterial;
    Push(data->group, &command);

    for (usize i = 0; i < BeltTrait::Capacity; i++) {
        if (belt->belt.items[i] != 0) {
            // TODO: Cache align and scale
            auto info = GetItemInfo(belt->belt.items[i]);
            v3 dir = V3(Dir::ToIV3(belt->belt.direction));
            v3 horzDir = V3(Dir::ToIV3(belt->belt.itemTurnDirections[i]));
            v3 itemBegin = V3(Dir::ToIV3(Dir::Opposite(belt->belt.direction))) * 0.5f;
            v3 itemOffset = itemBegin + dir * belt->belt.itemPositions[i] - V3(0.0f, info->beltAlign, 0.0f) + horzDir * belt->belt.itemHorzPositions[i] * 0.5f;
            RenderCommandDrawMesh command {};
            command.transform = Translate(WorldPos::Relative(data->camera->targetWorldPosition, WorldPos::Make(belt->p, itemOffset))) * Scale(V3(info->beltScale));
            command.mesh = info->mesh;
            command.material = info->material;
            Push(data->group, &command);
        }
    }

    if (lateClearLastItemSlot) {
        belt->belt.items[BeltTrait::Capacity - 1] = 0;
    }

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
    if (dir == belt->belt.direction) {
        if (belt->belt.items[BeltTrait::FirstSlot] == 0) {
            index = BeltTrait::FirstSlot;
        }
    } else if (dir != Dir::Opposite(belt->belt.direction)) {
        u32 midSlot = BeltTrait::Capacity / 2 + 1;
        if (belt->belt.items[midSlot] == 0) {
            index = midSlot;
            horzPos = 1.0f;
            turnDir = Dir::Opposite(dir);
        }
    }
    if (index != U32::Max) {
        belt->belt.items[index] = itemID;
        belt->belt.itemPositions[index] = callerItemPos - 1.0f + (index * (1.0f / BeltTrait::Capacity));
        belt->belt.itemHorzPositions[index] = horzPos;
        belt->belt.itemTurnDirections[index] = turnDir;
        result = true;
    }
    return result;
}

u32 BeltGrabItem(Entity* entity, Direction dir) {
    u32 result = 0;
    auto belt = (Belt*)entity;
    if (belt->belt.direction == dir) {
        if (belt->belt.items[BeltTrait::LastSlot] && belt->belt.itemPositions[BeltTrait::LastSlot] == 1.0f) {
            result = belt->belt.items[BeltTrait::LastSlot];
            belt->belt.items[BeltTrait::LastSlot] = 0;
        }
    }
    return result;
}
