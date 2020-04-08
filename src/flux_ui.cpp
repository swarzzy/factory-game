#include "flux_ui.h"
#include "../ext/imgui/imgui_internal.h"

void DrawMenu(Ui* ui) {
    ImGui::BeginMainMenuBar();
    if (ImGui::BeginMenu("Tools")) {
        if (ImGui::MenuItem("Entity lister")) {
            ui->entityListerOpen = !ui->entityListerOpen;
        }
        if (ImGui::MenuItem("Entity inspector")) {
            ui->entityInspectorOpen = !ui->entityInspectorOpen;
        }
        if (ImGui::MenuItem("Asset manager")) {
            ui->assetManagerOpen = !ui->assetManagerOpen;
        }
        if (ImGui::MenuItem("Show bounding volumes", nullptr, ui->showBoundingVolumes)) {
            ui->showBoundingVolumes = !ui->showBoundingVolumes;
        }
        if (ImGui::MenuItem("Show debug overlay", nullptr, ui->showDebugOverlay)) {
            ui->showDebugOverlay = !ui->showDebugOverlay;
        }
        if (ImGui::MenuItem("Save world")) {
            ui->wantsSave = true;
        }
        if (ImGui::MenuItem("Load from")) {
            ui->wantsLoadLoadFrom = true;
        }
        if (ImGui::MenuItem("Save as")) {
            ui->wantsSaveAs = true;
        }
        ImGui::EndMenu();
    }
    ImGui::EndMainMenuBar();
}

void DrawAssetManager(Context* context) {
    auto ui = &context->ui;
    ImGuiIO* io = &ImGui::GetIO();
    //ImGui::SetNextWindowSize({200, 400});
    auto windowFlags = 0;//ImGuiWindowFlags_NoResize; //ImGuiWindowFlags_AlwaysAutoResize;
    if (ImGui::Begin("Asset manager", (bool*)&ui->assetManagerOpen, windowFlags)) {
        if (ImGui::BeginTabBar("Asset manager tab bar")) {
            if (ImGui::BeginTabItem("Meshes")) {
                ImGui::Text("Load:");
                ImGui::SameLine();
                ImGui::PushID("Load mesh filename input");
                ImGui::InputText("", ui->assetLoadFileBuffer, array_count(ui->assetLoadFileBuffer));
                ImGui::PopID();
                //ImGui::SameLine();
                char formatString[16];
                sprintf_s(formatString, 16, "%s", ToString(ui->meshLoadFormat));
                ImGui::PushID("Mesh format selector");
                if (ImGui::BeginCombo("", formatString)) {
                    if (ImGui::Selectable("AAB", ui->meshLoadFormat == MeshFileFormat::AAB)) {
                        ui->meshLoadFormat = MeshFileFormat::AAB;
                    }
                    if (ImGui::Selectable("Flux", ui->meshLoadFormat == MeshFileFormat::Flux)) {
                        ui->meshLoadFormat = MeshFileFormat::Flux;
                    }
                    ImGui::EndCombo();
                }
                ImGui::PopID();
                ImGui::SameLine();
                if (ImGui::Button("Load")) {
                    AddMesh(&context->assetManager, ui->assetLoadFileBuffer, ui->meshLoadFormat);
                }

                ImGui::BeginChild("Asset manager list", {0.0f, -80.0f}, true);
                for (auto& slot : context->assetManager.meshTable) {
                    char buffer[150];
                    sprintf_s(buffer, 150, "[%lu] %s", (long)slot.id, slot.name);
                    bool selected = slot.id == ui->selectedMesh;
                    if (ImGui::Selectable(buffer, selected)) {
                        ui->selectedMesh = slot.id;
                    }
                }
                ImGui::EndChild();

                if (ImGui::Button("Unload")) {
                    if (ui->selectedMesh) {
                        UnloadMesh(&context->assetManager, ui->selectedMesh);
                    }
                }

                ImGui::SameLine();

                if (ImGui::Button("Delete")) {
                    if (ui->selectedMesh) {
                        RemoveMesh(&context->assetManager, ui->selectedMesh);
                    }
                }

                ImGui::Text("Asset info:");
                if (ui->selectedMesh) {
                    auto slot = GetMeshSlot(&context->assetManager, ui->selectedMesh);
                    if (slot) {
                        ImGui::Text("ID: %lu", slot->id);
                        ImGui::Text("Name: %s", slot->name);
                        ImGui::Text("File: %s", slot->filename);
                        ImGui::Text("State: %s", ToString(slot->state));
                    }
                }
                ImGui::EndTabItem();
            }
            if (ImGui::BeginTabItem("Textures")) {
                ImGui::Text("Load:");
                ImGui::SameLine();
                ImGui::PushID("Load texture filename input");
                ImGui::InputText("", ui->assetLoadFileBuffer, array_count(ui->assetLoadFileBuffer));
                ImGui::PopID();
                ImGui::SameLine();
                if (ImGui::Button("Load")) {
                    // TODO: properties
                    AddTexture(&context->assetManager, ui->assetLoadFileBuffer, (TextureFormat)ui->textureLoadFormat, (TextureWrapMode)ui->textureLoadWrapMode, (TextureFilter)ui->textureLoadFilter, (DynamicRange)ui->textureLoadDynamicRange);
                }

#define selectable_item(enumerator, uiVar) do { if (ImGui::Selectable(ToString(enumerator), ui->textureLoadFormat == (u32)enumerator)) { uiVar = (u32)enumerator; } } while (false)

                ImGui::PushItemWidth(100);
                if (ImGui::BeginCombo("format", ToString((TextureFormat)ui->textureLoadFormat))) {
                    selectable_item(TextureFormat::Unknown, ui->textureLoadFormat);
                    selectable_item(TextureFormat::SRGBA8, ui->textureLoadFormat);
                    selectable_item(TextureFormat::SRGB8, ui->textureLoadFormat);
                    selectable_item(TextureFormat::RGBA8, ui->textureLoadFormat);
                    selectable_item(TextureFormat::RGB8, ui->textureLoadFormat);
                    selectable_item(TextureFormat::RGB16F, ui->textureLoadFormat);
                    selectable_item(TextureFormat::RG16F, ui->textureLoadFormat);
                    selectable_item(TextureFormat::R8, ui->textureLoadFormat);
                    selectable_item(TextureFormat::RG8, ui->textureLoadFormat);
                    ImGui::EndCombo();
                }

                ImGui::SameLine();

                ImGui::PushItemWidth(100);
                if (ImGui::BeginCombo("filter", ToString((TextureFilter)ui->textureLoadFilter))) {
                    selectable_item(TextureFilter::None, ui->textureLoadFilter);
                    selectable_item(TextureFilter::Bilinear, ui->textureLoadFilter);
                    selectable_item(TextureFilter::Trilinear, ui->textureLoadFilter);
                    selectable_item(TextureFilter::Anisotropic, ui->textureLoadFilter);
                    ImGui::EndCombo();
                }

                ImGui::PushItemWidth(100);
                if (ImGui::BeginCombo("wrap mode", ToString((TextureWrapMode)ui->textureLoadWrapMode))) {
                    selectable_item(TextureWrapMode::Repeat, ui->textureLoadWrapMode);
                    selectable_item(TextureWrapMode::ClampToEdge, ui->textureLoadWrapMode);
                    ImGui::EndCombo();
                }

                ImGui::SameLine();

                ImGui::PushItemWidth(100);
                if (ImGui::BeginCombo("range", ToString((DynamicRange)ui->textureLoadDynamicRange))) {
                    selectable_item(DynamicRange::LDR, ui->textureLoadDynamicRange);
                    selectable_item(DynamicRange::HDR, ui->textureLoadDynamicRange);
                    ImGui::EndCombo();
                }
#undef selectable_item

                ImGui::BeginChild("Texture manager list", {0.0f, -80.0f}, true);
                for (auto& slot : context->assetManager.textureTable) {
                    char buffer[150];
                    sprintf_s(buffer, 150, "[%lu] %s", (long)slot.id, slot.name);
                    bool selected = slot.id == ui->selectedTexture;
                    if (ImGui::Selectable(buffer, selected)) {
                        ui->selectedTexture = slot.id;
                    }
                }
                ImGui::EndChild();

                if (ImGui::Button("Unload")) {
                    if (ui->selectedTexture) {
                        UnloadTexture(&context->assetManager, ui->selectedTexture);
                    }
                }

                ImGui::SameLine();

                if (ImGui::Button("Delete")) {
                    if (ui->selectedTexture) {
                        RemoveTexture(&context->assetManager, ui->selectedTexture);
                    }
                }

                ImGui::Text("Texture info:");
                if (ui->selectedTexture) {
                    auto slot = GetTextureSlot(&context->assetManager, ui->selectedTexture);
                    if (slot) {
                        ImGui::Text("ID: %lu", slot->id);
                        ImGui::Text("Name: %s", slot->name);
                        ImGui::Text("File: %s", slot->filename);
                        ImGui::Text("State: %s", ToString(slot->state));
                        ImGui::Text("Format: %s", ToString(slot->format));
                        ImGui::Text("Filter: %s", ToString(slot->filter));
                        ImGui::Text("Wrap mode: %s", ToString(slot->wrapMode));
                        ImGui::Text("Dynamic range: %s", ToString(slot->range));
                    }
                }
                ImGui::EndTabItem();
            }
        }

        ImGui::EndTabBar();
    }
    ImGui::End();
}

void DrawEntityLister(Context* context) {
    auto ui = &context->ui;
    ImGuiIO* io = &ImGui::GetIO();
    ImGui::SetNextWindowSize({200, 400});
    auto windowFlags = ImGuiWindowFlags_NoResize; //ImGuiWindowFlags_AlwaysAutoResize;
    if (ImGui::Begin("Entity lister", (bool*)&ui->entityListerOpen, windowFlags)) {
        ImGui::BeginChild("Entity lister list");
        for (Entity& e : context->world->entityTable) {
            bool wasSelected = (e.id == ui->selectedEntity);
            char buffer[128];
            sprintf_s(buffer, 128, "id: %lu", (uint)e.id);
            bool selected = ImGui::Selectable(buffer, wasSelected);
            if (selected) {
                ui->selectedEntity = e.id;
            }
        }
        ImGui::EndChild();
    }
    ImGui::End();
}

u32 DrawTextureCombo(AssetManager* assetManager, u32 id, const char* name) {
    u32 newID = id;
    char buffer[256];
    auto textreSlot = GetTextureSlot(assetManager, id);
    if (textreSlot) {
        sprintf_s(buffer, 256, "%s", textreSlot->name);
    } else {
        buffer[0] = 0;
    }

    if (ImGui::BeginCombo(name, buffer)) {
        for (auto& asset : assetManager->textureTable) {
            sprintf_s(buffer, 256, "[%lu] %s",(long)asset.id, asset.name);
            bool wasSelected = asset.id == id;
            bool selected = ImGui::Selectable(buffer, wasSelected);
            if (selected) {
                newID = asset.id;
            }
        }
        ImGui::EndCombo();
    }
    return newID;
}


void DrawEntityInspector(Context* context, Ui* ui, World* world) {
    auto io = ImGui::GetIO();
    ImGui::SetNextWindowSize({300, 600});
    auto windowFlags = ImGuiWindowFlags_NoResize; //ImGuiWindowFlags_AlwaysAutoResize;
    if (ImGui::Begin("Entity inspector", (bool*)&ui->entityInspectorOpen, windowFlags)) {
        if (ImGui::Button("Add entity")) {
            ui->wantsAddEntity = true;
        }
        if (!ui->selectedEntity)  {
            ImGui::Text("No entity selected");
        } else {
            auto entity = Get(&world->entityTable, &ui->selectedEntity);

            char buffer[16];
            sprintf_s(buffer, 16, "%lu", (uint)ui->selectedEntity);
            ImGui::Text("id: %s", buffer);
            ImGui::SameLine();
            if (ImGui::Button("Delete")) {
                ui->selectedEntity = 0;
                DeleteEntity(world, entity->id);
            } else {
                ImGui::Separator();
                ImGui::Text("Position");
                ImGui::PushID("Entity position drag");
                ImGui::DragFloat3("", entity->p.data);
                ImGui::PopID();

                ImGui::Separator();
                ImGui::Text("Scale");
                bool uniform = ui->uniformEntityScale;
                ImGui::Checkbox("Uniform scale", &uniform);
                ui->uniformEntityScale = uniform;
                ImGui::PushID("Entity scale drag");
                if (ui->uniformEntityScale) {
                    ImGui::DragFloat("", &entity->scale.x);
                    entity->scale.y = entity->scale.x;
                    entity->scale.z = entity->scale.x;
                } else {
                    ImGui::DragFloat3("", entity->scale.data);
                }
                ImGui::PopID();

                ImGui::Separator();
                ImGui::Text("Rotation");
                ImGui::SliderFloat3("Angles", entity->rotationAngles.data, 0.0f, 360.0f);

                ImGui::Separator();
                ImGui::Text("Mesh");
                ImGui::PushID("Entity inspector mesh combo");
                char buffer[256];
                auto meshSlot = GetMeshSlot(&context->assetManager, entity->mesh);
                if (meshSlot) {
                    sprintf_s(buffer, 256, "%s", meshSlot->name);
                } else {
                    buffer[0] = 0;
                }
                if (ImGui::BeginCombo("", buffer)) {
                    for (auto& asset : context->assetManager.meshTable) {
                        sprintf_s(buffer, 256, "[%lu] %s",(long)asset.id, asset.name);
                        bool wasSelected = asset.id == entity->mesh;
                        bool selected = ImGui::Selectable(buffer, wasSelected);
                        if (selected) {
                            entity->mesh = asset.id;
                        }
                    }
                    ImGui::EndCombo();
                }
                ImGui::PopID();

                ImGui::Separator();
                if (ImGui::CollapsingHeader("Material")) {
                    if (ImGui::BeginCombo("workflow", ToString(entity->material.workflow))) {
                        Material::Workflow newWorkflow = entity->material.workflow;
                        if (ImGui::Selectable("Phong")) {
                            newWorkflow = Material::Phong;
                        }
                        if (ImGui::Selectable("PBR metallic")) {
                            newWorkflow = Material::PBRMetallic;
                        }
                        if (ImGui::Selectable("PBR specular")) {
                            newWorkflow = Material::PBRSpecular;
                        }
                        if (newWorkflow != entity->material.workflow) {
                            entity->material = {};
                            entity->material.workflow = newWorkflow;
                        }
                        ImGui::EndCombo();
                    }

                    switch (entity->material.workflow) {
                    case Material::Phong: {
                        bool useDiffuseMap = entity->material.phong.useDiffuseMap;
                        ImGui::Checkbox("Diffuse map", &useDiffuseMap);
                        entity->material.phong.useDiffuseMap = useDiffuseMap;
                        if (entity->material.phong.useDiffuseMap) {
                            entity->material.phong.diffuseMap = DrawTextureCombo(&context->assetManager, entity->material.phong.diffuseMap, "diffuse map");
                        } else {
                            ImGui::ColorEdit3("Diffuse color", entity->material.phong.diffuseValue.data);
                        }

                        bool useSpecularMap = entity->material.phong.useSpecularMap;
                        ImGui::Checkbox("Specular map", &useSpecularMap);
                        entity->material.phong.useSpecularMap = useSpecularMap;
                        if (entity->material.phong.useSpecularMap) {
                            entity->material.phong.specularMap = DrawTextureCombo(&context->assetManager, entity->material.phong.specularMap, "specular map");
                        } else {
                            ImGui::ColorEdit3("Specular color", entity->material.phong.specularValue.data);
                        }
                    } break;
                    case Material::PBRMetallic: {
                        bool useAlbedoMap = entity->material.pbrMetallic.useAlbedoMap;
                        ImGui::Checkbox("Albedo map", &useAlbedoMap);
                        entity->material.pbrMetallic.useAlbedoMap = useAlbedoMap;
                        if (entity->material.pbrMetallic.useAlbedoMap) {
                            entity->material.pbrMetallic.albedoMap = DrawTextureCombo(&context->assetManager, entity->material.pbrMetallic.albedoMap, "albedo map");
                        } else {
                            ImGui::ColorEdit3("Albedo", entity->material.pbrMetallic.albedoValue.data);
                        }

                        bool useRoughnessMap = entity->material.pbrMetallic.useRoughnessMap;
                        ImGui::Checkbox("Roughness map", &useRoughnessMap);
                        entity->material.pbrMetallic.useRoughnessMap = useRoughnessMap;
                        if (entity->material.pbrMetallic.useRoughnessMap) {
                            entity->material.pbrMetallic.roughnessMap = DrawTextureCombo(&context->assetManager, entity->material.pbrMetallic.roughnessMap, "roughness map");
                        } else {
                            ImGui::SliderFloat("Roughness", &entity->material.pbrMetallic.roughnessValue, 0.0f, 1.0f);
                        }

                        bool useMetallicMap = entity->material.pbrMetallic.useMetallicMap;
                        ImGui::Checkbox("Metallic map", &useMetallicMap);
                        entity->material.pbrMetallic.useMetallicMap = useMetallicMap;
                        if (entity->material.pbrMetallic.useMetallicMap) {
                            entity->material.pbrMetallic.metallicMap = DrawTextureCombo(&context->assetManager, entity->material.pbrMetallic.metallicMap, "metallic map");
                        } else {
                            ImGui::SliderFloat("Metallic", &entity->material.pbrMetallic.metallicValue, 0.0f, 1.0f);
                        }

                        bool useNormalMap = entity->material.pbrMetallic.useNormalMap;
                        ImGui::Checkbox("Normal map", &useNormalMap);
                        entity->material.pbrMetallic.useNormalMap = useNormalMap;
                        if (entity->material.pbrMetallic.useNormalMap) {
                            if (ImGui::BeginCombo("Normal format select", ToString(entity->material.pbrMetallic.normalFormat))) {
                                if (ImGui::Selectable("OpenGL", entity->material.pbrMetallic.normalFormat == NormalFormat::OpenGL)) {
                                    entity->material.pbrMetallic.normalFormat = NormalFormat::OpenGL;
                                }
                                if (ImGui::Selectable("DirectX", entity->material.pbrMetallic.normalFormat == NormalFormat::DirectX)) {
                                    entity->material.pbrMetallic.normalFormat = NormalFormat::DirectX;
                                }
                                ImGui::EndCombo();
                            }
                            entity->material.pbrMetallic.normalMap = DrawTextureCombo(&context->assetManager, entity->material.pbrMetallic.normalMap, "normal map");
                        }

                        bool useAOMap = entity->material.pbrMetallic.useAOMap;
                        ImGui::Checkbox("AO map", &useAOMap);
                        entity->material.pbrMetallic.useAOMap = useAOMap;
                        if (entity->material.pbrMetallic.useAOMap) {
                            entity->material.pbrMetallic.AOMap = DrawTextureCombo(&context->assetManager, entity->material.pbrMetallic.AOMap, "ao map");
                        }

                        bool emitsLight = entity->material.pbrMetallic.emitsLight;
                        ImGui::Checkbox("Emits light", &emitsLight);
                        entity->material.pbrMetallic.emitsLight = emitsLight;

                        if (emitsLight) {
                            bool useEmissionMap = entity->material.pbrMetallic.useEmissionMap;
                            ImGui::Checkbox("Emission map", &useEmissionMap);
                            entity->material.pbrMetallic.useEmissionMap = useEmissionMap;

                            if (entity->material.pbrMetallic.useEmissionMap) {
                                entity->material.pbrMetallic.emissionMap = DrawTextureCombo(&context->assetManager, entity->material.pbrMetallic.emissionMap, "emission map");
                            } else {
                                ImGui::ColorEdit3("emission color", entity->material.pbrMetallic.emissionValue.data);
                                ImGui::SliderFloat("emission intensity", &entity->material.pbrMetallic.emissionIntensity, 0.0f, 10.0f);
                            }
                        }
                    } break;
                    default: {} break;
                    }
                }
            }
        }
    }
    ImGui::End();
}

// TODO: Factor loading logic out if this
World* LoadWorldFrom(Ui* ui, AssetManager* assetManager) {
    World* world = nullptr;
    if (!ImGui::IsPopupOpen("Load world from")) {
        ImGui::OpenPopup("Load world from");
    }
    if (ImGui::BeginPopupModal("Load world from", 0, ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::PushID("Load world from filename input");
        if (ImGui::InputText("", ui->worldNameBuffer, array_count(ui->worldNameBuffer))) {
            mbstowcs(ui->worldNameBufferW, ui->worldNameBuffer, array_count(ui->worldNameBuffer));
        }
        ImGui::PopID();

        if (ImGui::Button("Load")) {
            world = LoadWorldFromDisc(assetManager, ui->worldNameBufferW);
            if (world) {
                strcpy_s(world->name, array_count(world->name), ui->worldNameBuffer);
                ui->wantsLoadLoadFrom = false;
                ui->worldNameBuffer[0] = 0;
                ui->worldNameBufferW[0] = 0;
                ImGui::CloseCurrentPopup();
            }
        }

        ImGui::SameLine();

        if (ImGui::Button("Close")) {
            ui->wantsLoadLoadFrom = false;
            ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();
    }
    return world;
}

// TODO: Factor saving logic out of this
void SaveWorldAs(Ui* ui, World* world, AssetManager* assetManager) {
    if (!ImGui::IsPopupOpen("Save world as")) {
        ImGui::OpenPopup("Save world as");
    }
    if (ImGui::BeginPopupModal("Save world as", 0, ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::PushID("Save world as filename input");
        if (ImGui::InputText("", ui->worldNameBuffer, array_count(ui->worldNameBuffer))) {
            mbstowcs(ui->worldNameBufferW, ui->worldNameBuffer, array_count(ui->worldNameBuffer));
        }
        ImGui::PopID();

        if (ImGui::Button("Save")) {
            if (!PlatformDebugGetFileSize(ui->worldNameBufferW)) {
                if (SaveToDisk(assetManager, world, ui->worldNameBufferW)) {
                    strcpy_s(world->name, array_count(world->name), ui->worldNameBuffer);
                    ui->wantsSaveAs = false;
                    ui->worldNameBuffer[0] = 0;
                    ui->worldNameBufferW[0] = 0;
                    ImGui::CloseCurrentPopup();
                }
            }
        }
    }

    ImGui::SameLine();

    if (ImGui::Button("Close")) {
        ui->wantsSaveAs = false;
        ImGui::CloseCurrentPopup();
    }
    ImGui::EndPopup();
}

bool IsMouseCapturedByUI() {
    ImGuiIO* io = &ImGui::GetIO();
    bool result = io->WantCaptureMouse;
    return result;
}

bool IsKeyboardCapturedByUI() {
    ImGuiIO* io = &ImGui::GetIO();
    bool result = io->WantCaptureKeyboard;
    return result;
}


// NOTE: Reference https://github.com/ocornut/imgui/issues/2109#issuecomment-430096134
void DrawTestDock() {
    bool open = true;

    ImGuiWindowFlags window_flags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking;
    ImGuiViewport* viewport = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(viewport->Pos);
    ImGui::SetNextWindowSize(viewport->Size);
    ImGui::SetNextWindowViewport(viewport->ID);
    ImGui::SetNextWindowBgAlpha(0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
    window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
    window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;

    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
    ImGui::Begin("Main dockspace window", &open, window_flags);
    ImGui::PopStyleVar();
    ImGui::PopStyleVar(2);


    if (ImGui::DockBuilderGetNode(ImGui::GetID("MainDockspace")) == NULL) {
        ImGuiID dockspace_id = ImGui::GetID("MainDockspace");
        ImGuiViewport* viewport = ImGui::GetMainViewport();
        ImGui::DockBuilderRemoveNode(dockspace_id); // Clear out existing layout
        ImGui::DockBuilderAddNode(dockspace_id, ImGuiDockNodeFlags_DockSpace); // Add empty node
        ImGui::DockBuilderSetNodeSize(dockspace_id, viewport->Size);

        ImGuiID dock_main_id = dockspace_id; // This variable will track the document node, however we are not using it here as we aren't docking anything into it.a
        ImGuiID dock_id_left = ImGui::DockBuilderSplitNode(dock_main_id, ImGuiDir_Left, 0.15f, NULL, &dock_main_id);
        ImGuiID dock_id_right = ImGui::DockBuilderSplitNode(dock_main_id, ImGuiDir_Right, 0.25f, NULL, &dock_main_id);
        ImGuiID dock_id_bottom = ImGui::DockBuilderSplitNode(dock_main_id, ImGuiDir_Down, 0.20f, NULL, &dock_main_id);

        ImGui::DockBuilderDockWindow("Entity lister", dock_id_left);
        ImGui::DockBuilderDockWindow("Entity inspector", dock_id_right);
        ImGui::DockBuilderDockWindow("Asset manager", dock_id_right);
        //ImGui::DockBuilderDockWindow("Debug overlay", dock_id_bottom);
        ImGui::DockBuilderFinish(dockspace_id);
    }

    ImGui::PushStyleColor(ImGuiCol_DockingEmptyBg, 0);
    ImGuiID dockspace_id = ImGui::GetID("MainDockspace");
    ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), ImGuiDockNodeFlags_PassthruCentralNode);
    ImGui::PopStyleColor();
    ImGui::End();
}

void UpdateUi(Context* context) {
    auto ui = &context->ui;
    auto world = context->world;

    DrawTestDock();
#if 0
    ImGuiViewport* viewport = ImGui::GetMainViewport();
    auto dockspace_size = viewport->Size;

    ImGuiID dockspace_id = ImGui::GetID("Main dockspace");
    ImGui::DockBuilderRemoveNode(dockspace_id); // Clear out existing layout
    ImGui::DockBuilderAddNode(dockspace_id, ImGuiDockNodeFlags_DockSpace); // Add empty node
    ImGui::DockBuilderSetNodeSize(dockspace_id, dockspace_size);

    ImGuiID dock_main_id = dockspace_id; // This variable will track the document node, however we are not using it here as we aren't docking anything into it.
    ImGuiID dock_id_prop = ImGui::DockBuilderSplitNode(dock_main_id, ImGuiDir_Left, 0.20f, NULL, &dock_main_id);
    ImGuiID dock_id_bottom = ImGui::DockBuilderSplitNode(dock_main_id, ImGuiDir_Down, 0.20f, NULL, &dock_main_id);

    ImGui::DockBuilderDockWindow("Entity inspector", dock_id_prop);
    ImGui::DockBuilderFinish(dockspace_id);
#endif
    //auto mainDockID = ImGui::GetID("Main dockspace");

    //auto dockRight = ImGui::DockBuilderSplitNode(mainDockID, ImGuiDir_Right, 0.2f, nullptr, &mainDockID);
    //ImGui::DockBuilderDockWindow("Entity inspector", dockRight);

    //ImGui::DockBuilderFinish(mainDockID);

    DrawMenu(ui);

    GlobalDrawDebugOverlay = ui->showDebugOverlay;

    if (ui->entityListerOpen) {
        DrawEntityLister(context);
    }
    if (ui->entityInspectorOpen) {
        DrawEntityInspector(context, ui, world);
    }
    if (ui->assetManagerOpen) {
        DrawAssetManager(context);
    }

    auto rd = context->camera.mouseRay;
    auto ro = context->camera.position;
    if (!IsMouseCapturedByUI() && !IsKeyboardCapturedByUI()) {
        if (MouseButtonPressed(MouseButton::Left)) {
            DEBUG_OVERLAY_TRACE(context->camera.position);
            auto raycast = Raycast(context, &context->assetManager, world, ro, rd);
            if (raycast) {
                ui->selectedEntity = raycast.Unwrap().entityId;
            }
        }
    }
}
