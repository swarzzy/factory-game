#pragma once

struct Ui {
    b32 entityListerOpen = 1;
    b32 entityInspectorOpen = 1;
    b32 assetManagerOpen;
    b32 uniformEntityScale;
    b32 wantsAddEntity;
    b32 wantsSave;
    b32 wantsLoadLoadFrom;
    b32 wantsSaveAs;
    u32 showBoundingVolumes;
    u32 showDebugOverlay;
    u32 selectedEntity;
    u32 selectedMesh;
    u32 selectedTexture;
    u32 textureLoadWrapMode = (u32)TextureWrapMode::Default;
    u32 textureLoadFormat = (u32)TextureFormat::Unknown;
    u32 textureLoadFilter = (u32)TextureFilter::Default;;
    u32 textureLoadDynamicRange = (u32)DynamicRange::LDR;
    MeshFileFormat meshLoadFormat;
    char assetLoadFileBuffer[MaxAssetPathSize];
    char worldNameBuffer[128];
    wchar_t worldNameBufferW[128];
};

static_assert(array_count(typedecl(Ui, worldNameBuffer)) == array_count(typedecl(Ui, worldNameBufferW)));

World* LoadWorldFrom(Ui* ui, AssetManager* assetManager);
void SaveWorldAs(Ui* ui, World* world, AssetManager* assetManager);

bool IsMouseCapturedByUI();
bool IsKeyboardCapturedByUI();
void UpdateUi(Context* context);
