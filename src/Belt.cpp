#include "Belt.h"

Entity* CreateBelt(GameWorld* world, WorldPos p) {
    Belt* belt = AddBlockEntity<Belt>(world, p.block);
    if (belt) {
        belt->type = EntityType::Belt;
        MakeEntityNeighborhoodDirty(world, belt);
        belt->dirtyNeighborhood = true;
        belt->direction = Direction::North;
    }
    return belt;
}

void BeltDelete(Entity* entity, GameWorld* world) {
    MakeEntityNeighborhoodDirty(world, (BlockEntity*)entity);
}

void BeltDropPickup(Entity* entity, GameWorld* world, WorldPos p) {
    auto belt = (Belt*)entity;
    auto pickup = (Pickup*)CreatePickupEntity(world, p);
    pickup->item = Item::Belt;
    pickup->count = 1;
    for (usize i = 0; i < array_count(belt->items); i++) {
        if (belt->items[i] != Item::None) {
            auto pickup = (Pickup*)CreatePickupEntity(world, p);
            pickup->item = belt->items[i];
            pickup->count = 1;
        }
    }
}

void BeltUpdateAndRender(Belt* belt, void* _data) {
    auto data = (EntityUpdateAndRenderData*)_data;

    bool passedToNeighbor = false;
    bool lateClearLastItemSlot = false;
    for (usize i = 0; i < Belt::Capacity; i++) {
        if (belt->items[i] != Item::None) {
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
                if (i < Belt::Capacity - 1) {
                    if (belt->items[i + 1] == Item::None) {
                        belt->itemPositions[i + 1] = belt->itemPositions[i];
                        belt->items[i + 1] = belt->items[i];
                        belt->items[i] = Item::None;
                        i++;
                    } else {
                        belt->itemPositions[i] = max;
                    }
                } else {
                    auto to = belt->p + DirectionToIV3(belt->direction);
                    auto toEntity = GetEntity(belt->world, to);
                    if (toEntity && toEntity->type == EntityType::Belt) {
                        auto neighborBelt = (Belt*)toEntity;
                        bool neighborHasFreeSlot = false;
                        u32 slot = 0;
                        f32 horzPos = 0.0f;
                        Direction turnDir = belt->direction;
                        if (neighborBelt->direction == belt->direction) {
                            if (neighborBelt->items[Belt::FirstSlot] == Item::None) {
                                neighborHasFreeSlot = true;
                            }
                        } else if (neighborBelt->direction != OppositeDirection(belt->direction)) {
                            u32 midSlot = Belt::Capacity / 2 + 1;
                            if (neighborBelt->items[midSlot] == Item::None) {
                                neighborHasFreeSlot = true;
                                slot = midSlot;
                                horzPos = 1.0f;
                                turnDir = OppositeDirection(belt->direction);
                            }
                        }
                        if (neighborHasFreeSlot) {
                            neighborBelt->items[slot] = belt->items[i];
                            neighborBelt->itemPositions[slot] = belt->itemPositions[i] - max + (slot * (1.0f / Belt::Capacity));
                            neighborBelt->itemHorzPositions[slot] = horzPos;
                            neighborBelt->itemTurnDirections[slot] = turnDir;
                            if (belt->generation > neighborBelt->generation) {
                                belt->items[Belt::LastSlot] = Item::None;
                            } else {
                                lateClearLastItemSlot = true;
                            }
                            passedToNeighbor = true;
                        }
                    }
                }
                if (!passedToNeighbor) {
                    belt->itemPositions[i] = max;
                }
            }
        }
    }

    auto context = GetContext();
    RenderCommandDrawMesh command {};
    command.transform = Translate(WorldPos::Relative(data->camera->targetWorldPosition, WorldPos::Make(belt->p))) * M4x4(RotateY(AngleY(Direction::North, belt->direction)));
    command.mesh = context->beltStraightMesh;
    command.material = &context->beltMaterial;
    Push(data->group, &command);

    for (usize i = 0; i < Belt::Capacity; i++) {
        if (belt->items[i] != Item::None) {
            v3 dir = V3(DirectionToIV3(belt->direction));
            v3 horzDir = V3(DirectionToIV3(belt->itemTurnDirections[i]));
            v3 itemBegin = V3(DirectionToIV3(OppositeDirection(belt->direction))) * 0.5f;
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
        belt->items[Belt::Capacity - 1] = Item::None;
    }

}

void BeltNeighborhoodUpdate(Belt* belt) {
}

void BeltRotate(Belt* belt, void* _data) {
    auto data = (EntityRotateData*)_data;
    belt->direction = RotateYCW(belt->direction);
    MakeEntityNeighborhoodDirty(belt->world, belt);
    BeltNeighborhoodUpdate(belt);
}

void BeltBehavior(Entity* entity, EntityBehaviorInvoke reason, void* data) {
    auto belt = (Belt*)entity;
    switch (reason) {
    case EntityBehaviorInvoke::UpdateAndRender: { BeltUpdateAndRender(belt, data); } break;
    case EntityBehaviorInvoke::Rotate: { BeltRotate(belt, data); } break;
    default: {} break;
    }
}
