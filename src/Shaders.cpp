#include "Shaders.h"

GLuint CompileGLSL(const char* name, const char* vertexSource, const char* fragmentSource)
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
                            char* message = (char*)StackAlloc(logLength);
                            if (message) {
                                glGetProgramInfoLog(programHandle, logLength, 0, message);
                                log_print("[Error]: Failed to link shader program (%s) \n%s\n", name, message);
                            } else {
                                log_print("[Error]: Failed to link shader program (%s) \n", name);
                            }
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
                    char* message = (char*)StackAlloc(logLength);
                    if (message) {
                        glGetProgramInfoLog(fragmentHandle, logLength, 0, message);
                        log_print("[Error]: Failed to compile frag shader (%s) \n%s\n", name, message);
                    } else {
                        log_print("[Error]: Failed to compile frag shader (%s) \n", name);
                    }
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
            char* message = (char*)StackAlloc(logLength);
            if (message) {
                glGetProgramInfoLog(vertexHandle, logLength, 0, message);
                log_print("[Error]: Failed to compile vert shader (%s) \n%s\n", name, message);
            } else {
                log_print("[Error]: Failed to compile vert shader (%s) \n", name);
            }
        }
    }
    else
    {
        assert(false, "Falled to create vertex shader");
    }
    return resultHandle;
}

void RecompileShaders(Renderer* renderer)
{
    log_print("[Renderer] Recompiling %lu shaders\n", array_count(renderer->shaderHandles));
    for (u32x i = 0; i < array_count(renderer->shaderHandles); i++)
    {
        auto handle = renderer->shaderHandles[i];
        if (handle)
        {
            DeleteProgram(handle);
        }
        renderer->shaderHandles[i] = CompileGLSL(ShaderNames[i], ShaderSources[i].vert, ShaderSources[i].frag);
    }
}

template <typename T, u32 Binding>
void UniformBufferInit(UniformBuffer<T, Binding>* buffer) {
    buffer->mapped = false;
    ReallocUniformBuffer(buffer);
}

template<typename T, u32 Binding>
inline void ReallocUniformBuffer(UniformBuffer<T, Binding>* buffer)
{
    glFinish();
    if (buffer->handle) {
        glDeleteBuffers(1, &buffer->handle);
        buffer->handle = 0;
    }

    glCreateBuffers(1, &buffer->handle);
    assert(buffer->handle);
    glNamedBufferStorage(buffer->handle, sizeof(T), 0, GL_DYNAMIC_STORAGE_BIT | GL_MAP_WRITE_BIT);
}

template<typename T, u32 Binding>
T* UniformBufferMap(UniformBuffer<T, Binding>* buffer)
{
    timed_scope();
    assert(!buffer->mapped);

    buffer->mapped = true;
    // TODO: This is not a particulary good streaming scheme
    // it is not provides a good control
    // we need more elaborate scheme or just switch to another API
    auto mem = (T*)glMapNamedBufferRange(buffer->handle, 0, sizeof(T), GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT | GL_MAP_UNSYNCHRONIZED_BIT);
    assert(mem);
    return mem;
}

template<typename T, u32 Binding>
void UniformBufferUnmap(UniformBuffer<T, Binding>* buffer)
{
    timed_scope();

    assert(buffer->mapped);
    glUnmapNamedBuffer(buffer->handle);

    glBindBuffer(GL_UNIFORM_BUFFER, buffer->handle);
    glBindBufferRange(GL_UNIFORM_BUFFER, Binding, buffer->handle, 0, sizeof(T));
    glBindBuffer(GL_UNIFORM_BUFFER, 0);

    buffer->mapped = false;
}
