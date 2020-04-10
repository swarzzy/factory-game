#include "flux_renderer.h"

#include "World.h"

#include "flux_std140.h"
#include "flux_shaders.h"

struct Renderer {
    union {
        Shaders shaders;
        GLuint shaderHandles[ShaderCount];
    };

    u32 uniformBufferAligment;
    u32 maxSupportedSampleCount;
    u32 maxTexArrayLayers;

    GLuint lineBufferHandle;
    GLuint chunkIndexBuffer;
    v4 clearColor;

    uv2 renderRes;
    u32 sampleCount;
    f32 gamma = 2.4f;
    f32 exposure = 1.0f;

    GLuint offscreenBufferHandle;
    GLuint offscreenColorTarget;
    GLuint offscreenDepthTarget;

    GLuint offscreenDownsampledBuffer;
    GLuint offscreenDownsampledColorTarget;

    static constexpr u32 RandomValuesTextureSize = 1024;
    // NOTE: Number of cascades is always 3
    static constexpr u32 NumShadowCascades = 3;
    GLuint shadowMapFramebuffers[NumShadowCascades];
    GLuint shadowMapDepthTarget;
    GLuint shadowMapDebugColorTarget;
    u32 shadowMapRes = 2048;
    GLuint randomValuesTexture;
    b32 stableShadows = true;
    b32 showShadowCascadesBoundaries;
    f32 shadowFilterScale = 1.0f;

    // TODO: Camera should cares about that
    m4x4 shadowCascadeViewProjMatrices[NumShadowCascades];
    f32 shadowCascadeBounds[NumShadowCascades];

    f32 shadowConstantBias = 0.004f;
    f32 shadowSlopeBiasScale = 1.2f;
    f32 shadowNormalBiasScale;

    GLuint srgbBufferHandle;
    GLuint srgbColorTarget;

    GLfloat maxAnisotropy;

    GLuint captureFramebuffer;

    GLuint BRDFLutHandle;

    b32 debugF;
    b32 debugD;
    b32 debugG;
    b32 debugNormals;

    static constexpr u32 TextureTransferBufferCount = array_count(typedecl(AssetManager, assetQueue));
    u32 textureTransferBuffersUsageCount;
    b32 textureTransferBuffersUsage[TextureTransferBufferCount];
    GLuint textureTransferBuffers[TextureTransferBufferCount];

    Material fallbackPhongMaterial;
    Material fallbackMetallicMaterial;
    // TODO: Check is using 0 handle has consistant behavior on all GPUs
    // Maybe we need to create some placeholder texture here
    GLuint nullTexture2D = 0;

    // NOTE: Chunk::Size * Chunk::Size * Chunk::Size * VerticesInQuad * QuadsInVoxel
    // Dividing by two because the maximum index count will be used when chunk has chuckerboard pattern,
    // i.e. only half of voxels are filled
    static constexpr uptr ChunkIndexBufferCount = Chunk::Size * Chunk::Size * Chunk::Size * 6 * 6 / 2;
    static constexpr uptr ChunkIndexBufferSize = Chunk::Size * Chunk::Size * Chunk::Size * 6 * 6 / 2 * sizeof(u32);

    GLuint chunkIndexBufferHandle;

    static constexpr u32 TerrainTextureSize = 256;
    GLuint terrainTexArray;

    UniformBuffer<ShaderFrameData, ShaderFrameData::Binding> frameUniformBuffer;
    UniformBuffer<ShaderMeshData, ShaderMeshData::Binding> meshUniformBuffer;
};

GLenum ToOpenGL(TextureWrapMode mode) {
    GLenum result;
    switch (mode) {
    case TextureWrapMode::Repeat: { result = GL_REPEAT; } break;
    case TextureWrapMode::ClampToEdge: { result = GL_CLAMP_TO_EDGE; } break;
        invalid_default();
    }
    return result;
}

struct GLTextureFormat {
    GLenum internal;
    GLenum format;
    GLenum type;
};

GLTextureFormat ToOpenGL(TextureFormat format) {
    GLTextureFormat result;
    switch (format) {
    case TextureFormat::SRGBA8: { result.internal = GL_SRGB8_ALPHA8; result.format = GL_RGBA; result.type = GL_UNSIGNED_BYTE; } break;
    case TextureFormat::SRGB8: { result.internal = GL_SRGB8; result.format = GL_RGB; result.type = GL_UNSIGNED_BYTE; } break;
    case TextureFormat::RGBA8: { result.internal = GL_RGBA8; result.format = GL_RGBA; result.type = GL_UNSIGNED_BYTE; } break;
    case TextureFormat::RGB8: { result.internal = GL_RGB8; result.format = GL_RGB; result.type = GL_UNSIGNED_BYTE; } break;
    case TextureFormat::RGB16F: { result.internal = GL_RGB16F; result.format = GL_RGB; result.type = GL_FLOAT; } break;
    case TextureFormat::RG16F: { result.internal = GL_RG16F; result.format = GL_RG; result.type = GL_FLOAT; } break;
    case TextureFormat::RG32F: { result.internal = GL_RG32F; result.format = GL_RG; result.type = GL_FLOAT; } break;
    case TextureFormat::R8: { result.internal = GL_R8; result.format = GL_RED; result.type = GL_UNSIGNED_BYTE; } break;
    case TextureFormat::RG8: { result.internal = GL_RG8; result.format = GL_RG; result.type = GL_UNSIGNED_BYTE; } break;
        invalid_default();
    }
    return result;
}

struct GLTextureFilter {
    GLenum min;
    GLenum mag;
    bool anisotropic;
};

GLTextureFilter ToOpenGL(TextureFilter filter) {
    GLTextureFilter result = {};
    switch (filter) {
    case TextureFilter::None: { result.min = GL_NEAREST; result.mag = GL_NEAREST; } break;
    case TextureFilter::Bilinear: { result.min = GL_LINEAR; result.mag = GL_LINEAR; } break;
    case TextureFilter::Trilinear: { result.min = GL_LINEAR_MIPMAP_LINEAR; result.mag = GL_LINEAR; } break;
    case TextureFilter::Anisotropic: { result.min = GL_LINEAR_MIPMAP_LINEAR; result.mag = GL_LINEAR; result.anisotropic = true; } break;
    }
    return result;
}

TexTransferBufferInfo GetTextureTransferBuffer(Renderer* renderer, u32 size) {
    TexTransferBufferInfo result = {};
    result.renderer = renderer;
    GLuint bufferHandle = 0;
    u32 index = 0;
    if (renderer->textureTransferBuffersUsageCount < Renderer::TextureTransferBufferCount) {
        for (u32x i = 0; i < Renderer::TextureTransferBufferCount; i++) {
            if (!renderer->textureTransferBuffersUsage[i]) {
                renderer->textureTransferBuffersUsage[i] = true;
                bufferHandle = renderer->textureTransferBuffers[i];
                index = i;
                renderer->textureTransferBuffersUsageCount++;
                break;
            }
        }
    }
    if (bufferHandle) {
        glBindBuffer(GL_PIXEL_UNPACK_BUFFER, bufferHandle);
        glBufferData(GL_PIXEL_UNPACK_BUFFER, size, 0, GL_STREAM_DRAW);
        result.ptr = glMapBufferRange(GL_PIXEL_UNPACK_BUFFER, 0, size, GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT);
        //((byte*)result.ptr)[size - 1] = 5;
        result.index = index;
        glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);
    }
    return result;
}

void CompleteTextureTransfer(TexTransferBufferInfo* info, Texture* texture) {
    auto renderer = info->renderer;
    if (info->ptr) {
        auto bufferHandle = renderer->textureTransferBuffers[info->index];
        glBindBuffer(GL_PIXEL_UNPACK_BUFFER, bufferHandle);
        glUnmapBuffer(GL_PIXEL_UNPACK_BUFFER);
        renderer->textureTransferBuffersUsage[info->index] = false;
        assert(renderer->textureTransferBuffersUsageCount > 0);
        renderer->textureTransferBuffersUsageCount--;

        GLuint handle;
        if (!texture->gpuHandle) {
            glGenTextures(1, &handle);
            assert(handle);
            texture->gpuHandle = handle;
        } else {
            handle = texture->gpuHandle;
        }

        glBindTexture(GL_TEXTURE_2D, handle);

        auto wrapMode = ToOpenGL(texture->wrapMode);
        auto format = ToOpenGL(texture->format);
        auto filter = ToOpenGL(texture->filter);

        glTexImage2D(GL_TEXTURE_2D, 0, format.internal, texture->width,
                     texture->height, 0, format.format, format.type, 0);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrapMode);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrapMode);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, filter.mag);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, filter.min);
        if (filter.anisotropic) {
            // TODO: Anisotropy value
            glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_ARB, 8.0f);
        }

        // TODO: Mips control
        glGenerateMipmap(GL_TEXTURE_2D);

        glBindTexture(GL_TEXTURE_2D, 0);

        glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);
        *info = {};
    }
}

void UploadToGPU(ChunkMesh* mesh) {
    if (!mesh->gpuHandle) {
        GLuint handle;
        glCreateBuffers(1, &handle);
        assert(handle);
        mesh->gpuHandle = handle;
    }

    GLuint handle = mesh->gpuHandle;
    glBindBuffer(GL_ARRAY_BUFFER, handle);
    uptr size = mesh->vertexCount * ChunkMesh::VertexSize;
    glBufferData(GL_ARRAY_BUFFER, 0, nullptr, GL_STATIC_DRAW);
    glBufferData(GL_ARRAY_BUFFER, size, nullptr, GL_STATIC_DRAW);
    void* ptr = glMapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY);

    uptr offset = 0;

    auto block = mesh->end;
    while (block) {
        memcpy((byte*)ptr + offset, block->vertices, sizeof(block->vertices[0]) * block->vertexCount);
        offset += sizeof(block->vertices[0]) * block->vertexCount;
        block = block->next;
    }

    block = mesh->end;
    while (block) {
        memcpy((byte*)ptr + offset, block->normals, sizeof(block->normals[0]) * block->vertexCount);
        offset += sizeof(block->normals[0]) * block->vertexCount;
        block = block->next;
    }

    block = mesh->end;
    while (block) {
        memcpy((byte*)ptr + offset, block->tangents, sizeof(block->tangents[0]) * block->vertexCount);
        offset += sizeof(block->tangents[0]) * block->vertexCount;
        block = block->next;
    }

    block = mesh->end;
    while (block) {
        memcpy((byte*)ptr + offset, block->values, sizeof(block->values[0]) * block->vertexCount);
        offset += sizeof(block->values[0]) * block->vertexCount;
        block = block->next;
    }

    assert(offset == size);

    glUnmapBuffer(GL_ARRAY_BUFFER);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}


void UploadToGPU(Texture* texture) {
    if (!texture->gpuHandle) {
        GLuint handle;
        glGenTextures(1, &handle);
        if (handle) {
            glBindTexture(GL_TEXTURE_2D, handle);

            auto wrapMode = ToOpenGL(texture->wrapMode);
            auto format = ToOpenGL(texture->format);
            auto filter = ToOpenGL(texture->filter);

            glTexImage2D(GL_TEXTURE_2D, 0, format.internal, texture->width,
                         texture->height, 0, format.format, format.type, texture->data);

            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrapMode);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrapMode);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, filter.mag);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, filter.min);
            if (filter.anisotropic) {
                // TODO: Anisotropy value
                glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_ARB, 8.0f);
            }

            // TODO: Mips control
            glGenerateMipmap(GL_TEXTURE_2D);

            glBindTexture(GL_TEXTURE_2D, 0);

            texture->gpuHandle = handle;
        }
    }
}

void UploadToGPU(CubeTexture* texture) {
    if (!texture->gpuHandle) {
        GLuint handle;
        glGenTextures(1, &handle);
        if (handle) {
            glBindTexture(GL_TEXTURE_CUBE_MAP, handle);

            auto wrapMode = ToOpenGL(texture->wrapMode);
            auto filter = ToOpenGL(texture->filter);
            auto format = ToOpenGL(texture->format);

            glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, wrapMode);
            glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, wrapMode);
            glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, wrapMode);

            glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, filter.mag);
            glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, filter.min);
            if (filter.anisotropic) {
                // TODO: Anisotropy value
                glTexParameterf(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAX_ANISOTROPY_ARB, 8.0f);
            }

            for (u32 i = 0; i < 6; i++) {
                void* data = texture->data[i];
                glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0,
                             format.internal, texture->width, texture->height, 0,
                             format.format, format.type, data);
            }

            if (texture->useMips) {
                glGenerateMipmap(GL_TEXTURE_CUBE_MAP);
            }

            glBindTexture(GL_TEXTURE_CUBE_MAP, 0);

            texture->gpuHandle = handle;
        }
    }
}

void UploadToGPU(Mesh* mesh) {
    while (mesh) {
        if (!mesh->gpuVertexBufferHandle && !mesh->gpuIndexBufferHandle) {
            GLuint vboHandle;
            GLuint iboHandle;
            glGenBuffers(1, &vboHandle);
            glGenBuffers(1, &iboHandle);
            if (vboHandle && iboHandle) {
                // NOTE: Using SOA layout of buffer
                uptr verticesSize = mesh->vertexCount * sizeof(v3);
                // TODO: this is redundant. Use only vertexCount
                uptr normalsSize = mesh->vertexCount * sizeof(v3);
                uptr uvsSize = mesh->vertexCount * sizeof(v2);
                uptr tangentsSize = mesh->vertexCount * sizeof(v3);
                uptr bitangentsSize = mesh->vertexCount * sizeof(v3);
                uptr indexBufferSize = mesh->indexCount * sizeof(u32);
                uptr vertexBufferSize = verticesSize + normalsSize + uvsSize + tangentsSize + bitangentsSize;

                glBindBuffer(GL_ARRAY_BUFFER, vboHandle);

                glBufferData(GL_ARRAY_BUFFER, vertexBufferSize, 0, GL_STATIC_DRAW);
                glBufferSubData(GL_ARRAY_BUFFER, 0, verticesSize, (void*)mesh->vertices);
                glBufferSubData(GL_ARRAY_BUFFER, verticesSize, normalsSize, (void*)mesh->normals);
                glBufferSubData(GL_ARRAY_BUFFER, verticesSize + normalsSize, uvsSize, (void*)mesh->uvs);
                glBufferSubData(GL_ARRAY_BUFFER, verticesSize + normalsSize + uvsSize, tangentsSize, (void*)mesh->tangents);
                glBufferSubData(GL_ARRAY_BUFFER, verticesSize + normalsSize + uvsSize + tangentsSize, bitangentsSize, (void*)mesh->bitangents);

                glBindBuffer(GL_ARRAY_BUFFER, 0);

                glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, iboHandle);

                glBufferData(GL_ELEMENT_ARRAY_BUFFER, indexBufferSize, (void*)mesh->indices, GL_STATIC_DRAW);

                glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

                mesh->gpuVertexBufferHandle = vboHandle;
                mesh->gpuIndexBufferHandle = iboHandle;
            }
        }
        mesh = mesh->next;
    }
}

void FreeGPUBuffer(u32 id) {
    GLuint handle = id;
    glDeleteBuffers(1, &handle);
}
void FreeGPUTexture(u32 id) {
    GLuint handle = id;
    glDeleteTextures(1, &handle);
}

void GenBRDFLut(const Renderer* renderer, Texture* t) {
    assert(t->gpuHandle);
    assert(t->wrapMode == TextureWrapMode::ClampToEdge);
    assert(t->filter == TextureFilter::Bilinear);
    assert(t->format == TextureFormat::RG32F);

    glUseProgram(renderer->shaders.BRDFIntegrator);

    glViewport(0, 0, t->width, t->height);

    glDisable(GL_DEPTH_TEST);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, renderer->captureFramebuffer);

    glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                           GL_TEXTURE_2D, t->gpuHandle, 0);
    glClear(GL_COLOR_BUFFER_BIT);
    glDrawArrays(GL_TRIANGLES, 0, 6);

    glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, 0, 0);

    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
    glFlush();
}

void ReloadShadowMaps(Renderer* renderer, u32 newResolution = 0) {
    // TODO: There are maybe could be a problems on some drivers
    // with changing framebuffer attachments so this code needs to be checked
    // on different GPUs and drivers
    if (newResolution) {
        renderer->shadowMapRes = newResolution;
    }
    glBindTexture(GL_TEXTURE_2D_ARRAY, renderer->shadowMapDepthTarget);
    glTexImage3D(GL_TEXTURE_2D_ARRAY, 0, GL_DEPTH_COMPONENT32, renderer->shadowMapRes, renderer->shadowMapRes, Renderer::NumShadowCascades, 0, GL_DEPTH_COMPONENT, GL_FLOAT, 0);
    glBindTexture(GL_TEXTURE_2D_ARRAY, renderer->shadowMapDebugColorTarget);
    glTexImage3D(GL_TEXTURE_2D_ARRAY, 0, GL_R8, renderer->shadowMapRes, renderer->shadowMapRes, Renderer::NumShadowCascades, 0, GL_RED, GL_UNSIGNED_BYTE, 0);
    glBindTexture(GL_TEXTURE_2D_ARRAY, 0);
}

uv2 GetRenderResolution(Renderer* renderer) {
    return renderer->renderRes;
}

u32 GetRenderSampleCount(Renderer* renderer) {
    return renderer->sampleCount;
}

u32 GetRenderMaxSampleCount(Renderer* renderer) {
    return renderer->maxSupportedSampleCount;
}

void ChangeRenderResolution(Renderer* renderer, uv2 newRes, u32 newSampleCount) {
    // TODO: There are maybe could be a problems on some drivers
    // with changing framebuffer attachments so this code needs to be checked
    // on different GPUs and drivers
    if (newSampleCount <= renderer->maxSupportedSampleCount) {
        renderer->renderRes = newRes;
        renderer->sampleCount = newSampleCount;
        glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, renderer->offscreenColorTarget);
        glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, newSampleCount, GL_RGBA16F, renderer->renderRes.x, renderer->renderRes.y, GL_FALSE);

        glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, renderer->offscreenDepthTarget);
        glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, newSampleCount, GL_DEPTH_COMPONENT32, renderer->renderRes.x, renderer->renderRes.y, GL_FALSE);

        glBindTexture(GL_TEXTURE_2D, renderer->offscreenDownsampledColorTarget);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, renderer->renderRes.x, renderer->renderRes.y, 0, GL_RGBA, GL_FLOAT, 0);

        glBindTexture(GL_TEXTURE_2D, renderer->srgbColorTarget);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, renderer->renderRes.x, renderer->renderRes.y, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);

        glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, 0);
        glBindTexture(GL_TEXTURE_2D, 0);
    }
}

Renderer* InitializeRenderer(uv2 renderRes, u32 sampleCount) {
    Renderer* renderer = nullptr;
    renderer = (Renderer*)PlatformAlloc(sizeof(Renderer));
    *renderer = {};

    RecompileShaders(renderer);

    GLint maxSamples = 1;
    glGetIntegerv(GL_MAX_SAMPLES, &maxSamples);
    renderer->maxSupportedSampleCount = maxSamples;
    assert(renderer->maxSupportedSampleCount >= sampleCount);
    glGetIntegerv(GL_UNIFORM_BUFFER_OFFSET_ALIGNMENT, (GLint*)&renderer->uniformBufferAligment);

    GLint maxTexArrayLayers = 256;
    glGetIntegerv(GL_MAX_ARRAY_TEXTURE_LAYERS, &maxTexArrayLayers);
    renderer->maxTexArrayLayers = (u32)maxTexArrayLayers;

    ReallocUniformBuffer(&renderer->frameUniformBuffer);
    ReallocUniformBuffer(&renderer->meshUniformBuffer);

    GLfloat maxAnisotropy;
    glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_ARB, &maxAnisotropy);
    renderer->maxAnisotropy = maxAnisotropy;

    renderer->gamma = 2.4f;
    renderer->exposure = 1.0f;
    renderer->renderRes = renderRes;
    renderer->sampleCount = sampleCount;

    GLuint lineBufferHandle;
    glGenBuffers(1, &lineBufferHandle);
    assert(lineBufferHandle);
    renderer->lineBufferHandle = lineBufferHandle;

    glGenFramebuffers(1, &renderer->offscreenBufferHandle);
    assert(renderer->offscreenBufferHandle);
    glGenTextures(1, &renderer->offscreenColorTarget);
    assert(renderer->offscreenColorTarget);
    glGenTextures(1, &renderer->offscreenDepthTarget);
    assert(renderer->offscreenDepthTarget);

    glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, renderer->offscreenColorTarget);
    glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, sampleCount, GL_RGBA16F, renderRes.x, renderRes.y, GL_FALSE);

    glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, renderer->offscreenDepthTarget);
    glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, sampleCount, GL_DEPTH_COMPONENT32, renderRes.x, renderRes.y, GL_FALSE);

    glBindFramebuffer(GL_FRAMEBUFFER, renderer->offscreenBufferHandle);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D_MULTISAMPLE, renderer->offscreenColorTarget, 0);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D_MULTISAMPLE, renderer->offscreenDepthTarget, 0);
    assert(glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE);

    glGenFramebuffers(1, &renderer->offscreenDownsampledBuffer);
    assert(renderer->offscreenDownsampledBuffer);
    glGenTextures(1, &renderer->offscreenDownsampledColorTarget);
    assert(renderer->offscreenDownsampledColorTarget);

    glBindTexture(GL_TEXTURE_2D, renderer->offscreenDownsampledColorTarget);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, renderRes.x, renderRes.y, 0, GL_RGBA, GL_FLOAT, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glBindFramebuffer(GL_FRAMEBUFFER, renderer->offscreenDownsampledBuffer);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, renderer->offscreenDownsampledColorTarget, 0);
    assert(glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE);

    glGenFramebuffers(1, &renderer->srgbBufferHandle);
    assert(renderer->srgbBufferHandle);
    glGenTextures(1, &renderer->srgbColorTarget);
    assert(renderer->srgbColorTarget);

    glBindTexture(GL_TEXTURE_2D, renderer->srgbColorTarget);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, renderRes.x, renderRes.y, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glBindFramebuffer(GL_FRAMEBUFFER, renderer->srgbBufferHandle);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, renderer->srgbColorTarget, 0);
    assert(glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    glGenFramebuffers(1, &renderer->captureFramebuffer);
    assert(renderer->captureFramebuffer);

    glGenFramebuffers(3, renderer->shadowMapFramebuffers);
    // TODO: Checking!
    assert(renderer->shadowMapFramebuffers[0]);

    { // Initializing depth targets
        glGenTextures(1, &renderer->shadowMapDepthTarget);
        glBindTexture(GL_TEXTURE_2D_ARRAY, renderer->shadowMapDepthTarget);

        glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE);
        glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL);

        glTexImage3D(GL_TEXTURE_2D_ARRAY, 0, GL_DEPTH_COMPONENT32, renderer->shadowMapRes, renderer->shadowMapRes, Renderer::NumShadowCascades, 0, GL_DEPTH_COMPONENT, GL_FLOAT, 0);
    }
    { // Initializing debug color targets
        glGenTextures(1, &renderer->shadowMapDebugColorTarget);
        glBindTexture(GL_TEXTURE_2D_ARRAY, renderer->shadowMapDebugColorTarget);

        glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE);
        glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL);

        glTexImage3D(GL_TEXTURE_2D_ARRAY, 0, GL_R8, renderer->shadowMapRes, renderer->shadowMapRes, Renderer::NumShadowCascades, 0, GL_RED, GL_UNSIGNED_BYTE, 0);
    }

    glBindTexture(GL_TEXTURE_2D_ARRAY, 0);

    for (u32x i = 0; i < Renderer::NumShadowCascades; i++) {
        glBindFramebuffer(GL_FRAMEBUFFER, renderer->shadowMapFramebuffers[i]);
        glFramebufferTextureLayer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, renderer->shadowMapDepthTarget, 0, i);
        glFramebufferTextureLayer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, renderer->shadowMapDebugColorTarget, 0, i);
        //glDrawBuffer(GL_NONE);
        //glReadBuffer(GL_NONE);
        assert(glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE);
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    ReloadShadowMaps(renderer);

    glGenTextures(1, &renderer->randomValuesTexture);
    assert(renderer->randomValuesTexture);
    {
        glBindTexture(GL_TEXTURE_1D, renderer->randomValuesTexture);
        defer { glBindTexture(GL_TEXTURE_1D, 0); };

        u8* randomTextureBuffer = (u8*)PlatformAlloc(sizeof(u8) * Renderer::RandomValuesTextureSize);
        defer { PlatformFree(randomTextureBuffer); };
        RandomSeries series = {};
        for (u32x i = 0; i < Renderer::RandomValuesTextureSize; i++)
        {
            randomTextureBuffer[i] = (u8)(RandomUnilateral(&series) * 255.0f);
        }
        glTexImage1D(GL_TEXTURE_1D, 0, GL_R8, Renderer::RandomValuesTextureSize, 0, GL_RED, GL_UNSIGNED_BYTE, randomTextureBuffer);
        glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    }

    if (!renderer->BRDFLutHandle) {
        Texture t = CreateTexture(512, 512, TextureFormat::RG32F, TextureWrapMode::ClampToEdge, TextureFilter::Bilinear);
        UploadToGPU(&t);
        assert(t.gpuHandle);
        GenBRDFLut(renderer, &t);
        renderer->BRDFLutHandle = t.gpuHandle;
#if 0
        void* bitmap = PlatformAlloc(sizeof(f32) * 2 * 512 * 512);
        defer { PlatformFree(bitmap); };
        glBindTexture(GL_TEXTURE_2D, renderer->BRDFLutHandle);
        glGetTexImage(GL_TEXTURE_2D, 0, GL_RG, GL_FLOAT, bitmap);
        PlatformDebugWriteFile(L"brdf_lut.aab", bitmap, sizeof(f32) * 2 * 512 * 512);
#endif
    }

    // NOTE: Texture transfer buffers
    glGenBuffers(Renderer::TextureTransferBufferCount, renderer->textureTransferBuffers);

    // NOTE: Fallback material
    renderer->fallbackPhongMaterial.workflow = Material::Phong;
    renderer->fallbackPhongMaterial.phong.useDiffuseMap = false;
    renderer->fallbackPhongMaterial.phong.useSpecularMap = false;
    renderer->fallbackPhongMaterial.phong.diffuseValue = V3(0.7f, 0.7f, 0.7f);
    renderer->fallbackPhongMaterial.phong.specularValue = V3(0.0f, 0.0f, 0.0f);

    renderer->fallbackMetallicMaterial.workflow = Material::PBRMetallic;
    renderer->fallbackMetallicMaterial.pbrMetallic.useAlbedoMap = false;
    renderer->fallbackMetallicMaterial.pbrMetallic.useRoughnessMap = false;
    renderer->fallbackMetallicMaterial.pbrMetallic.useMetallicMap = false;
    renderer->fallbackMetallicMaterial.pbrMetallic.useNormalMap = false;

    renderer->fallbackMetallicMaterial.pbrMetallic.albedoValue = V3(0.7f, 0.7f, 0.7f);
    renderer->fallbackMetallicMaterial.pbrMetallic.roughnessValue = 1.0f;
    renderer->fallbackMetallicMaterial.pbrMetallic.metallicValue = 0.0f;

    // NOTE: null texture
    glGenTextures(1, &renderer->nullTexture2D);
    assert(renderer->nullTexture2D);
    glBindTexture(GL_TEXTURE_2D, renderer->nullTexture2D);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    u8 nullTexData[] = {255, 0, 255, 255};
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullTexData);
    glBindTexture(GL_TEXTURE_2D, 0);

    // NOTE: Chunk index buffer
    static_assert(Renderer::ChunkIndexBufferCount % 6 == 0);
    GLuint chunkIndexBuffer;

    glGenBuffers(1, &chunkIndexBuffer);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, chunkIndexBuffer);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, Renderer::ChunkIndexBufferSize, 0, GL_STATIC_DRAW);
    u32* buffer = (u32*)glMapBuffer(GL_ELEMENT_ARRAY_BUFFER, GL_WRITE_ONLY);

    u32 k = 0;
    for (u32 i = 0; i < Renderer::ChunkIndexBufferCount; i += 6)
    {
        assert(i < Renderer::ChunkIndexBufferCount);
        buffer[i] = k;
        buffer[i + 1] = k + 1;
        buffer[i + 2] = k + 2;
        buffer[i + 3] = k + 2;
        buffer[i + 4] = k + 3;
        buffer[i + 5] = k + 0;
        k += 4;
    }

    glUnmapBuffer(GL_ELEMENT_ARRAY_BUFFER);
    renderer->chunkIndexBufferHandle = chunkIndexBuffer;

    GLuint terrainTexArray;
    glGenTextures(1, &terrainTexArray);
    glBindTexture(GL_TEXTURE_2D_ARRAY, terrainTexArray);

    // NOTE: https://www.khronos.org/opengl/wiki/Common_Mistakes#Automatic_mipmap_generation
    //glEnable(GL_TEXTURE_2D_ARRAY);

    glTexImage3D(GL_TEXTURE_2D_ARRAY, 0, GL_SRGB8,
                 Renderer::TerrainTextureSize, Renderer::TerrainTextureSize,
                 renderer->maxTexArrayLayers, 0, GL_RGB, GL_UNSIGNED_BYTE, 0);

    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameterf(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_LOD_BIAS, -0.1f); // -0.86 for trilinear
    glTexParameterf(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAX_ANISOTROPY_ARB, renderer->maxAnisotropy);

    glBindTexture(GL_TEXTURE_2D_ARRAY, 0);
    renderer->terrainTexArray = terrainTexArray;

    return renderer;
}

u16 VoxelValueToTerrainIndex(VoxelValue value) {
    u16 index = 0;
    switch(value) {
    case VoxelValue::Stone: { index = 0; } break;
    case VoxelValue::Grass: { index = 1; } break;
    invalid_default();
    }
    return index;
}

void SetVoxelTexture(Renderer* renderer, VoxelValue value, void* data) {
    glBindTexture(GL_TEXTURE_2D_ARRAY, renderer->terrainTexArray);
    glTexSubImage3D(GL_TEXTURE_2D_ARRAY, 0, 0, 0, VoxelValueToTerrainIndex(value),
                    Renderer::TerrainTextureSize, Renderer::TerrainTextureSize, 1,
                    GL_RGB, GL_UNSIGNED_BYTE, data);
    glGenerateMipmap(GL_TEXTURE_2D_ARRAY);
    glBindTexture(GL_TEXTURE_2D_ARRAY, renderer->terrainTexArray);
}

void GenIrradanceMap(const Renderer* renderer, CubeTexture* t, GLuint sourceHandle) {
    assert(t->gpuHandle);
#if defined(PROFILE_IRRADANCE_GEN)
    SOKO_INFO("Generating irradance map...");
    glFinish();
    i64 beginTime = GetTimeStamp();
#endif

    // TODO: Make this constexpr
    static auto projInv = Inverse(PerspectiveGLRH(0.1, 10.0f, 90.0f, 1.0f)).Unwrap();
    static m3x3 capViews[] = {
        M3x3(Inverse(LookAtGLRH(V3(0.0f), V3(1.0f, 0.0f, 0.0f), V3(0.0f, -1.0f, 0.0f))).Unwrap()),
        M3x3(Inverse(LookAtGLRH(V3(0.0f), V3(-1.0f, 0.0f, 0.0f), V3(0.0f, -1.0f, 0.0f))).Unwrap()),
        M3x3(Inverse(LookAtGLRH(V3(0.0f), V3(0.0f, 1.0f, 0.0f), V3(0.0f, 0.0f, 1.0f))).Unwrap()),
        M3x3(Inverse(LookAtGLRH(V3(0.0f), V3(0.0f, -1.0f, 0.0f), V3(0.0f, 0.0f, -1.0f))).Unwrap()),
        M3x3(Inverse(LookAtGLRH(V3(0.0f), V3(0.0f, 0.0f, 1.0f), V3(0.0f, -1.0f, 0.0f))).Unwrap()),
        M3x3(Inverse(LookAtGLRH(V3(0.0f), V3(0.0f, 0.0f, -1.0f), V3(0.0f, -1.0f, 0.0f))).Unwrap()),
    };

    auto prog = renderer->shaders.IrradanceConvolver;
    glUseProgram(prog);

    auto buffer = Map(renderer->frameUniformBuffer);
    buffer->invProjMatrix = projInv;
    Unmap(renderer->frameUniformBuffer);

    glDisable(GL_DEPTH_TEST);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, renderer->captureFramebuffer);

    glBindTextureUnit(IrradanceConvolver::SourceCubemap, sourceHandle);

    for (u32 i = 0; i < 6; i++) {
        glViewport(0, 0, t->width, t->height);
        auto buffer = Map(renderer->frameUniformBuffer);
        buffer->invViewMatrix = M4x4(capViews[i]);
        Unmap(renderer->frameUniformBuffer);

        glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                               GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, t->gpuHandle, 0);
        glClear(GL_COLOR_BUFFER_BIT);

        glDrawArrays(GL_TRIANGLES, 0, 6);
    }
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
    glFlush();
#if defined (PROFILE_IRRADANCE_GEN)
    glFinish();
    i64 timeElapsed = GetTimeStamp() - beginTime;
    SOKO_INFO("Time: %i64 us", timeElapsed);
#endif

}

void GenEnvPrefiliteredMap(const Renderer* renderer, CubeTexture* t, GLuint sourceHandle, u32 mipLevels) {
    assert(t->gpuHandle);
    assert(t->useMips);
    assert(t->filter == TextureFilter::Trilinear);

    const m4x4 capProj = Inverse(PerspectiveGLRH(0.1, 10.0f, 90.0f, 1.0f)).Unwrap();
    const m3x3 capViews[] = {
        M3x3(Inverse(LookAtGLRH(V3(0.0f), V3(1.0f, 0.0f, 0.0f), V3(0.0f, -1.0f, 0.0f))).Unwrap()),
        M3x3(Inverse(LookAtGLRH(V3(0.0f), V3(-1.0f, 0.0f, 0.0f), V3(0.0f, -1.0f, 0.0f))).Unwrap()),
        M3x3(Inverse(LookAtGLRH(V3(0.0f), V3(0.0f, 1.0f, 0.0f), V3(0.0f, 0.0f, 1.0f))).Unwrap()),
        M3x3(Inverse(LookAtGLRH(V3(0.0f), V3(0.0f, -1.0f, 0.0f), V3(0.0f, 0.0f, -1.0f))).Unwrap()),
        M3x3(Inverse(LookAtGLRH(V3(0.0f), V3(0.0f, 0.0f, 1.0f), V3(0.0f, -1.0f, 0.0f))).Unwrap()),
        M3x3(Inverse(LookAtGLRH(V3(0.0f), V3(0.0f, 0.0f, -1.0f), V3(0.0f, -1.0f, 0.0f))).Unwrap()),
    };

    auto prog = renderer->shaders.EnvMapPrefilter;
    glUseProgram(prog);
    glDisable(GL_DEPTH_TEST);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, renderer->captureFramebuffer);

    assert(t->width == t->height);
    glUniform1i(EnvMapPrefilterShader::Resolution, t->width);

    auto buffer = Map(renderer->frameUniformBuffer);
    buffer->invProjMatrix = capProj;
    Unmap(renderer->frameUniformBuffer);

    glBindTextureUnit(EnvMapPrefilterShader::SourceCubemap, sourceHandle);

    // TODO: There are still visible seams on low mip levels

    for (u32 mipLevel = 0; mipLevel < mipLevels; mipLevel++) {
        // TODO: Pull texture size out of imges and put to a cubemap itself
        // all sides should havethe same size
        u32 w = (u32)(t->width * Pow(0.5f, (f32)mipLevel));
        u32 h = (u32)(t->height * Pow(0.5f, (f32)mipLevel));

        glViewport(0, 0, w, h);
        f32 roughness = (f32)mipLevel / (f32)(mipLevels - 1);
        glUniform1f(EnvMapPrefilterShader::Roughness, roughness);

        for (u32 i = 0; i < 6; i++) {
            // TODO: Use another buffer for this
            auto buffer = Map(renderer->frameUniformBuffer);
            buffer->invViewMatrix = M4x4(capViews[i]);
            Unmap(renderer->frameUniformBuffer);
            glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                                   GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, t->gpuHandle, mipLevel);
            glClear(GL_COLOR_BUFFER_BIT);
            glDrawArrays(GL_TRIANGLES, 0, 6);
        }
    }
    glFlush();
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
}

void DrawSkybox(Renderer* renderer, RenderGroup* group, const m4x4* invView, const m4x4* invProj) {
    glDepthMask(GL_FALSE);
    defer { glDepthMask(GL_TRUE); };

    auto prog = renderer->shaders.Skybox;
    glUseProgram(prog);

    glBindTextureUnit(SkyboxShader::CubeTexture, group->skyboxHandle);

    glDrawArrays(GL_TRIANGLES, 0, 6);
}

m3x3 MakeNormalMatrix(m4x4 model) {
    auto inverted= Inverse(model).Unwrap();
    inverted = Transpose(inverted);
    auto normal = M3x3(inverted);
    return normal;
}

// TODO: @Speed Make shure this function inlined
m4x4 CalcShadowProjection(const CameraBase* camera, f32 nearPlane, f32 farPlane, m4x4 lightLookAt, u32 shadowMapRes, bool stable) {
    // NOTE Shadow projection bounds
    v3 min;
    v3 max;

    if (stable) {
        v4 bSphereP;
        f32 bSphereR;

        // NOTE: Computing frustum bounding sphere
        // Reference: https://lxjk.github.io/2017/04/15/Calculate-Minimal-Bounding-Sphere-of-Frustum.html
        f32 ar = 1.0f / camera->aspectRatio;
        f32 k = Sqrt(1.0f + ar * ar) * Tan(ToRad(camera->fovDeg) * 0.5f);
        f32 kSq = k * k;
        f32 f = farPlane;
        f32 n = nearPlane;//  - 100.0f; // TODO: Manual offsets
        if (kSq >= ((f - n) / (f + n))) {
            bSphereP = V4(0.0f, 0.0f, -f, 1.0f);
            bSphereR = f * k;
        } else {
            bSphereP = V4(0.0f, 0.0f, -0.5f * (f + n) * (1.0f + kSq), 1.0f);
            bSphereR = 0.5f * Sqrt((f - n) * (f - n) + 2.0f * (f * f + n * n) * kSq + (f + n) * (f + n) * kSq * kSq);
        }

        // TODO: Fix this!
        // Grow sphere a little bit in order to get some safe margin
        // Because it looks like cascase boundries calculation is
        // incorrect and causes artifacts on cascade edges, especially
        // between first and second cascade
        bSphereR *= 1.3f;

        // From camera space to world space
        bSphereP = camera->invViewMatrix * bSphereP;
        // From world space to light space
        bSphereP = lightLookAt * bSphereP;

        // Constructing AABB in light space
        v3 xAxis = V3(1.0f, 0.0f, 0.0f);
        v3 yAxis = V3(0.0f, 1.0f, 0.0f);
        v3 zAxis = V3(0.0f, 0.0f, 1.0f);

        min = bSphereP.xyz - (xAxis * bSphereR + yAxis * bSphereR + zAxis * bSphereR);
        max = bSphereP.xyz + (xAxis * bSphereR + yAxis * bSphereR + zAxis * bSphereR);

        // NOTE: Update: We cannot just clamp Z because it will change AABB size and
        // destroy the stability. So here are couls be problems if Z is positive
        // which is means that near plane depth would be negative.

        // We get min and max Z values in light space, so Z is negative forward therefore if sphere
        // is in front of the light source then we have negative values.
        // Clamp max Z to 0 because if it's positive, then we are behind the light source.
        //if (max.z > 0.0f) max.z = 0.0f;

        // Inverting Z sign because we will use it for building projection matrix.
        // So it describe distance to far plane.
        min.z = -min.z;
        max.z = -max.z;
        // Swapping values so we can use min as near plane distance and max for far plane
        auto tmp = min.z;
        min.z = max.z;
        max.z = tmp;

        auto bboxSideSize = Abs(max.x - min.x);
        f32 pixelSize = bboxSideSize / shadowMapRes;

        min.x = Round(min.x / pixelSize) * pixelSize;
        min.y = Round(min.y / pixelSize) * pixelSize;
        min.z = Round(min.z / pixelSize) * pixelSize;

        max.x = Round(max.x / pixelSize) * pixelSize;
        max.y = Round(max.y / pixelSize) * pixelSize;
        max.z = Round(max.z / pixelSize) * pixelSize;

        //_OVERLAY_TRACE(min);

    } else {
        Basis cameraBasis;
        cameraBasis.zAxis = Normalize(camera->front);
        cameraBasis.xAxis = Normalize(Cross(V3(0.0f, 1.0f, 0.0), cameraBasis.zAxis));
        cameraBasis.yAxis = Cross(cameraBasis.zAxis, cameraBasis.xAxis);
        cameraBasis.p = camera->position;

        auto camFrustumCorners = GetFrustumCorners(cameraBasis, camera->fovDeg, camera->aspectRatio, nearPlane, farPlane);
        for (u32x i = 0; i < array_count(camFrustumCorners.corners); i++) {
            // TODO: Fix this!
            // Grow it a little bit in order to get some safe margin
            // Because it looks like cascase boundries calculation is
            // incorrect and causes artifacts on cascade edges, especially
            // between first and second cascade.
            camFrustumCorners.corners[i] *=  1.2f;
            camFrustumCorners.corners[i] = (lightLookAt * V4(camFrustumCorners.corners[i], 1.0f)).xyz;
        }

        min = V3(F32::Max);
        max = V3(-F32::Max);

        for (u32x i = 0; i < array_count(camFrustumCorners.corners); i++) {
            auto corner = camFrustumCorners.corners[i];
            if (corner.x < min.x) min.x = corner.x;
            if (corner.x > max.x) max.x = corner.x;
            if (corner.y < min.y) min.y = corner.y;
            if (corner.y > max.y) max.y = corner.y;
            if (corner.z < min.z) min.z = corner.z;
            if (corner.z > max.z) max.z = corner.z;
        }

        v3 minViewSpace = min;
        v3 maxViewSpace = max;

        // NOTE: Inverting Z because right-handed Z is negative-forward
        // but ortho ptojections gets constructed from Z positive-far
        auto tmp = minViewSpace.z;
        minViewSpace.z = -maxViewSpace.z;
        maxViewSpace.z = -tmp;

        min = minViewSpace;
        max = maxViewSpace;
    }

    auto result = OrthoGLRH(min.x, max.x, min.y, max.y, min.z, max.z);
    return result;
}

void RenderShadowMap(Renderer* renderer, RenderGroup* group, AssetManager* manager) {
    if (group->commandQueueAt) {
        auto shader = renderer->shaders.Shadow;
        for (u32 i = 0; i < group->commandQueueAt; i++) {
            CommandQueueEntry* command = group->commandQueue + i;

            switch (command->type) {
            case RenderCommand::LineBegin:
            {
            } break;
            case RenderCommand::DrawMesh: {
                auto* data = (RenderCommandDrawMesh*)(group->renderBuffer + command->rbOffset);

                auto mesh = GetMesh(manager, data->meshID);
                if (mesh) {
                    auto normalMatrix = MakeNormalMatrix(data->transform);

                    auto meshBuffer = Map(renderer->meshUniformBuffer);
                    meshBuffer->modelMatrix = data->transform;
                    meshBuffer->normalMatrix = normalMatrix;
                    Unmap(renderer->meshUniformBuffer);

                    while (mesh) {
                        glBindBuffer(GL_ARRAY_BUFFER, mesh->gpuVertexBufferHandle);

                        auto posAttrLoc = ShadowPassShader::PositionAttribLocation;
                        glEnableVertexAttribArray(posAttrLoc);
                        glVertexAttribPointer(posAttrLoc, 3, GL_FLOAT, GL_FALSE, 0, 0);

                        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh->gpuIndexBufferHandle);
                        glDrawElements(GL_TRIANGLES, mesh->indexCount, GL_UNSIGNED_INT, 0);
                        mesh = mesh->next;
                    }
                }
            } break;
            }
        }
    }
}

void ShadowPass(Renderer* renderer, RenderGroup* group, AssetManager* manager) {
    auto light = &group->dirLight;
    auto camera = group->camera;


    glEnable(GL_POLYGON_OFFSET_FILL);
    defer { glDisable(GL_POLYGON_OFFSET_FILL); };
    glPolygonOffset(renderer->shadowSlopeBiasScale, 0.0f);
    glEnable(GL_DEPTH_TEST);
    glViewport(0, 0, (GLsizei)renderer->shadowMapRes, (GLsizei)renderer->shadowMapRes);

    auto shader = renderer->shaders.Shadow;
    glUseProgram(shader);

    for (u32x cascadeIndex = 0; cascadeIndex < Renderer::NumShadowCascades; cascadeIndex++) {
        auto viewProj = renderer->shadowCascadeViewProjMatrices[cascadeIndex];

        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, renderer->shadowMapFramebuffers[cascadeIndex]);
        glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
        glUniform1i(ShadowPassShader::CascadeIndexLocation, cascadeIndex);
        RenderShadowMap(renderer, group, manager);
    }
}

void MainPass(Renderer* renderer, RenderGroup* group, AssetManager* assetManager) {

    DEBUG_OVERLAY_SLIDER(renderer->gamma, 1.0f, 10.0f);
    DEBUG_OVERLAY_SLIDER(renderer->exposure, 0.0f, 10.0f);
    bool showShadowCascadesBoundaries = renderer->showShadowCascadesBoundaries;
    DEBUG_OVERLAY_TOGGLE(showShadowCascadesBoundaries);
    renderer->showShadowCascadesBoundaries = showShadowCascadesBoundaries;

    auto camera = group->camera;
    auto dirLight = group->dirLight;

    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, renderer->offscreenBufferHandle);
    glViewport(0, 0, renderer->renderRes.x, renderer->renderRes.y);
    glClearColor(renderer->clearColor.r, renderer->clearColor.g, renderer->clearColor.b, renderer->clearColor.a);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glEnable(GL_DEPTH_TEST);

    if (group->commandQueueAt) {
        for (u32 i = 0; i < group->commandQueueAt; i++) {
            CommandQueueEntry* command = group->commandQueue + i;

            switch (command->type) {
            case RenderCommand::BeginChunkBatch: {
                auto* data = (RenderCommandPushChunk*)(group->renderBuffer + command->rbOffset);
                auto program = renderer->shaders.Chunk;

                glUseProgram(program);

                glBindTextureUnit(ChunkShader::TerrainAtlasSampler, renderer->terrainTexArray);

                for (u32 i = 0; i < command->instanceCount; i++) {
                    auto entry = data + i;
                    m4x4 modelMatrix = Translate(entry->offset);
                    m3x3 normalMatrix = MakeNormalMatrix(modelMatrix);

                    glUniform3fv(ChunkShader::OffsetUniformLocation, 1, entry->offset.data);

                    auto* mesh = entry->mesh;

                    glBindBuffer(GL_ARRAY_BUFFER, mesh->gpuHandle);

                    glEnableVertexAttribArray(ChunkShader::Position);
                    glEnableVertexAttribArray(ChunkShader::Normal);
                    glEnableVertexAttribArray(ChunkShader::Tangent);
                    glEnableVertexAttribArray(ChunkShader::VoxelValue);

                    uptr normalsOffset = mesh->vertexCount * sizeof(v3);
                    uptr tangentsOffset = normalsOffset + mesh->vertexCount * sizeof(v3);
                    uptr valuesOffset = tangentsOffset + mesh->vertexCount * sizeof(v3);

                    glVertexAttribPointer(ChunkShader::Position, 3, GL_FLOAT, GL_FALSE, 0, 0);
                    glVertexAttribPointer(ChunkShader::Normal, 3, GL_FLOAT, GL_FALSE, 0, (void*)normalsOffset);
                    glVertexAttribPointer(ChunkShader::Tangent, 3, GL_FLOAT, GL_FALSE, 0, (void*)tangentsOffset);
                    glVertexAttribIPointer(ChunkShader::VoxelValue, 1, GL_UNSIGNED_SHORT, 0, (void*)valuesOffset);

                    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, renderer->chunkIndexBufferHandle);
                    // TODO: Look for glDrawElementsInstancedBaseInstance

                    u32 quadCount = mesh->vertexCount / 4;
                    u32 indexCount = quadCount * 6;
                    glDrawElements(GL_TRIANGLES, indexCount, GL_UNSIGNED_INT, 0);
                }
            } break;
            case RenderCommand::DrawWater: {
                auto* data = (RenderCommandDrawWater*)(group->renderBuffer + command->rbOffset);
                auto program = renderer->shaders.Water;

                m3x3 normalMatrix = MakeNormalMatrix(data->transform);

                glUseProgram(program);

                auto meshBuffer = Map(renderer->meshUniformBuffer);
                meshBuffer->modelMatrix = data->transform;
                meshBuffer->normalMatrix = normalMatrix;
                Unmap(renderer->meshUniformBuffer);

                auto* mesh = data->mesh;


                glBindBuffer(GL_ARRAY_BUFFER, mesh->gpuVertexBufferHandle);

                glEnableVertexAttribArray(WaterShader::Position);
                glEnableVertexAttribArray(WaterShader::Normal);
                glEnableVertexAttribArray(WaterShader::UV);

                u64 normalsOffset = mesh->vertexCount * sizeof(v3);
                u64 uvsOffset = normalsOffset + mesh->vertexCount * sizeof(v3);

                glVertexAttribPointer(WaterShader::Position, 3, GL_FLOAT, GL_FALSE, 0, 0);
                glVertexAttribPointer(WaterShader::Normal, 3, GL_FLOAT, GL_FALSE, 0, (void*)normalsOffset);
                glVertexAttribPointer(WaterShader::UV, 2, GL_FLOAT, GL_FALSE, 0, (void*)uvsOffset);

                glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh->gpuIndexBufferHandle);

                glDrawElements(GL_TRIANGLES, mesh->indexCount, GL_UNSIGNED_INT, 0);
            } break;
            case RenderCommand::LineBegin: {
                auto* data = (RenderCommandLineBegin*)(group->renderBuffer + command->rbOffset);

                glUseProgram(renderer->shaders.Line);

                auto meshBuffer = Map(renderer->meshUniformBuffer);
                meshBuffer->lineColor = data->color;
                Unmap(renderer->meshUniformBuffer);

                uptr bufferSize = command->instanceCount * sizeof(RenderCommandPushLineVertex);
                void* instanceData = (void*)((byte*)data + sizeof(RenderCommandLineBegin));

                glBindBuffer(GL_ARRAY_BUFFER, renderer->lineBufferHandle);
                glBufferData(GL_ARRAY_BUFFER, bufferSize, instanceData, GL_STATIC_DRAW);

                glEnableVertexAttribArray(0);
                glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(v3), 0);

                glLineWidth(data->width);

                GLuint lineType;
                switch (data->type) {
                case RenderCommandLineBegin::Segments: { lineType = GL_LINES; } break;
                case RenderCommandLineBegin::Strip: { lineType = GL_LINE_STRIP; } break;
                default: {lineType = GL_LINES; assert(false, "sdf"); } break;
                }

                glDrawArrays(lineType, 0, command->instanceCount);

            } break;
            case RenderCommand::DrawMesh: {
                auto data = (RenderCommandDrawMesh*)(group->renderBuffer + command->rbOffset);
                auto mesh = GetMesh(assetManager, data->meshID);
                if (mesh) {
                    if (data->material.workflow == Material::Phong) {
                        auto meshProg = renderer->shaders.Mesh;

                        glUseProgram(meshProg);
                        auto meshBuffer = Map(renderer->meshUniformBuffer);

                        glBindTextureUnit(MeshShader::ShadowMap, renderer->shadowMapDepthTarget);

                        if (data->material.phong.useDiffuseMap) {
                            auto diffuseMap = GetTexture(assetManager, data->material.phong.diffuseMap);
                            if (diffuseMap) {
                                meshBuffer->phongUseDiffuseMap = 1;
                                glBindTextureUnit(MeshShader::DiffMap, diffuseMap->gpuHandle);
                            } else {
                                meshBuffer->phongUseDiffuseMap = 0;
                                meshBuffer->customPhongDiffuse = renderer->fallbackPhongMaterial.phong.diffuseValue;
                            }
                        } else {
                            meshBuffer->phongUseDiffuseMap = 0;
                            meshBuffer->customPhongDiffuse = data->material.phong.diffuseValue;
                        }

                        if (data->material.phong.useSpecularMap) {
                            auto specularMap = GetTexture(assetManager, data->material.phong.specularMap);
                            if (specularMap) {
                                meshBuffer->phongUseSpecularMap = 1;
                                glBindTextureUnit(MeshShader::SpecMap, specularMap->gpuHandle);
                            } else {
                                meshBuffer->phongUseSpecularMap = 0;
                                meshBuffer->customPhongSpecular = renderer->fallbackPhongMaterial.phong.specularValue;
                            }
                        } else {
                            meshBuffer->phongUseSpecularMap = 0;
                            meshBuffer->customPhongSpecular = data->material.phong.specularValue;
                        }

                        m3x3 normalMatrix = MakeNormalMatrix(data->transform);

                        meshBuffer->modelMatrix = data->transform;
                        meshBuffer->normalMatrix = normalMatrix;
                        Unmap(renderer->meshUniformBuffer);

                        while (mesh) {
                            glBindBuffer(GL_ARRAY_BUFFER, mesh->gpuVertexBufferHandle);

                            glEnableVertexAttribArray(0);
                            glEnableVertexAttribArray(1);
                            glEnableVertexAttribArray(2);

                            u64 normalsOffset = mesh->vertexCount * sizeof(v3);
                            u64 uvsOffset = normalsOffset + mesh->vertexCount * sizeof(v3);

                            glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
                            glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, (void*)normalsOffset);
                            glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 0, (void*)uvsOffset);

                            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh->gpuIndexBufferHandle);

                            glDrawElements(GL_TRIANGLES, mesh->indexCount, GL_UNSIGNED_INT, 0);
                            mesh = mesh->next;
                        }
                    } else if (data->material.workflow == Material::PBRMetallic ||
                               data->material.workflow == Material::PBRSpecular) {
                        assert(group->irradanceMapHandle);

                        auto meshProg = renderer->shaders.PbrMesh;
                        glUseProgram(meshProg);

                        auto meshBuffer = Map(renderer->meshUniformBuffer);

                        // TODO: Are they need to be binded every shader invocation?
                        glBindTextureUnit(MeshPBRShader::IrradanceMap, group->irradanceMapHandle);
                        glBindTextureUnit(MeshPBRShader::EnviromentMap, group->envMapHandle);
                        glBindTextureUnit(MeshPBRShader::ShadowMap, renderer->shadowMapDepthTarget);

                        auto m = &data->material;

                        // Getting materials
                        switch (m->workflow) {
                        case Material::PBRMetallic: {
                            meshBuffer->metallicWorkflow = 1;

                            // TODO: Refactor these
                            if (data->material.pbrMetallic.useAlbedoMap) {
                                auto albedoMap = GetTexture(assetManager, data->material.pbrMetallic.albedoMap);
                                if (albedoMap) {
                                    meshBuffer->pbrUseAlbedoMap = 1;
                                    glBindTextureUnit(MeshPBRShader::AlbedoMap, albedoMap->gpuHandle);
                                } else {
                                    meshBuffer->pbrUseAlbedoMap = 0;
                                    meshBuffer->pbrAlbedoValue = renderer->fallbackMetallicMaterial.pbrMetallic.albedoValue;
                                }
                            } else {
                                meshBuffer->pbrUseAlbedoMap = 0;
                                meshBuffer->pbrAlbedoValue = data->material.pbrMetallic.albedoValue;
                            }

                            if (data->material.pbrMetallic.useRoughnessMap) {
                                auto roughnessMap = GetTexture(assetManager, data->material.pbrMetallic.roughnessMap);
                                if (roughnessMap) {
                                    meshBuffer->pbrUseRoughnessMap = 1;
                                    glBindTextureUnit(MeshPBRShader::RoughnessMap, roughnessMap->gpuHandle);
                                } else {
                                    meshBuffer->pbrUseRoughnessMap = 0;
                                    meshBuffer->pbrRoughnessValue = renderer->fallbackMetallicMaterial.pbrMetallic.roughnessValue;;
                                }
                            } else {
                                meshBuffer->pbrUseRoughnessMap = 0;
                                meshBuffer->pbrRoughnessValue = data->material.pbrMetallic.roughnessValue;
                            }

                            if (data->material.pbrMetallic.useMetallicMap) {
                                auto metallicMap = GetTexture(assetManager, data->material.pbrMetallic.metallicMap);
                                if (metallicMap) {
                                    meshBuffer->pbrUseMetallicMap = 1;
                                    glBindTextureUnit(MeshPBRShader::MetallicMap, metallicMap->gpuHandle);
                                } else {
                                    meshBuffer->pbrUseMetallicMap = 0;
                                    meshBuffer->pbrMetallicValue = renderer->fallbackMetallicMaterial.pbrMetallic.metallicValue;;
                                }
                            } else {
                                meshBuffer->pbrUseMetallicMap = 0;
                                meshBuffer->pbrMetallicValue = data->material.pbrMetallic.metallicValue;
                            }

                            if (data->material.pbrMetallic.useNormalMap) {
                                meshBuffer->normalFormat = m->pbrMetallic.normalFormat == NormalFormat::OpenGL ? 0 : 1;
                                auto normalMap = GetTexture(assetManager, data->material.pbrMetallic.normalMap);
                                if (normalMap) {
                                    meshBuffer->pbrUseNormalMap = 1;
                                    glBindTextureUnit(MeshPBRShader::NormalMap, normalMap->gpuHandle);
                                } else {
                                    meshBuffer->pbrUseNormalMap = 0;
                                }
                            } else {
                                meshBuffer->pbrUseNormalMap = 0;
                            }

                            if (data->material.pbrMetallic.useAOMap) {
                                auto aoMap = GetTexture(assetManager, data->material.pbrMetallic.AOMap);
                                if (aoMap) {
                                    meshBuffer->pbrUseAOMap = 1;
                                    glBindTextureUnit(MeshPBRShader::AOMap, aoMap->gpuHandle);
                                } else {
                                    meshBuffer->pbrUseAOMap = 0;
                                }
                            } else {
                                meshBuffer->pbrUseAOMap = 0;
                            }

                            if (data->material.pbrMetallic.emitsLight) {
                                meshBuffer->emitsLight = 1;
                                if (data->material.pbrMetallic.useEmissionMap) {
                                    auto emissionMap = GetTexture(assetManager, data->material.pbrMetallic.emissionMap);
                                    if (emissionMap) {
                                        meshBuffer->pbrUseEmissionMap = 1;
                                        glBindTextureUnit(MeshPBRShader::EmissionMap, emissionMap->gpuHandle);
                                    } else {
                                        meshBuffer->pbrUseEmissionMap = 0;
                                    }
                                } else {
                                    meshBuffer->pbrUseEmissionMap = 0;
                                    meshBuffer->pbrEmissionValue = data->material.pbrMetallic.emissionValue * data->material.pbrMetallic.emissionIntensity;
                                }
                            } else {
                                meshBuffer->emitsLight = 0;
                            }

                        } break;
                        case Material::PBRSpecular: {
                            // TODO: Implement
#if 0
                            meshBuffer->metallicWorkflow = 0;

                            auto albedoMap = GetTexture(assetManager, m->pbrMetallic.albedo);
                            auto specularMap = GetTexture(assetManager, m->pbrSpecular.specular);
                            auto glossMap = GetTexture(assetManager, m->pbrSpecular.gloss);
                            auto normalMap = GetTexture(assetManager, m->pbrMetallic.normals);

                            auto albedoHandle = albedoMap ? albedoMap->gpuHandle : renderer->nullTexture2D;
                            glBindTextureUnit(MeshPBRShader::AlbedoMap, albedoHandle);

                            auto specularHandle = specularMap ? specularMap->gpuHandle : renderer->nullTexture2D;
                            glBindTextureUnit(MeshPBRShader::SpecularMap, specularHandle);

                            auto glossHandle = glossMap ? glossMap->gpuHandle : renderer->nullTexture2D;
                            glBindTextureUnit(MeshPBRShader::GlossMap, glossHandle);

                            auto normalHandle = normalMap ? normalMap->gpuHandle : renderer->nullTexture2D;
                            glBindTextureUnit(MeshPBRShader::NormalMap, normalHandle);
#endif
                        } break;
                            invalid_default();
                        }

                        glBindTextureUnit(MeshPBRShader::BRDFLut, renderer->BRDFLutHandle);

                        auto normalMatrix = MakeNormalMatrix(data->transform);
                        meshBuffer->modelMatrix = data->transform;
                        meshBuffer->normalMatrix = normalMatrix;

                        Unmap(renderer->meshUniformBuffer);

                        while (mesh) {
                            glBindBuffer(GL_ARRAY_BUFFER, mesh->gpuVertexBufferHandle);

                            bool hasBitangents = mesh->bitangents ? true : false;

                            // TODO: This is very slow and stupid
                            // There are probably sould be different shaders for meshes that have bitangents
                            // and for those that do not.
                            // Or maybe bitangents sholdn't be optional at all?
                            auto meshBuffer = Map(renderer->meshUniformBuffer);
                            meshBuffer->hasBitangents = hasBitangents ? 1 : 0;
                            Unmap(renderer->meshUniformBuffer);


                            glEnableVertexAttribArray(0);
                            glEnableVertexAttribArray(1);
                            glEnableVertexAttribArray(2);
                            glEnableVertexAttribArray(3);
                            if (hasBitangents) {
                                glEnableVertexAttribArray(4);
                            }

                            u64 normalsOffset = mesh->vertexCount * sizeof(v3);
                            u64 uvsOffset = normalsOffset + mesh->vertexCount * sizeof(v3);
                            u64 tangentsOffset = uvsOffset + mesh->vertexCount * sizeof(v2);

                            glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
                            glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, (void*)normalsOffset);
                            glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 0, (void*)uvsOffset);
                            glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, 0, (void*)tangentsOffset);

                            if (hasBitangents) {
                                u64 bitangentsOffset = tangentsOffset + mesh->vertexCount * sizeof(v3);
                                glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, 0, (void*)bitangentsOffset);
                            }

                            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh->gpuIndexBufferHandle);

                            glDrawElements(GL_TRIANGLES, mesh->indexCount, GL_UNSIGNED_INT, 0);
                            mesh = mesh->next;
                        }
                    } else {
                        unreachable();
                    }
                }
            } break;
            }
        }
        Reset(group);
    }

    if (group->drawSkybox) {
        glDepthFunc(GL_EQUAL);
        DrawSkybox(renderer, group, &camera->invViewMatrix, &camera->invProjectionMatrix);
        glDepthFunc(GL_LESS);
    }
}

void Begin(Renderer* renderer, RenderGroup* group) {
    auto light = group->dirLight;
    auto camera = group->camera;

    DEBUG_OVERLAY_SLIDER(renderer->shadowSlopeBiasScale, 0.0f, 2.5f);
    DEBUG_OVERLAY_SLIDER(renderer->shadowConstantBias, 0.0f, 0.5f);


    i32 EnableStableShadows = renderer->stableShadows;
    DEBUG_OVERLAY_SLIDER(EnableStableShadows, 0, 1);
    renderer->stableShadows = EnableStableShadows;

    //
    // NOTE: Calculating light-space matrices
    //

    // TODO: Fix the mess with lookAt matrices4
    m4x4 lightLookAt = LookAtGLRH(light.from, light.from + light.dir, V3(0.0f, 1.0f, 0.0f));

    f32 frustrumDepth = camera->farPlane - camera->nearPlane;
    f32 cascadeDepth = frustrumDepth / Renderer::NumShadowCascades;
    for (u32x cascadeIndex = 0; cascadeIndex < Renderer::NumShadowCascades; cascadeIndex++) {
        f32 cascadeNear = cascadeDepth * cascadeIndex;
        f32 cascadeFar = cascadeDepth * (cascadeIndex + 1);
        renderer->shadowCascadeBounds[cascadeIndex] = camera->nearPlane + cascadeFar;
        auto proj = CalcShadowProjection(camera, cascadeNear, cascadeFar, lightLookAt, renderer->shadowMapRes, renderer->stableShadows);
        auto viewProj = proj * lightLookAt;
        renderer->shadowCascadeViewProjMatrices[cascadeIndex] = viewProj;
    }

    //
    // NOTE: Fill frame uniform buffer
    //

    m4x4 viewProj = camera->projectionMatrix * camera->viewMatrix;

    auto lightViewProj0 = renderer->shadowCascadeViewProjMatrices;
    auto lightViewProj1 = renderer->shadowCascadeViewProjMatrices + 1;
    auto lightViewProj2 = renderer->shadowCascadeViewProjMatrices + 2;

    auto frameBuffer = Map(renderer->frameUniformBuffer);
    frameBuffer->viewProjMatrix = viewProj;
    frameBuffer->viewMatrix = camera->viewMatrix;
    frameBuffer->projectionMatrix = camera->projectionMatrix;
    frameBuffer->invViewMatrix = camera->invViewMatrix;
    frameBuffer->invProjMatrix = camera->invProjectionMatrix;
    frameBuffer->lightSpaceMatrices[0] = *lightViewProj0;
    frameBuffer->lightSpaceMatrices[1] = *lightViewProj1;
    frameBuffer->lightSpaceMatrices[2] = *lightViewProj2;
    frameBuffer->dirLight.pos = light.from;
    frameBuffer->dirLight.dir = light.dir;
    frameBuffer->dirLight.ambient = light.ambient;
    frameBuffer->dirLight.diffuse = light.diffuse;
    frameBuffer->dirLight.specular = light.specular;
    frameBuffer->viewPos = camera->position;
    frameBuffer->shadowCascadeSplits = *((v3*)&renderer->shadowCascadeBounds);
    frameBuffer->showShadowCascadeBoundaries = (i32)renderer->showShadowCascadesBoundaries;
    frameBuffer->shadowFilterSampleScale = renderer->shadowFilterScale;
    frameBuffer->debugF = renderer->debugF;
    frameBuffer->debugG = renderer->debugG;
    frameBuffer->debugD = renderer->debugD;
    frameBuffer->debugNormals = renderer->debugNormals;
    frameBuffer->constShadowBias = renderer->shadowConstantBias;
    frameBuffer->gamma = renderer->gamma;
    frameBuffer->exposure = renderer->exposure;
    frameBuffer->screenSize = V2((f32)renderer->renderRes.x, (f32)renderer->renderRes.y);

    Unmap(renderer->frameUniformBuffer);
}

void End(Renderer* renderer) {
    glBindFramebuffer(GL_READ_FRAMEBUFFER, renderer->offscreenBufferHandle);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, renderer->offscreenDownsampledBuffer);
    glBlitFramebuffer(0, 0, renderer->renderRes.x, renderer->renderRes.y,
                      0, 0, renderer->renderRes.x, renderer->renderRes.y,
                      GL_COLOR_BUFFER_BIT, GL_LINEAR);

    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, renderer->srgbBufferHandle);
    //glClearColor(renderer->clearColor.r, renderer->clearColor.g, renderer->clearColor.b, renderer->clearColor.a);
    glClear(GL_COLOR_BUFFER_BIT);

    auto prog = renderer->shaders.PostFx;
    glUseProgram(prog);

    glBindTextureUnit(PostFxShader::ColorSourceLinear, renderer->offscreenDownsampledColorTarget);

    glDrawArrays(GL_TRIANGLES, 0, 6);

    // NOTE: FXAA pass
    static bool enableFXAA = true;
    DEBUG_OVERLAY_TOGGLE(enableFXAA);
    if (enableFXAA) {
        glBindFramebuffer(GL_READ_FRAMEBUFFER, renderer->srgbBufferHandle);
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
        //glClearColor(renderer->clearColor.r, renderer->clearColor.g,  renderer->clearColor.b, renderer->clearColor.a);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glUseProgram(renderer->shaders.FXAA);
        glBindTextureUnit(FXAAShader::ColorSourcePerceptual, renderer->srgbColorTarget);
        glDrawArrays(GL_TRIANGLES, 0, 6);
    } else {
        glBindFramebuffer(GL_READ_FRAMEBUFFER, renderer->srgbBufferHandle);
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);

        glBlitFramebuffer(0, 0, renderer->renderRes.x, renderer->renderRes.y,
                          0, 0, renderer->renderRes.x, renderer->renderRes.y,
                          GL_COLOR_BUFFER_BIT, GL_LINEAR);
    }

    static i32 showShadowMap = false;
    DEBUG_OVERLAY_SLIDER(showShadowMap, 0, 1);

    if (showShadowMap) {
        static i32 shadowCascadeLevel = 0;
        DEBUG_OVERLAY_SLIDER(shadowCascadeLevel, 0, Renderer::NumShadowCascades - 1);
        glBindFramebuffer(GL_READ_FRAMEBUFFER, renderer->shadowMapFramebuffers[shadowCascadeLevel]);
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);

        glBlitFramebuffer(0, 0, renderer->shadowMapRes, renderer->shadowMapRes,
                          0, 0, 512, 512, GL_COLOR_BUFFER_BIT, GL_LINEAR);
    }
}
