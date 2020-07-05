#include "Profile.h"

void DrawDebugProfiler(ProfilerBuffer* buffer) {
    ImGui::Columns(4, nullptr, false);
    ImGui::SetColumnWidth(0, 200);
    ImGui::SetColumnWidth(1, 100);
    ImGui::SetColumnWidth(2, 100);
    ImGui::SetColumnWidth(3, 100);

    ImGui::Button("name");
    ImGui::NextColumn();
    ImGui::Button("cycles");
    ImGui::NextColumn();
    ImGui::Button("hits");
    ImGui::NextColumn();
    ImGui::Button("cy/hit");
    ImGui::Separator();
    ImGui::NextColumn();
    ImGui::Columns(1);
    if (ImGui::BeginChild("perf counters table")) {
        ImGui::Columns(4);
        ImGui::SetColumnWidth(0, 200);
        ImGui::SetColumnWidth(1, 100);
        ImGui::SetColumnWidth(2, 100);
        ImGui::SetColumnWidth(3, 100);
        auto oneOverTicksPerSecond = 1.0f / GetTicksPerSecond();
        for (usize i = 0; i < array_count(buffer->records); i++) {
            auto record = buffer->records + i;
            if (record->func) {
                ImGui::Text("%s(%lu)", record->func, record->line);
                ImGui::NextColumn();
                ImGui::Text("%.2f", (record->cyclesAvg * 1000.0f) * oneOverTicksPerSecond);
                ImGui::NextColumn();
                ImGui::Text("%lu", record->invokeCountAvg);
                ImGui::NextColumn();
                ImGui::Text("%.2f", (record->cyclesPerCallAvg * 1000.0f) * oneOverTicksPerSecond);
                ImGui::NextColumn();
            }
        }
    }
    ImGui::EndChild();
}

void UpdateDebugProfiler(ProfilerBuffer* buffer) {
    for (u32 i = 0 ; i < GlobalDebugCallRecordsCount; i++) {
        auto capture = GlobalDebugCallRecords + i;
        auto record = buffer->records + i;
        record->file = capture->file;
        record->func = capture->func;
        record->line = capture->line;
        record->cycleCount[buffer->at] = capture->cycleCount;
        record->invokeCount[buffer->at] = capture->invokeCount;
        record->cyclesAvg = 0;
        record->invokeCountAvg = 0;
        record->cyclesPerCallAvg = 0;
        for (u32 j = 0; j < ProfilerBufferRecord::Size; j++) {
            record->cyclesAvg += record->cycleCount[j];
            record->invokeCountAvg += record->invokeCount[j];
        }
        record->cyclesAvg /= ProfilerBufferRecord::Size;
        record->invokeCountAvg /= ProfilerBufferRecord::Size;
        if (record->invokeCountAvg != 0) {
            record->cyclesPerCallAvg = record->cyclesAvg / record->invokeCountAvg;
        } else {
            record->cyclesPerCallAvg = 0;
        }

        capture->cycleCount = 0;
        capture->invokeCount = 0;
    }
    buffer->at = (buffer->at + 1) % ProfilerBufferRecord::Size;
}
