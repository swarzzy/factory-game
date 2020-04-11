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

    auto begin = PlatformGetTimeStamp();
    bool generated = false;
    for (i32 z = regionBegin.z; z <= regionEnd.z; z++) {
        for (i32 y = regionBegin.y; y <= regionEnd.y; y++) {
            for (i32 x = regionBegin.x; x <= regionEnd.x; x++) {
                auto chunk = GetChunk(world, x, y, z);
                // TODO: Empty chunks
                if (!chunk) {
                    generated = true;
                    chunk = AddChunk(world, IV3(x, y, z));
                    assert(chunk);
                    GenChunk(&world->worldGen, chunk);
                }
                if (!chunk->mesh) {
                    ScheduleChunkMeshing(world, world->mesher, chunk);
                }
                if (!GlobalPlatform.supportsAsyncGPUTransfer) {
                    if (chunk->mesh) {
                        auto state = chunk->mesh->state;
                        if (state == ChunkMesh::State::ReadyForUpload) {
                            if (chunk->mesh->vertexCount) {
                                ScheduleChunkMeshUpload(chunk->mesh);
                            } else {
                                chunk->mesh->state = ChunkMesh::State::Complete;
                            }
                        } else if (state == ChunkMesh::State::UploadComplete) {
                            //auto begin = PlatformGetTimeStamp();
                            CompleteChunkMeshUpload(chunk->mesh);
                            //auto end = PlatformGetTimeStamp();
                            //printf("[Region] Loaded mesh on gpu: %f ms\n", (end - begin) * 1000.0f);
                        }

                    }
                }
            }
        }
    }

    auto end = PlatformGetTimeStamp();
    if (generated) {
    printf("Generating new chunks: %f ms\n", (end - begin) * 1000.0f);
    }

    for (i32 z = regionBegin.z; z <= regionEnd.z; z++) {
        for (i32 y = regionBegin.y; y <= regionEnd.y; y++) {
            for (i32 x = regionBegin.x; x <= regionEnd.x; x++) {
                auto chunk = GetChunk(world, x, y, z);
                if (!chunk->mesh) {
                    ScheduleChunkMeshing(world, world->mesher, chunk);
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
        for (auto& record : region->world->mesher->chunkMeshPool) {
            if (record.chunk) {
                auto meshState = record.chunk->mesh->state;
                if (meshState == ChunkMesh::State::Complete) {
                    if (record.mesh.vertexCount) {
                        RenderCommandPushChunk chunkCommand = {};
                        chunkCommand.mesh = &record.mesh;
                        chunkCommand.offset = RelativePos(camera->targetWorldPosition, WorldPos::Make(record.chunk->p * (i32)Chunk::Size));
                        Push(renderGroup, &chunkCommand);
                    }
                }
            }
        }
    } else {
        for (i32 z = regionBegin.z; z <= regionEnd.z; z++) {
            for (i32 y = regionBegin.y; y <= regionEnd.y; y++) {
                for (i32 x = regionBegin.x; x <= regionEnd.x; x++) {
                    auto chunk = GetChunk(region->world, x, y, z);
                    if (chunk) {
                        auto meshState = chunk->mesh->state;
                        if (meshState == ChunkMesh::State::Complete) {
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
