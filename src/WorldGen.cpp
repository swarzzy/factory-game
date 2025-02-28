#include "WorldGen.h"

Noise2D CreateNoise2D(u32 seed) {
    Noise2D noise;
    noise.seed = seed;
    srand(seed);

    for (u32x i = 0; i < Noise2D::Size; i++) {
        noise.values[i] = rand() / (f32)RAND_MAX;
        noise.permutationTable[i] = i;
    }

    for (u32x i = 0; i < Noise2D::Size; i++) {
        u32 sh = rand() & Noise2D::BitMask;
        u32 tmp = noise.permutationTable[i];
        noise.permutationTable[i] = noise.permutationTable[sh];
        noise.permutationTable[sh] = tmp;
        noise.permutationTable[i + Noise2D::Size] = noise.permutationTable[i];
    }

    return noise;
}

f32 Sample(Noise2D* noise, float x, float y) {
    x = Abs(x);
    y = Abs(y);

    u32 xi = (u32)Floor(x);
    u32 yi = (u32)Floor(y);

    f32 tx = x - xi;
    f32 ty = y - yi;

    u32 x0 = xi & Noise2D::BitMask;
    u32 x1 = (xi + 1) & Noise2D::BitMask;
    u32 y0 = yi & Noise2D::BitMask;
    u32 y1 = (yi + 1) & Noise2D::BitMask;

    float c00 = noise->values[noise->permutationTable[noise->permutationTable[x0] + y0]];
    float c10 = noise->values[noise->permutationTable[noise->permutationTable[x1] + y0]];
    float c01 = noise->values[noise->permutationTable[noise->permutationTable[x0] + y1]];
    float c11 = noise->values[noise->permutationTable[noise->permutationTable[x1] + y1]];

    tx = SmoothStep(0.0f, 1.0f, tx);
    ty = SmoothStep(0.0f, 1.0f, ty);

    f32 vMin = Lerp(c00, c10, tx);
    f32 vMax = Lerp(c01, c11, tx);
    f32 result = Lerp(vMin, vMax, ty);
    return result;
}

u32 GetHeightFromNoise(Noise2D* noise, i32 x, i32 z, u32 maxHeight) {
    f32 noiseX = (f32)x * 0.1f;
    f32 noiseY = (f32)z * 0.1f;
    f32 freq = 0.2f;
    f32 ampl = 1.0f;
    f32 value = 0.0f;
    for (u32 i = 0; i < 4; i++) {
        value += Sample(noise, noiseX * freq, noiseY * freq) * ampl;
        freq = freq * 2.0f;
        ampl = ampl / 2.0f;
    }

    u32 blockHeight = (u32)((float)maxHeight * value) + 1;
    return blockHeight;
}

void ChunkFillWork(void* data0, void* data1, void* data2, u32 threadID) {
    auto worldGen = (WorldGen*)data0;
    auto chunk = (Chunk*)data1;
    GenChunk(worldGen, chunk);
    auto prevState = AtomicExchange((volatile u32*)&chunk->state, (u32)ChunkState::Filled);
    assert(prevState == (u32)ChunkState::Filling);
}

bool ScheduleChunkFill(WorldGen* gen, Chunk* chunk) {
    bool result = false;
    chunk->state = ChunkState::Filling;
    WriteFence();
    if (PlatformPushWork(GetPlatform()->lowPriorityQueue, ChunkFillWork, gen, chunk, nullptr)) {
        result = true;
    }
    else {
        chunk->state = ChunkState::Complete;
    }
    return result;
}

void GenChunk(WorldGen* gen, Chunk* chunk) {
    if (chunk->p.y == 0) {
        for (u32 bz = 0; bz < Chunk::Size; bz++) {
            for (u32 bx = 0; bx < Chunk::Size; bx++) {
                auto wp = ChunkPos::ToWorld(ChunkPos{chunk->p, UV3(bx, 0, bz)});
                u32 grassHeight = GetHeightFromNoise(&gen->noise, wp.block.x, wp.block.z, 10);
                u32 rockHeight = GetHeightFromNoise(&gen->anotherNoise, wp.block.x, wp.block.z, 8);
                auto height = grassHeight;
                auto value = grassHeight > rockHeight ? BlockValue::Grass : BlockValue::Stone;
                if (rockHeight <= 7 && grassHeight <= 7) {
                    value = BlockValue::Water;
                    height = 6;
                }
                for (u32 by = 0; by < height; by++) {
                    i32 coal = rand() % 1000;
                    if (value != BlockValue::Water) {
                        if (coal == 1) {
                            value = BlockValue::CoalOre;
                        }
                    }
                    auto block = GetBlockValueRaw(chunk, bx, by, bz);
                    *block = value;
                }
            }
        }
    } else if (chunk->p.y < 0) {
        for (u32 bz = 0; bz < Chunk::Size; bz++) {
            for (u32 by = 0; by < Chunk::Size; by++) {
                for (u32 bx = 0; bx < Chunk::Size; bx++) {
                    auto block = GetBlockValueRaw(chunk, bx, by, bz);
                    *block = BlockValue::Stone;
                }
            }
        }
    }
}

void RunNoise2DTest() {
    auto noise = CreateNoise2D(234234);
    auto file = fopen("noise.ppm", "wb");
    assert(file);
    fprintf(file, "P3\n%lu %lu \n255\n", 512, 512);
    for (u32 y = 0; y < 512; y++) {
        for (u32 x = 0; x < 512; x++) {
            f32 noiseX = ((f32)x - 256.0f) * 0.2f;
            f32 noiseY = ((f32)y - 256.0f) * 0.2f;
            f32 freq = 0.1f;
            f32 ampl = 1.0f;
            f32 value = 0.0f;
            for (u32 i = 0; i < 5; i++) {
                value += Sample(&noise, noiseX * freq, noiseY * freq) * ampl;
                freq = freq * 2.0f;
                ampl = ampl / 2.0f;
            }
            u32 v = Clamp((u32)(value * 128.0f), (u32)0, (u32)255);
            fprintf(file, "%lu, %lu, %lu ", v, v, v);
        }
    }
    fclose(file);
}
