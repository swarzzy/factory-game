#pragma once
#include "Common.h"

#pragma pack(push, 1)

struct FluxVector3 {
    f32 x, y, z;
};

struct FluxFileHeader {
    static const u32 MagicValue = 0xffaabbcc;
    u32 magicValue = MagicValue;
    enum : u32 { Mesh } type;
};

struct FluxMeshEntry {
    // TODO: next
    u32 next;
    char name[128];
    FluxVector3 aabbMin;
    FluxVector3 aabbMax;
    u32 vertexCount;
    u32 indexCount;
    // Offsets
    u32 vertices;
    u32 normals;
    u32 uv;
    u32 tangents;
    u32 bitangents;
    u32 colors;
    u32 indices;
};

struct FluxMeshHeader {
    FluxFileHeader header;
    u32 version = 1;
    u32 entryCount;
    u32 entries;
    u32 data;
    u32 dataSize;
    char name[128];
};
#pragma pack(pop)
