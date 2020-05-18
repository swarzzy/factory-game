#pragma once
#include "Platform.h"
#include <cstring>

template <typename T>
inline T Kilobytes(T kb) { return kb * static_cast<T>(1024); }
template <typename T>
inline T Megabytes(T mb) { return mb * static_cast<T>(1024) * static_cast<T>(1024); }

#define ClearMemory(x) memset((x), 0, sizeof((*(x))))

constexpr u64 DefaultAligment = 16;

struct alignas(DefaultAligment) MemoryArena
{
    void* begin;
    uptr offset;
    uptr size;
    uptr free;
    b32 isTemporary;
    i32 tempCount;
};

enum MemoryArenaFlags : u32
{
    MemoryArenaFlag_None = 0,
    MemoryArenaFlag_ClearTemp = (1 << 0)
};

struct TempMemory
{
    MemoryArena* arena;
    uptr offset;
};

MemoryArena* AllocateArena(uptr size, bool isTemp);

inline TempMemory
BeginTemporaryMemory(MemoryArena* arena, MemoryArenaFlags flags = MemoryArenaFlag_None)
{
    assert(arena->isTemporary);

    arena->tempCount++;
    return TempMemory { arena, arena->offset };
}

inline void EndTemporaryMemory(TempMemory* frame)
{
    if (frame->arena)
    {
        auto arena = frame->arena;
        if (frame->offset < arena->offset)
        {
            uptr markOffset = frame->offset;
            arena->free += arena->offset - frame->offset;
            arena->offset = frame->offset;
        }
        arena->tempCount--;
        assert(arena->tempCount >= 0);
        *frame = {};
    }
}

struct ScopedTempMemory
{
    TempMemory frame;
    static inline ScopedTempMemory Make(MemoryArena* arena, MemoryArenaFlags flags = MemoryArenaFlag_None) { return ScopedTempMemory { BeginTemporaryMemory(arena, flags)}; }
    ~ScopedTempMemory() { EndTemporaryMemory(&this->frame); }
};

inline bool CheckTempArena(const MemoryArena* arena)
{
    assert(arena->isTemporary);
    bool result = (arena->tempCount == 0) && (arena->offset == 0);
    return result;
}

inline uptr CalculatePadding(uptr offset, uptr alignment)
{
    uptr padding = 0;
    auto alignmentMask = alignment - 1;
    if (offset & alignmentMask)
    {
        padding = alignment - (offset & alignmentMask);
    }
    return padding;
}

inline void* PushSizeInternal(MemoryArena* arena, uptr size, uptr aligment)
{
    uptr padding = 0;
    uptr useAligment = 0;
    uptr currentAddress = (uptr)arena->begin + arena->offset;

    if (aligment == 0)
    {
        useAligment = DefaultAligment;
    }
    else
    {
        useAligment = aligment;
    }

    if (arena->offset % useAligment != 0)
    {
        padding = CalculatePadding(currentAddress, useAligment);
    }

    // TODO: Grow!
    assert(size + padding <= arena->free);
    uptr nextAdress = (uptr)((byte*)arena->begin + arena->offset + padding);
    assert(nextAdress % useAligment == 0);
    arena->offset += size + padding;
    arena->free -= size + padding;

    return (void*)nextAdress;
}

inline void* PushSize(MemoryArena* arena, uptr size, u32 flags = MemoryArenaFlag_ClearTemp, uptr aligment = 0)
{
    void* mem = PushSizeInternal(arena, size, aligment);
    if (arena->isTemporary && (flags & MemoryArenaFlag_ClearTemp))
    {
        memset(mem, 0, size);
    }
    return mem;
}

inline MemoryArena* AllocateSubArena(MemoryArena* arena, uptr size, bool isTemporary)
{
    MemoryArena* result = nullptr;
    uptr chunkSize = size + sizeof(arena);
    void* chunk = PushSize(arena, chunkSize, DefaultAligment);
    if (chunk)
    {
        MemoryArena header = {};
        header.free = size;
        header.size = size;
        header.isTemporary = isTemporary;
        header.begin = (void*)((byte*)chunk + sizeof(MemoryArena));
        memcpy(chunk, &header, sizeof(MemoryArena));
        result = (MemoryArena*)chunk;
    }
    return result;
}

MemoryArena* AllocateArena(uptr size, bool isTemp);
