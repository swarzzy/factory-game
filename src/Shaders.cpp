#include "Shaders.h"

GLuint CompileGLSL(MemoryArena* tempArena, const char* name, const char* vertexSource, const char* fragmentSource)
{
    GLuint resultHandle = 0;

    GLuint vertexHandle = glCreateShader(GL_VERTEX_SHADER);
    if (vertexHandle)
    {
        glShaderSource(vertexHandle, 1, &vertexSource, nullptr);
        glCompileShader(vertexHandle);

        GLint vertexResult = 0;
        glGetShaderiv(vertexHandle, GL_COMPILE_STATUS, &vertexResult);
        if (vertexResult)
        {
            GLuint fragmentHandle;
            fragmentHandle = glCreateShader(GL_FRAGMENT_SHADER);
            if (fragmentHandle)
            {
                glShaderSource(fragmentHandle, 1, &fragmentSource, nullptr);
                glCompileShader(fragmentHandle);

                GLint fragmentResult = 0;
                glGetShaderiv(fragmentHandle, GL_COMPILE_STATUS, &fragmentResult);
                if (fragmentResult)
                {
                    GLint programHandle;
                    programHandle = glCreateProgram();
                    if (programHandle)
                    {
                        glAttachShader(programHandle, vertexHandle);
                        glAttachShader(programHandle, fragmentHandle);
                        glLinkProgram(programHandle);

                        GLint linkResult = 0;
                        glGetProgramiv(programHandle, GL_LINK_STATUS, &linkResult);
                        if (linkResult)
                        {
                            glDeleteShader(vertexHandle);
                            glDeleteShader(fragmentHandle);
                            resultHandle = programHandle;
                        }
                        else
                        {
                            i32 logLength;
                            glGetProgramiv(programHandle, GL_INFO_LOG_LENGTH, &logLength);
                            // TODO: Stop using alloca
                            auto frame = BeginTemporaryMemory(tempArena);
                            defer { EndTemporaryMemory(&frame); };
                            char* message = (char*)PushSize(tempArena, logLength);
                            glGetProgramInfoLog(programHandle, logLength, 0, message);
                            log_print("[Error]: Failed to link shader program (%s) \n%s\n", name, message);
                        }
                    }
                    else
                    {
                        assert(false, "Failed to create shader program");
                    }
                }
                else
                {
                    GLint logLength;
                    glGetShaderiv(fragmentHandle, GL_INFO_LOG_LENGTH, &logLength);
                    auto frame = BeginTemporaryMemory(tempArena);
                    defer { EndTemporaryMemory(&frame); };
                    char* message = (char*)PushSize(tempArena, logLength);
                    glGetShaderInfoLog(fragmentHandle, logLength, nullptr, message);
                    log_print("[Error]: Failed to compile frag shader (%s)\n%s\n", name, message);
                }
            }
            else
            {
                assert(false, "Failed to create fragment shader");
            }
        }
        else
        {
            GLint logLength;
            glGetShaderiv(vertexHandle, GL_INFO_LOG_LENGTH, &logLength);
            auto frame = BeginTemporaryMemory(tempArena);
            defer { EndTemporaryMemory(&frame); };
            char* message = (char*)PushSize(tempArena, logLength);
            glGetShaderInfoLog(vertexHandle, logLength, nullptr, message);
            log_print("[Error]: Failed to compile vertex shader (%s)\n%s", name, message);
        }
    }
    else
    {
        assert(false, "Falled to create vertex shader");
    }
    return resultHandle;
}

void RecompileShaders(MemoryArena* tempArena, Renderer* renderer)
{
    log_print("[Renderer] Recompiling %lu shaders\n", array_count(renderer->shaderHandles));
    for (u32x i = 0; i < array_count(renderer->shaderHandles); i++)
    {
        auto handle = renderer->shaderHandles[i];
        if (handle)
        {
            DeleteProgram(handle);
        }
        renderer->shaderHandles[i] = CompileGLSL(tempArena, ShaderNames[i], ShaderSources[i].vert, ShaderSources[i].frag);
    }
}

template <typename T, u32 Binding>
void UniformBufferInit(UniformBuffer<T, Binding>* buffer, u32 swapBufferCount, Allocator allocator) {
    buffer->currentBuffer = 0;
    buffer->mapped = false;
    buffer->allocator = allocator;
    buffer->bufferCount = swapBufferCount;
    buffer->size = sizeof(T) * swapBufferCount;
    buffer->buffers = (UniformBufferData*)allocator.Alloc(sizeof(UniformBufferData) * swapBufferCount, 0);
    assert(buffer->buffers);
    ClearArray(buffer->buffers, swapBufferCount);
    ReallocUniformBuffer(buffer);
}

template<typename T, u32 Binding>
inline void ReallocUniformBuffer(UniformBuffer<T, Binding>* buffer)
{
    glFinish();
    for (u32 i = 0; i < buffer->bufferCount; i++) {
        auto info = buffer->buffers + i;
        if (info->fence) {
            auto waitResult = glClientWaitSync(info->fence, 0, 0);
            if (waitResult == GL_TIMEOUT_EXPIRED || waitResult == GL_WAIT_FAILED) {
                log_print("[Renderer] Reallocating a uniform buffer which is currently used!\n");
                assert(false);
            }
            glDeleteSync(info->fence);
            info->fence = 0;
        }
        if (info->handle) {
            glDeleteBuffers(1, &info->handle);
            info->handle = 0;
        }

        glCreateBuffers(1, &info->handle);
        assert(info->handle);
        glNamedBufferStorage(info->handle, sizeof(T), 0, GL_DYNAMIC_STORAGE_BIT | GL_MAP_WRITE_BIT);
    }
}

template<typename T, u32 Binding>
T* UniformBufferMap(UniformBuffer<T, Binding>* buffer)
{
    timed_scope();
    assert(!buffer->mapped);

    auto availableBufferIndex = buffer->currentBuffer;
    auto info = buffer->buffers + availableBufferIndex;

    if (info->fence) {
        auto waitResult = glClientWaitSync(info->fence, 0, Globals::UniformBufferMaxTimeout);
        if (waitResult == GL_TIMEOUT_EXPIRED || waitResult == GL_WAIT_FAILED) {
            log_print("[Renderer] API has failed while waiting for uniform buffer");
        }
        glDeleteSync(info->fence);
        info->fence = 0;
    }

    buffer->mapped = true;
    // TODO: GL_MAP_FLUSH_EXPLICIT_BIT
    auto mem = (T*)glMapNamedBufferRange(info->handle, 0, sizeof(T), GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT | GL_MAP_UNSYNCHRONIZED_BIT);
    assert(mem);
    return mem;
}

template<typename T, u32 Binding>
void UniformBufferUnmap(UniformBuffer<T, Binding>* buffer)
{
    timed_scope();

    assert(buffer->mapped);
    auto bufferIndex = buffer->currentBuffer;
    buffer->currentBuffer = (buffer->currentBuffer + 1) % buffer->bufferCount;
    auto info = buffer->buffers + bufferIndex;
    glUnmapNamedBuffer(info->handle);

    glBindBuffer(GL_UNIFORM_BUFFER, info->handle);
    glBindBufferRange(GL_UNIFORM_BUFFER, Binding, info->handle, 0, sizeof(T));
    glBindBuffer(GL_UNIFORM_BUFFER, 0);

    info->fence = glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0);
    buffer->mapped = false;
}
