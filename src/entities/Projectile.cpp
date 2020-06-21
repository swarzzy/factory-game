#include "Projectile.h"
#include "RenderGroup.h"

Projectile* CreateProjectile(WorldPos p) {
    auto world = GetWorld();
    auto projectile = (Projectile*)CreateProjectileEntity(world, p);
    return projectile;
}

Entity* CreateProjectileEntity(GameWorld* world, WorldPos p) {
    auto entity = AddSpatialEntity<Projectile>(world, p);
    if (entity) {
        entity->type = EntityType::Projectile;
        entity->scale = Globals::PickupScale;
    }
    return entity;
}

void ProjectileCollisionResponse(SpatialEntity* entity, const CollisionInfo* info) {
    auto projectile = (Projectile*)entity;
    auto p = projectile->p.block;
    auto radius = 4;
    auto radiusSq = radius * radius;
    for (i32 z = p.z - radius; z < (p.z + radius); z++) {
        for (i32 y = p.y - radius; y < (p.y + radius); y++) {
            for (i32 x = p.x - radius; x < (p.x + radius); x++) {
                auto pi = IV3(x, y, z);
                if (LengthSq(pi - p) <= radiusSq) {
                    auto chunkP = WorldPos::ToChunk(pi);
                    auto chunk = GetChunk(projectile->world, chunkP.chunk);
                    if (chunk) {
                        auto blockValue = GetBlockForModification(chunk, chunkP.block.x, chunkP.block.y, chunkP.block.z);
                        *blockValue = BlockValue::Empty;
                    }
                }
            }
        }
    }

    ScheduleEntityForDelete(entity->world, entity);
}

void ProjectileUpdateAndRender(Entity* _entity, EntityBehaviorInvoke reason, void* _data) {
    SpatialEntityBehavior(_entity, reason, _data);
    if (reason == EntityBehaviorInvoke::UpdateAndRender) {
        auto data = (EntityUpdateAndRenderData*)_data;
        auto entity = (Projectile*)_entity;

        auto context = GetContext();

        RenderCommandDrawMesh command{};
        command.transform = Translate(WorldPos::Relative(data->camera->targetWorldPosition, entity->p));
        command.mesh = context->grenadeMesh;
        command.material = &context->grenadeMaterial;
        command.transform = command.transform * Scale(V3(entity->scale));
        Push(data->group, &command);

        if (Globals::DrawCollisionVolumes) {
            f32 radius = entity->scale * 0.5f;
            v3 min = WorldPos::Relative(data->camera->targetWorldPosition, entity->p) - radius;
            v3 max = WorldPos::Relative(data->camera->targetWorldPosition, entity->p) + radius;
            DrawAlignedBoxOutline(data->group, min, max, V3(1.0f, 1.0f, 0.0f), 0.3f);
        }
    }
}
