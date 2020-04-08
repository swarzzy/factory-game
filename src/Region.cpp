#include "Region.h"

#include "flux_render_group.h"
#include "flux_renderer.h"
#include "flux_camera.h"

Region BeginRegion(GameWorld* world, iv3 origin, i32 dim) {
    Region region = {};
    region.world = world;
    region.origin = origin;
    region.dim = dim;

    i32 regionSpan = region.dim - 1;
    auto originChunk = ChunkPosFromWorldPos(region.origin);
    iv3 regionBegin = originChunk.chunk - IV3(regionSpan);
    iv3 regionEnd = originChunk.chunk + IV3(regionSpan);

    for (i32 z = regionBegin.z; z <= regionEnd.z; z++) {
        for (i32 y = regionBegin.y; y <= regionEnd.y; y++) {
            for (i32 x = regionBegin.x; x <= regionEnd.x; x++) {
                auto chunk = GetChunk(world, x, y, z);
                // TODO: Empty chunks
                if (!chunk) {
                    chunk = AddChunk(world, IV3(x, y, z));
                    assert(chunk);
                    DebugFillChunk(chunk);
                }

                if (!chunk->mesh) {
                    ClaimFurthestChunkMesh(world, chunk);
                    GenMesh(world->mesher, chunk);
                    if (chunk->mesh->vertexCount) {
                        UploadToGPU(chunk->mesh);
                    }
                }
            }
        }
    }

    return region;
}


void DrawRegion(Region* region, RenderGroup* renderGroup, Camera* camera) {
    i32 regionSpan = region->dim - 1;
    auto originChunk = ChunkPosFromWorldPos(region->origin);
    iv3 regionBegin = originChunk.chunk - IV3(regionSpan);
    iv3 regionEnd = originChunk.chunk + IV3(regionSpan);

    Push(renderGroup, &RenderCommandBeginChunkBatch{});

    if (region->debugRender) {
        for (auto& record : region->world->chunkMeshPool) {
            if (record.chunk) {
                if (record.mesh.vertexCount) {
                    RenderCommandPushChunk chunkCommand = {};
                    chunkCommand.mesh = &record.mesh;
                    chunkCommand.offset = RelativePos(camera->targetWorldPosition, WorldPos::Make(record.chunk->p * (i32)Chunk::Size));
                    Push(renderGroup, &chunkCommand);
                }
            }
        }
    } else {
        for (i32 z = regionBegin.z; z <= regionEnd.z; z++) {
            for (i32 y = regionBegin.y; y <= regionEnd.y; y++) {
                for (i32 x = regionBegin.x; x <= regionEnd.x; x++) {
                    auto chunk = GetChunk(region->world, x, y, z);
                    if (chunk) {
                        if (chunk->mesh->vertexCount) {
                            assert(chunk->mesh);
                            if (chunk->mesh->vertexCount) {
                                RenderCommandPushChunk chunkCommand = {};
                                chunkCommand.mesh = chunk->mesh;
                                chunkCommand.offset = RelativePos(camera->targetWorldPosition, WorldPos::Make(chunk->p * (i32)Chunk::Size));
                                Push(renderGroup, &chunkCommand);
                            }
                        }
                    }
                }
            }
        }
    }
    Push(renderGroup, &RenderCommandEndChunkBatch{});

    if (region->debugShowBoundaries) {
        v3 min = RelativePos(camera->targetWorldPosition, WorldPos::Make((originChunk.chunk - regionSpan) * (i32)Chunk::Size));
        v3 max = RelativePos(camera->targetWorldPosition, WorldPos::Make((originChunk.chunk + regionSpan + 1) * (i32)Chunk::Size));
        min -= V3(Voxel::HalfDim);
        max -= V3(Voxel::HalfDim);
        DrawAlignedBoxOutline(renderGroup, min, max, V3(0.0f, 0.0f, 1.0f), 2.0f);
    }
}
