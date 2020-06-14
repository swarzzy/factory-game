#pragma once

constexpr u32 MaxAssetPathSize = 256;
constexpr u32 MaxAssetNameSize = 128;


namespace Globals {
    constant f32 MeterScale = 1.0f;
    constant f32 BlockDim = 1.0f * Globals::MeterScale;
    constant f32 BlockHalfDim = BlockDim * 0.5f;
    constant f32 PickupScale = 0.23f;
    constant f32 DefaultBeltItemMeshAlign = 0.32f;

    static bool DrawCollisionVolumes = true;
    static bool CreativeModeEnabled = true;
    static  bool ShowDebugOverlay = false;
}
