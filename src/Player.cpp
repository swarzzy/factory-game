#include "Player.h"

Entity* CreatePlayerEntity(GameWorld* world, WorldPos p) {
    Player* entity = AddSpatialEntity<Player>(world, p);
    if (entity) {
        entity->type = EntityType::Player;
        entity->flags |= EntityFlag_ProcessOverlaps;
        entity->scale = 0.95f;
        entity->acceleration = 70.0f;
        entity->friction = 10.0f;
        entity->height = 1.8f;
        entity->selectedVoxel = GameWorld::InvalidPos;
        entity->jumpAcceleration = 420.0f;
        entity->runAcceleration = 140.0f;
        entity->inventory = AllocateEntityInventory(16, 128);
    }
    return entity;
}

void PlayerUpdateAndRender(Entity* _entity, EntityUpdateInvoke reason, f32 deltaTime, RenderGroup* group, Camera* camera) {
    if (reason == EntityUpdateInvoke::UpdateAndRender) {
        auto entity = (Player*)_entity;
        auto oldP = entity->p;
        v3 frameAcceleration = {};

        f32 playerAcceleration;
        v3 drag = entity->velocity * entity->friction;

        auto z = Normalize(V3(camera->front.x, 0.0f, camera->front.z));
        auto x = Normalize(Cross(V3(0.0f, 1.0f, 0.0f), z));
        auto y = V3(0.0f, 1.0f, 0.0f);

        if (entity->camera->inputMode == GameInputMode::Game || entity->camera->inputMode == GameInputMode::InGameUI) {

            if (KeyHeld(Key::W)) {
                frameAcceleration -= z;
            }
            if (KeyHeld(Key::S)) {
                frameAcceleration += z;
            }
            if (KeyHeld(Key::A)) {
                frameAcceleration -= x;
            }
            if (KeyHeld(Key::D)) {
                frameAcceleration += x;
            }

            if ((KeyHeld(Key::Space))) {
                //frameAcceleration += y;
            }

            if (KeyPressed(Key::Y)) {
                entity->flightMode = !entity->flightMode;
            }

            if (entity->flightMode) {
                if (KeyHeld(Key::Space)) {
                    frameAcceleration += y;
                }
                if (KeyHeld(Key::Ctrl)) {
                    frameAcceleration -= y;
                }
            } else {
                drag.y = 0.0f;
            }

            if ((KeyHeld(Key::Shift))) {
                playerAcceleration = entity->runAcceleration;
            } else {
                playerAcceleration = entity->acceleration;
            }
            frameAcceleration *= playerAcceleration;
        }

        // TODO: Physically correct friction
        frameAcceleration -= drag;

        if (!entity->flightMode) {
            if (camera->inputMode == GameInputMode::Game || camera->inputMode == GameInputMode::InGameUI) {
                if (KeyPressed(Key::Space) && entity->grounded) {
                    frameAcceleration += y * entity->jumpAcceleration * (1.0f / deltaTime) / 60.0f;
                }
            }

            frameAcceleration.y += -20.8f;
        }


        v3 movementDelta = 0.5f * frameAcceleration * deltaTime * deltaTime + entity->velocity * deltaTime;

        entity->velocity += frameAcceleration * deltaTime;
        DEBUG_OVERLAY_TRACE(entity->velocity);
        bool hitGround = false;
        MoveSpatialEntity(entity->world, entity, movementDelta, camera, nullptr);

        if (WorldPos::ToChunk(entity->p).chunk != WorldPos::ToChunk(oldP).chunk) {
            MoveRegion(entity->region, WorldPos::ToChunk(entity->p).chunk);
        }

        auto context = GetContext();
        if (entity->camera->mode != CameraMode::Gameplay) {
            RenderCommandDrawMesh command{};
            command.transform = Translate(WorldPos::Relative(camera->targetWorldPosition, entity->p));
            command.mesh = context->playerMesh;
            command.material = &context->playerMaterial;
            Push(group, &command);
        }
    }
}

void PlayerProcessOverlap(GameWorld* world, SpatialEntity* testEntity, SpatialEntity* overlappedEntity) {
    if (overlappedEntity->type == EntityType::Pickup) {
        auto pickup = static_cast<Pickup*>(overlappedEntity);
        assert(testEntity->inventory);
        auto itemRemainder = EntityInventoryPushItem(testEntity->inventory, pickup->item, pickup->count);
        if (itemRemainder == 0) {
            DeleteBlockEntityAfterThisFrame(world, overlappedEntity);
        } else {
            pickup->count = itemRemainder;
        }
    }
}
