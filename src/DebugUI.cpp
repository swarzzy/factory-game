#include "DebugUI.h"

void ChunkToolMain(DebugUI* ui) {
    auto context = GetContext();
    auto world = GetWorld();
    auto pool = &world->chunkPool;
    auto player = (SpatialEntity*)GetEntity(world, world->playerID);
    assert(player);
    iv3 min = IV3(I32::Max);
    iv3 max = IV3(I32::Min);
    auto chunk = pool->firstSimChunk;
    while (chunk) {
        if (chunk->p.x < min.x) min.x = chunk->p.x;
        if (chunk->p.y < min.y) min.y = chunk->p.y;
        if (chunk->p.z < min.z) min.z = chunk->p.z;

        if (chunk->p.x > max.x) max.x = chunk->p.x;
        if (chunk->p.y > max.y) max.y = chunk->p.y;
        if (chunk->p.z > max.z) max.z = chunk->p.z;

        chunk = chunk->nextActive;
    }
    if (ImGui::Button("Up##chunkLayerUP")) {
        if (ui->chunkLayer < max.y) ui->chunkLayer++;
    }
    ImGui::SameLine();
    if (ImGui::Button("Down##chunkLayerDown")) {
        if (ui->chunkLayer > min.y) ui->chunkLayer--;
    }
    ImGui::SameLine();
    if (ImGui::Button("Reset##chunkLayerDown")) {
        ui->chunkLayer = WorldPos::ToChunk(player->p).chunk.y;
    }
    ImGui::SameLine();
    ImGui::Text("Layer: %ld", (long)ui->chunkLayer);
    ImGui::SameLine();
    ImGui::Text("(chunks: %lu, active: %lu, visible: %lu)", (u32)world->chunkHashMap.entryCount, pool->simChunkCount, pool->renderedChunkCount);

    ImGui::Separator();

    //if(ImGui::BeginChild("map##chunkToolMainMap"), ImVec2(0.0f, 600.0f), false, ImGuiWindowFlags_HorizontalScrollbar) {

        ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(1.0f, 1.0f, 1.0f, 0.0f));

        ImGui::Button("##beginBtnCoordLabel", ImVec2(40.0f, 40.0f));
        ImGui::SameLine();

        for (i32 x = min.x; x <= max.x; x++) {
            char buffer[32];
            sprintf_s(buffer, 32, "%ld##btnCoordLabel", x);
            ImGui::Button(buffer, ImVec2(40.0f, 40.0f));
            ImGui::SameLine();
        }

        ImGui::PopStyleColor();
        ImGui::PopItemFlag();

        ImGui::NewLine();

        for (i32 z = min.z; z <= max.z; z++) {
            for (i32 x = min.x; x <= max.x; x++) {
                if (x == min.x) {
                    ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
                    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(1.0f, 1.0f, 1.0f, 0.0f));

                    char buffer[32];
                    sprintf_s(buffer, 32, "%ld##btnCoordLabel", z);
                    ImGui::Button(buffer, ImVec2(40.0f, 40.0f));
                    ImGui::SameLine();

                    ImGui::PopStyleColor();
                    ImGui::PopItemFlag();
                }
                auto chunk = GetChunk(world, IV3(x, ui->chunkLayer, z));
                if (chunk) {
                    char buffer[32];
                    if (chunk->simPropagationCount) {
                        sprintf_s(buffer, 32, "S##tile(%ld, %ld, %ld)", (long)chunk->p.x, (long)chunk->p.y, (long)chunk->p.z);
                    } else if (chunk->modified) {
                        sprintf_s(buffer, 32, "M##tile(%ld, %ld, %ld)", (long)chunk->p.x, (long)chunk->p.y, (long)chunk->p.z);
                    } else {
                        sprintf_s(buffer, 32, "##tile(%ld, %ld, %ld)", (long)chunk->p.x, (long)chunk->p.y, (long)chunk->p.z);
                    }

                    bool customStyle = false;
                    if (chunk->p == WorldPos::ToChunk(player->p).chunk) {
                        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(1.0f, 0.0f, 0.0f, 1.0f));
                        customStyle = true;
                    } else if (chunk->visible) {
                        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.0f, 1.0f, 0.0f, 1.0f));
                        customStyle = true;
                    } else if (chunk->active) {
                        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.9f, 0.7f, 0.0f, 1.0f));
                        customStyle = true;
                    } else {
                    }

                    if (ImGui::Button(buffer, ImVec2(40.0f, 40.0f))) {
                        ui->chunkToolState = ChunkToolState::ChunkInfo;
                        ui->selectedChunk = chunk->p;
                    }

                    if (ImGui::IsItemHovered()) {
                        ImGui::BeginTooltip();
                        ImGui::Text("(%ld, %ld, %ld)", (long)chunk->p.x, (long)chunk->p.y, (long)chunk->p.z);
                        ImGui::Text("visible: %s", chunk->visible ? "true" : "false");
                        ImGui::Text("active: %s", chunk->active ? "true" : "false");
                        ImGui::EndTooltip();
                    }
                    if (customStyle) {
                        ImGui::PopStyleColor();
                    }
                } else {
                    ImGui::Dummy(ImVec2(40.0f, 40.0f));
                }
                ImGui::SameLine();
            }
            ImGui::NewLine();
        }
        //}
        //ImGui::EndChild();
}

void ChunkToolChunkInfo(DebugUI* ui) {
    auto context = GetContext();
    auto world = GetWorld();
    auto pool = &world->chunkPool;
    auto chunk = GetChunk(world, ui->selectedChunk);

    if (ImGui::Button("Back##chunkInfoBack")) {
        ui->chunkToolState = ChunkToolState::Main;
    }

    ImGui::Separator();

    if (chunk) {
        ImGui::Text("Coord: {%ld, %ld, %ld}", (long)chunk->p.x, (long)chunk->p.y, (long)chunk->p.z);
        // TODO: Atomic read
        ImGui::BulletText("state: %s", ToString(chunk->state));
        ImGui::BulletText("locked: %s", chunk->locked ? "true" : "false");
        ImGui::BulletText("filled: %s", chunk->filled ? "true" : "false");
        ImGui::BulletText("primaryMeshValid: %s", chunk->primaryMeshValid ? "true" : "false");
        ImGui::BulletText("secondaryMeshValid: %s", chunk->secondaryMeshValid ? "true" : "false");
        ImGui::BulletText("remeshingAfterEdit: %s", chunk->remeshingAfterEdit ? "true" : "false");
        ImGui::BulletText("priority: %s", ToString(chunk->priority));
        ImGui::BulletText("shouldBeRemeshedAfterEdit: %s", chunk->shouldBeRemeshedAfterEdit ? "true" : "false");
        ImGui::BulletText("modified: %s", chunk->modified ? "true" : "false");
        ImGui::BulletText("active: %s", chunk->active ? "true" : "false");
        ImGui::BulletText("visible: %s", chunk->visible ? "true" : "false");
        ImGui::BulletText("primaryMeshPoolIndex: %lu", chunk->primaryMeshPoolIndex);
        ImGui::BulletText("secondaryMeshPoolIndex: %lu", chunk->secondaryMeshPoolIndex);
        ImGui::BulletText("simPropagationCount: %lu", chunk->simPropagationCount);
        if (ImGui::CollapsingHeader("Living entities")) {
            foreach(chunk->entityStorage) {
                auto info = GetEntityInfo(it->type);
                auto p = GetEntityPosition(it);
                ImGui::Text("[%llu] %s entity - %s, {%ld, %ld, %ld}", it->id, ToString(it->kind), info->name, p.block.x, p.block.y, p.block.z);
            }
        }

    }
}

void DebugUIUpdateAndRender(DebugUI* ui) {
    auto world = GetWorld();
    auto camera = world->camera;
    if (ui->chunkToolState != ChunkToolState::Closed) {
        if (camera->inputMode == GameInputMode::Game) {
            camera->inputMode = GameInputMode::InGameUI;
        }
        ImGui::SetNextWindowSize({650.0f, 650.0f}, ImGuiCond_FirstUseEver);
        auto windowFlags = ImGuiWindowFlags_HorizontalScrollbar; //ImGuiWindowFlags_NoResize; //ImGuiWindowFlags_AlwaysAutoResize;
        bool enabled = ui->chunkToolState != ChunkToolState::Closed;
        if (ImGui::Begin("Chunk tool", &enabled, windowFlags)) {
            switch (ui->chunkToolState) {
            case ChunkToolState::Main: { ChunkToolMain(ui); } break;
            case ChunkToolState::ChunkInfo: { ChunkToolChunkInfo(ui); } break;
            invalid_default();
            }
        }
        ImGui::End();
        if (!enabled) {
            ui->chunkToolState = ChunkToolState::Closed;
        }
    }
    if (ui->chunkToolState == ChunkToolState::Closed) {
        if (camera->inputMode == GameInputMode::InGameUI) {
            camera->inputMode = GameInputMode::Game;
        }
    }
}
