#include "DebugOverlay.h"

void DrawDebugPerformanceCounters() {
    const float DISTANCE = 10.0f;
    int corner = 1;
    ImGuiIO& io = ImGui::GetIO();
    ImVec2 window_pos = ImVec2((corner & 1) ? io.DisplaySize.x - DISTANCE : DISTANCE, (corner & 2) ? io.DisplaySize.y - DISTANCE : DISTANCE);
    ImVec2 window_pos_pivot = ImVec2((corner & 1) ? 1.0f : 0.0f, (corner & 2) ? 1.0f : 0.0f);
    ImGui::SetNextWindowPos(window_pos, ImGuiCond_Always, window_pos_pivot);
    ImGui::SetNextWindowBgAlpha(0.35f);
    bool open = true;
    if (ImGui::Begin("Overlay", &open, (corner != -1 ? ImGuiWindowFlags_NoMove : 0) | ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoNav)) {
        char fpsBuffer[128];
        auto platform = GetPlatform();
        sprintf_s(fpsBuffer, 128, "FPS: %11d\nUPS: %11d\ndT(abs):  %.4f\ndT(game): %.4f", platform->fps, platform->ups, platform->absDeltaTime, platform->gameDeltaTime);
        ImGui::Text("%s", fpsBuffer);
    }
    ImGui::End();
}

static const auto DebugOverlayFlags = 0;//ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoNav;
//ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoDecoration

void BeginDebugOverlay() {
    const float xPos = 10.0f;
    const float yPos = 10.0f;

    ImGuiIO& io = ImGui::GetIO();
    ImVec2 windowPos = ImVec2(xPos, yPos);
    ImVec2 windosPosPivot = ImVec2(0.0f, 0.0f);
    ImGui::SetNextWindowPos(windowPos, ImGuiCond_Always, windosPosPivot);
    ImGui::SetNextWindowBgAlpha(0.5f);
    ImGui::Begin("Debug overlay", nullptr, DebugOverlayFlags);
    ImGui::End();
}

bool DebugOverlayBeginCustom() {
    bool result = false;
    if (GlobalDrawDebugOverlay) {
        result = ImGui::Begin("Debug overlay", nullptr, DebugOverlayFlags);
    }
    return result;
}

void DebugOverlayEndCustom() {
    if (GlobalDrawDebugOverlay) {
        ImGui::End();
    }
}

void DebugOverlayPushInternal(const char* string) {
    if (GlobalDrawDebugOverlay) {
        if (ImGui::Begin("Debug overlay", nullptr, DebugOverlayFlags)) {
            ImGui::Separator();
            ImGui::Text("%s", string);
        }
        ImGui::End();
    }
}

void DebugOverlayPushString(const char* string) {
    DebugOverlayPushInternal(string);
}

void DebugOverlayPushVar(const char* title, uv3 var) {
    char buffer[128];
    sprintf_s(buffer, 128, "%s: x: %lu; y: %lu; z: %lu", title, (unsigned long)var.x, (unsigned long)var.y, (unsigned long)var.z);
    DebugOverlayPushInternal(buffer);
}

void DebugOverlayPushVar(const char* title, iv3 var) {
    char buffer[128];
    sprintf_s(buffer, 128, "%s: x: %ld; y: %ld; z: %ld", title, (long)var.x, (long)var.y, (long)var.z);
    DebugOverlayPushInternal(buffer);
}


void DebugOverlayPushVar(const char* title, v3 var) {
    char buffer[128];
    sprintf_s(buffer, 128, "%s: x: %.3f; y: %.3f; z: %.3f", title, var.x, var.y, var.z);
    DebugOverlayPushInternal(buffer);
}

void DebugOverlayPushVar(const char* title, v4 var) {
    char buffer[128];
    sprintf_s(buffer, 128, "%s: x: %.3f; y: %.3f; z: %.3f; w: %.3f", title, var.x, var.y, var.z, var.w);
    DebugOverlayPushInternal(buffer);
}


void DebugOverlayPushVar(const char* title, u32 var) {
    char buffer[128];
    sprintf_s(buffer, 128, "%s: x: %lu", title, (unsigned long)var);
    DebugOverlayPushInternal(buffer);
}

void DebugOverlayPushVar(const char* title, f32 var) {
    char buffer[128];
    sprintf_s(buffer, 128, "%s: x: %.5f", title, var);
    DebugOverlayPushInternal(buffer);
}

void DebugOverlayPushSlider(const char* title, f32* var, f32 min, f32 max) {
    if (GlobalDrawDebugOverlay) {
        if (ImGui::Begin("Debug overlay", nullptr, DebugOverlayFlags)) {
            ImGui::Separator();
            ImGui::SliderFloat(title, var, min, max);
        }
        ImGui::End();
    }
}

void DebugOverlayPushSlider(const char* title, i32* var, i32 min, i32 max) {
    if (GlobalDrawDebugOverlay) {
        if (ImGui::Begin("Debug overlay", nullptr, DebugOverlayFlags)) {
            ImGui::Separator();
            ImGui::SliderInt(title, var, min, max);
        }
        ImGui::End();
    }
}

void DebugOverlayPushSlider(const char* title, v3* var, f32 min, f32 max) {
    if (GlobalDrawDebugOverlay) {

        if (ImGui::Begin("Debug overlay", nullptr, DebugOverlayFlags)) {
            ImGui::Separator();
            ImGui::SliderFloat3(title, var->data, min, max);
        }
        ImGui::End();
    }
}

void DebugOverlayPushSlider(const char* title, v4* var, f32 min, f32 max) {
    if (GlobalDrawDebugOverlay) {
        if (ImGui::Begin("Debug overlay", nullptr, DebugOverlayFlags)) {
            ImGui::Separator();
            ImGui::SliderFloat4(title, var->data, min, max);
        }
        ImGui::End();
    }
}

void DebugOverlayPushToggle(const char* title, bool* var) {
    if (GlobalDrawDebugOverlay) {
        if (ImGui::Begin("Debug overlay", nullptr, DebugOverlayFlags)) {
            ImGui::Separator();
            ImGui::Checkbox(title, var);
        }
        ImGui::End();
    }
}
