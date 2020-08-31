#pragma once

constexpr u32 MaxAssetPathSize = 256;
constexpr u32 MaxAssetNameSize = 128;


namespace Globals {
    constant f32 MeterScale = 1.0f;
    constant f32 BlockDim = 1.0f * Globals::MeterScale;
    constant f32 BlockHalfDim = BlockDim * 0.5f;
    constant f32 PickupScale = 0.23f;
    constant f32 DefaultBeltItemMeshAlign = 0.32f;

    constant u32 ChunkBitShift = 5;
    constant u32 ChunkBitMask = (1 << ChunkBitShift) - 1;
    constant u32 ChunkSize = 1 << ChunkBitShift;

    constant char* DebugWorldName = "test_world";

    static bool DrawCollisionVolumes = true;
    static bool CreativeModeEnabled = true;
    static  bool ShowDebugOverlay = false;

    // Renderer

    constant u64 UniformBufferMaxTimeout = 1000000000;
}
