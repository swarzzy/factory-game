#include "SimRegion.h"

bool IsInside(iv3 min, iv3 max, iv3 x) {
    bool result = false;
    if (x.x >= min.x && x.x <= max.x &&
        x.y >= min.y && x.y <= max.y &&
        x.z >= min.z && x.z <= max.z) {
        result = true;
    }
    return result;
}

void RemoveChunkFromRegion(SimRegion* region, Chunk* chunk) {
    assert(chunk->active);
    assert(chunk->primaryMesh);
    assert(!chunk->secondaryMesh);
    auto prev = chunk->prevActive;
    auto next = chunk->nextActive;
    chunk->prevActive = nullptr;
    chunk->nextActive = nullptr;
    chunk->active = false;

    region->chunkMeshPoolUsage[chunk->primaryMeshPoolIndex] = false;
    auto mesh = region->chunkMeshPool + chunk->primaryMeshPoolIndex;
    FreeChunkMesh(region->world->mesher, mesh);
    chunk->primaryMesh = nullptr;

    assert(region->chunkCount);
    region->chunkCount--;
    if (!prev) {
        region->firstChunk = next;
        if (next) {
            next->prevActive = nullptr;
        }
    } else {
        prev->nextActive = next;
        if (next) {
            next->prevActive = prev;
        }
    }
}

void EvictFurthestChunkFromRegion(SimRegion* region) {
    Chunk* furthestChunkOutside = nullptr;
    i32 furthestDistOutside = 0;

    Chunk* furthestChunk = nullptr;
    i32 furthestDist = 0;
    Chunk* chunk = region->firstChunk;
    while (chunk) {
        auto state = chunk->state;
        if (state == ChunkState::Complete) {
            i32 dist = LengthSq(region->origin - chunk->p);
            if (!IsInside(region->min, region->max, chunk->p)) {
                if (dist > furthestDistOutside) {
                    furthestDistOutside = dist;
                    furthestChunkOutside = chunk;
                }
            }
            if (dist > furthestDist) {
                furthestDist = dist;
                furthestChunk = chunk;
            }
        }
        chunk = chunk->nextActive;
    }

    if (furthestChunkOutside) {
        RemoveChunkFromRegion(region, furthestChunkOutside);
    } else {
        printf("[Sim region]: Warn! Evicting a chunk which is inside region bounds\n");
        RemoveChunkFromRegion(region, furthestChunk);
    }
}

struct GetChunkMeshFromPoolResult {
    ChunkMesh* mesh;
    u32 index;
};

GetChunkMeshFromPoolResult GetChunkMeshFromPool(SimRegion* region) {
    GetChunkMeshFromPoolResult result{};
    i32 index = -1;
    for (u32x i = 0; i < region->maxChunkCount; i++) {
        if (!region->chunkMeshPoolUsage[i]) {
            region->chunkMeshPoolUsage[i] = 1;
            index = i;
            break;
        }
    }

    if (index != -1) {
        result = { region->chunkMeshPool + index, (u32)index };
    }
    return result;
}

void AddChunkToRegion(SimRegion* region, Chunk* chunk) {
    assert(!chunk->active);
    assert(!chunk->prevActive);
    assert(!chunk->nextActive);
    chunk->active = true;
    chunk->nextActive = region->firstChunk;
    if (chunk->nextActive) {
        chunk->nextActive->prevActive = chunk;
    }
    region->firstChunk = chunk;
    region->chunkCount++;

    auto mesh = GetChunkMeshFromPool(region);
    assert(mesh.mesh);
    assert(!chunk->primaryMesh);
    chunk->primaryMesh = mesh.mesh;
    chunk->primaryMeshPoolIndex = mesh.index;
}

void RegionUpdateChunkStates(SimRegion* region) {
    Chunk* chunk = region->firstChunk;
    while (chunk) {
        if (chunk->state == ChunkState::Filled) {
            chunk->filled = true;
            chunk->state = ChunkState::Complete;
        } else if (chunk->state == ChunkState::MeshingFinished) {
            chunk->meshValid = true;
            chunk->state = ChunkState::Complete;
        } else if (chunk->state == ChunkState::WaitsForUpload)
        {
            if (chunk->primaryMesh->vertexCount) {
                ScheduleChunkMeshUpload(chunk);
            } else {
                chunk->state = ChunkState::Complete;
                chunk->meshValid = true;
            }
        } else if (chunk->state == ChunkState::MeshUploadingFinished) {
            CompleteChunkMeshUpload(chunk);
        } else if (chunk->state == ChunkState::Complete) {
            if (!chunk->filled) {
                ScheduleChunkFill(&region->world->worldGen, chunk);
            } else {
                if (!chunk->meshValid) {
                    ScheduleChunkMeshing(region->world, chunk);
                }
            }
        }


        chunk = chunk->nextActive;
    }
}

// TODO: Make this an actual function
void ResizeRegion(SimRegion* region, u32 newSpan, MemoryArena* arena) {
    region->span = newSpan;
    u32 regionSide = newSpan * 2 + 1;
    u32 regionHeight = GameWorld::MaxHeightChunk - GameWorld::MinHeightChunk + 1;
    region->maxChunkCount = regionSide * regionSide * regionHeight + 16; // TODO: Formalize the number of extra chunks
    region->chunkMeshPool = (ChunkMesh*)PushSize(arena, sizeof(ChunkMesh) * region->maxChunkCount);
    region->chunkMeshPoolUsage = (byte*)PushSize(arena, sizeof(byte) * region->maxChunkCount);
    for (u32x i = 0; i < region->maxChunkCount; i++) {
        region->chunkMeshPool[i].mesher = region->world->mesher;
    }
}

void MoveRegion(SimRegion* region, iv3 newP) {
    iv3 newMin = newP - IV3(region->span);
    newMin.y = GameWorld::MinHeightChunk;
    iv3 newMax = newP + IV3(region->span);
    newMax.y = GameWorld::MaxHeightChunk;

    region->origin = newP;
    region->min = newMin;
    region->max = newMax;

    for (i32 z = newMin.z; z <= newMax.z; z++) {
        for (i32 y = newMin.y; y <= newMax.y; y++) {
            for (i32 x = newMin.x; x <= newMax.x; x++) {
                Chunk* chunk = GetChunk(region->world, x, y, z);
                if (!chunk) {
                    chunk = AddChunk(region->world, IV3(x, y, z));
                    assert(chunk);
                }
                // TODO: Checking that chunk is not currently in this region. For now we assume that there is only one
                // region in thr world, so just checking for chunk to be not active. In the future we probably
                // want to have region id's and check is chunk already in this region by this id
                if (!chunk->active) {
                    assert(region->chunkCount <= region->maxChunkCount);
                    if (region->chunkCount == region->maxChunkCount) {
                        EvictFurthestChunkFromRegion(region);
                    }
                    assert(region->chunkCount < region->maxChunkCount);
                    AddChunkToRegion(region, chunk);
                }
            }
        }
    }
}

void DrawRegion(SimRegion* region, RenderGroup* renderGroup, Camera* camera) {
    auto chunk = region->firstChunk;
    Push(renderGroup, &RenderCommandBeginChunkBatch{});
    while (chunk) {
        auto state = chunk->state;
        if (state == ChunkState::Complete) {
            if (chunk->primaryMesh->vertexCount) {
                assert(chunk->primaryMesh);
                RenderCommandPushChunk chunkCommand = {};
                chunkCommand.mesh = chunk->primaryMesh;
                chunkCommand.offset = RelativePos(camera->targetWorldPosition, WorldPos::Make(chunk->p * (i32)Chunk::Size));
                Push(renderGroup, &chunkCommand);
            }
        }
        chunk = chunk->nextActive;
    }
    Push(renderGroup, &RenderCommandEndChunkBatch{});
}
