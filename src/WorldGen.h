#pragma once

struct Chunk;

struct Noise2D {
    static const u32 BitShift = 8;
    static const u32 BitMask = (1 << BitShift) - 1;
    static const u32 Size = 1 << BitShift;
    u32 seed;
    u32 permutationTable[Size * 2];
    f32 values[Size];
};

Noise2D CreateNoise2D(u32 seed);
f32 Sample(Noise2D* noise, float x, float y);

struct WorldGen {
    Noise2D noise;
    Noise2D anotherNoise;

    void Init(u32 seed) {
        this->noise = CreateNoise2D(seed);
        this->anotherNoise = CreateNoise2D(seed + 93485);
    }
};

void GenChunk(WorldGen* gen, Chunk* chunk);
void RunNoise2DTest();
