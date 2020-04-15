#pragma once

Texture LoadTextureFromFile(const char* filename, TextureFormat format = TextureFormat::Unknown, TextureWrapMode wrapMode = TextureWrapMode::Default, TextureFilter filter = TextureFilter::Default, DynamicRange range = DynamicRange::LDR);
Texture CreateTexture(i32 width, i32 height, TextureFormat format, TextureWrapMode wrapMode, TextureFilter filter, void* data = 0);
CubeTexture LoadCubemap(const char* backPath, const char* downPath, const char* frontPath, const char* leftPath, const char* rightPath, const char* upPath, DynamicRange range = DynamicRange::LDR, TextureFormat format = TextureFormat::Unknown, TextureFilter filter = TextureFilter::Default, TextureWrapMode wrapMode = TextureWrapMode::Default);
CubeTexture MakeEmptyCubemap(u32 w, u32 h, TextureFormat format, TextureFilter filter, TextureWrapMode wrapMode, bool useMips);

Mesh* LoadMeshFlux(const char* filename);


inline CubeTexture LoadCubemapLDR(const char* backPath, const char* downPath, const char* frontPath,
                               const char* leftPath, const char* rightPath, const char* upPath) {
    return LoadCubemap(backPath, downPath, frontPath, leftPath, rightPath, upPath, DynamicRange::LDR, TextureFormat::SRGB8);
}

inline CubeTexture LoadCubemapHDR(const char* backPath, const char* downPath, const char* frontPath,
                                  const char* leftPath, const char* rightPath, const char* upPath) {
    return LoadCubemap(backPath, downPath, frontPath, leftPath, rightPath, upPath, DynamicRange::HDR, TextureFormat::RGB16F);
}
