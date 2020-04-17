#pragma once
#include "Platform.h"

b32 GlobalDrawDebugOverlay = true;

void DrawDebugPerformanceCounters();
void BeginDebugOverlay();
bool DebugOverlayBeginCustom();
void DebugOverlayEndCustom();
void DebugOverlayPushString(const char* string);
void DebugOverlayPushVar(const char* title, uv3 var);
void DebugOverlayPushVar(const char* title, iv3 var);
void DebugOverlayPushVar(const char* title, v3 var);
void DebugOverlayPushVar(const char* title, v4 var);
void DebugOverlayPushVar(const char* title, u32 var);
void DebugOverlayPushVar(const char* title, f32 var);
void DebugOverlayPushSlider(const char* title, f32* var, f32 min, f32 max);
void DebugOverlayPushSlider(const char* title, i32* var, i32 min, i32 max);
void DebugOverlayPushSlider(const char* title, v3* var, f32 min, f32 max);
void DebugOverlayPushSlider(const char* title, v4* var, f32 min, f32 max);
void DebugOverlayPushToggle(const char* title, bool* var);

#define DEBUG_OVERLAY_TRACE(var) DebugOverlayPushVar(#var, var)
#define DEBUG_OVERLAY_SLIDER(var, min, max) DebugOverlayPushSlider(#var, &var, min, max)
#define DEBUG_OVERLAY_TOGGLE(var) DebugOverlayPushToggle(#var, &var)
