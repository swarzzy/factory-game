#define PLATFORM_WINDOWS
#include <windows.h>
#include "../Common.h"
#include "../Intrinsics.cpp"
#include <vector>
#include <string>
#include <sstream>

#include <stdlib.h>

#include "utils.cpp"

void Logger(void* data, const char* fmt, va_list* args) {
    vprintf(fmt, *args);
}

LoggerFn* GlobalLogger = Logger;
void* GlobalLoggerData = nullptr;

inline void AssertHandler(void* data, const char* file, const char* func, u32 line, const char* assertStr, const char* fmt, va_list* args) {
    log_print("[Assertion failed] Expression (%s) result is false\nFile: %s, function: %s, line: %d.\n", assertStr, file, func, (int)line);
    if (args) {
        GlobalLogger(GlobalLoggerData, fmt, args);
    }
    debug_break();
}

AssertHandlerFn* GlobalAssertHandler = AssertHandler;
void* GlobalAssertHandlerData = nullptr;

#define INVALID_DEFAULT_CASE() assert(false)

#undef ERROR
#define ERROR(...) (printf(__VA_ARGS__), exit(EXIT_FAILURE))

static i32x _IDENT_LEVEL = 0;
#define IDENT_PUSH() (_IDENT_LEVEL++)
#define IDENT_POP() (_IDENT_LEVEL--)
#define IDENT_RESET() (_IDENT_LEVEL = 0)
static FILE* OutFile = 0;
// NOTE: Out line
#define L(format,...) do {assert(_IDENT_LEVEL >= 0); for (i32x i = 0; i < _IDENT_LEVEL; i++) fprintf(OutFile, "    "); fprintf(OutFile, concat(format, "\n"), __VA_ARGS__);} while(false)
// NOTE: Out line no eol
#define O(format, ...) do {assert(_IDENT_LEVEL >= 0); for (i32x i = 0; i < _IDENT_LEVEL; i++) fprintf(OutFile, "    "); fprintf(OutFile, format, __VA_ARGS__);} while(false)
// NOTE: Append line
#define A(...) fprintf(OutFile, __VA_ARGS__)

const char* DELIMETERS = " \f\n\r\t\v\n\r";
const char* EOL_MARKERS = " \n\r";

struct Program
{
    std::string name;
    std::string vert;
    std::string frag;
    std::string vertSource;
    std::string fragSource;
};

bool OneOf(char c, const char* t)
{
    bool result = false;
    // TODO: Ensure this is compile time evaluated
    const auto size = StrLength(t);
    for (u32x i = 0; i < size; i++)
    {
        if (c == t[i])
        {
            result = true;
            break;
        }
    }
    return result;
}

char* EatUntilOneOf(char* at, const char* chars)
{
    while (*at && !OneOf(*at, chars)) at++;
    return at;
}

char* EatUntilSpace(char* at)
{
    while (*at && !IsSpace(*at)) at++;
    return at;
}

std::vector<Program> ParseConfigFile(const char* filename)
{
    std::vector<Program> programs;
    u32 size;
    char* file = ReadEntireFileAsText(filename, &size);
    char* at = file;
    while (*at)
    {
        at = EatSpace(at);
        Program prog = {};
        auto nameBeg = at;
        at = EatUntilSpace(at);
        prog.name = std::string(nameBeg, (uptr)at - (uptr)nameBeg);
        at = EatUntilOneOf(at, "\"");
        at++;
        auto vertBeg = at;
        at = EatUntilOneOf(at, "\"");
        prog.vert = std::string(vertBeg, (uptr)at - (uptr)vertBeg);
        at++;
        at = EatUntilOneOf(at, "\"");
        at++;
        auto fragBeg = at;
        at = EatUntilOneOf(at, "\"");
        prog.frag = std::string(fragBeg, (uptr)at - (uptr)fragBeg);
        at++;
        at = EatSpace(at);
        programs.push_back(std::move(prog));
    }
    return std::move(programs);
}

std::string PreprocessShader(const std::string path, const std::string& source)
{
    std::string result = "";
    std::stringstream stream(source);
    std::string line;
    u32 lineCount = 0;
    while (std::getline(stream, line))
    {
        lineCount++;
        auto includePos = line.find("#include");
        auto commentPos = line.find("//");
        if (includePos != std::string::npos)
        {
            if (commentPos != std::string::npos && commentPos < includePos)
            {
                continue;
            }
            line.erase(0, sizeof("#include"));
            while (isspace((unsigned char)line[0])) line.erase(0, 1);
            while (isspace((unsigned char)line.back())) line.pop_back();
            auto dir = GetDirectory(path);
            std::string filename = dir + line;
            u32 size;
            auto includeSource = ReadEntireFileAsText(filename.c_str(), &size);
            if (!includeSource)
            {
                ERROR("Error: Failed to find file %s included in shader %s\n", filename.c_str(), path.c_str());
            }
            line = std::string("#line 100000\n") + std::string(includeSource) + std::string("\n#line ") + std::to_string(lineCount);
        }
        result += line + std::string("\n");
    }
    return result;
}

void OutShaderSource(const char* shaderSource)
{
    auto at = shaderSource;
    char lineBuffer[16384];
    while(*at)
    {
        auto lineBegin = at;
        u32 lineCount = 0;
        while (*at != '\n' && *at != '\r' && *at != 0)
        {
            at++;
            lineCount++;
        }
        if (lineCount)
        {
            assert(array_count(lineBuffer) >= lineCount + 2, "A shader source line is too big");
            memcpy(lineBuffer, lineBegin, lineCount);
            lineBuffer[lineCount] ='\\';
            lineBuffer[lineCount + 1] ='n';
            lineBuffer[lineCount + 2] = 0;
            L("\"%s\"", lineBuffer);
        }
        at++;
    }
}

int main(int argCount, char** args)
{
    assert(argCount > 1);
    const char* configFileName = args[1];

    // TODO: Test for name collisions
    std::vector<Program> programs = ParseConfigFile(configFileName);

    for (auto& prog : programs)
    {
        u32 size;
        char* _vert = ReadEntireFileAsText(prog.vert.c_str(), &size);
        char* _frag = ReadEntireFileAsText(prog.frag.c_str(), &size);
        if (!_vert)
        {
            ERROR("Error: Failed to read shader file %s\n", prog.vert.c_str());
        }
        if (!_frag)
        {
            ERROR("Error: Failed to read shader file %s\n", prog.frag.c_str());
        }
        std::string vert = PreprocessShader(prog.vert, std::string(_vert));
        std::string frag = PreprocessShader(prog.frag, std::string(_frag));
        prog.vertSource = std::move(vert);
        prog.fragSource = std::move(frag);
    }
    // TODO: Deal with CRLF and LF stuff!
    OutFile = fopen("shader_preprocessor_output.h", "wb");
    assert(OutFile);

    IDENT_RESET();
    L("// This file was generated by a glsl preprocessor\n");
    L("#pragma once");
    L("struct Shaders");
    L("{");
    IDENT_PUSH();
    for (auto& it : programs)
    {
        L("GLuint %s;", it.name.c_str());
    }
    IDENT_POP();
    L("};\n");

    L("const char* ShaderNames[] =");
    L("{");
    IDENT_PUSH();
    for (auto& it : programs)
    {
        L("\"%s\",", it.name.c_str());
    }
    IDENT_POP();
    L("};\n");

    L("const ShaderProgramSource ShaderSources[] =");
    L("{");
    IDENT_PUSH();
    for (auto& it : programs)
    {
        L("{");
        IDENT_PUSH();
        OutShaderSource(it.vertSource.c_str());
        A(",\n");
        OutShaderSource(it.fragSource.c_str());
        IDENT_POP();
        L("},");
    }
    IDENT_POP();
    L("};");
    fclose(OutFile);
}
