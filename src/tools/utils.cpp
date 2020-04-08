#include <stdlib.h>
#include <string>
#include <windows.h>

bool IsSpace(char c) {
    bool result;
    result = (c == ' '  ||
              c == '\n' ||
              c == '\r' ||
              c == '\t' ||
              c == '\v' ||
              c == '\f');
    return result;
}

bool Match(const char* str1, const char* str2) {
    bool result = true;
    while (*str2) {
        if (*str1 == 0) {
            result = false;
            break;
        } if (*str1 != *str2) {
            result = false;
            break;
        }
        str1++;
        str2++;
    }
    return result;
}

bool Match(const char* str, char c) {
    bool result = (*str == c);
    return result;
}

char* EatLine(char* at);
char* EatMultiLineComment(char* at);

char* EatSpace(char* at) {
    char* result = 0;
    while (*at) {
        if (Match(at, "//")) {
            at = EatLine(at);
        } else if (Match(at, "/*")) {
            at = EatMultiLineComment(at);
        } else if (IsSpace(*at)) {
            at++;
        } else {
            result = at;
            return result;
        }
    }
    result = at;
    return result;
}

char* EatLine(char* at) {
    char* result = 0;
    while (*at) {
        // NOTE: Supporting only LF and CRLF
        if (*at == '\n') {
            result = EatSpace(at);
            return result;
        }
        at++;
    }
    result = at;
    return result;
}

char* EatMultiLineComment(char* at) {
    char* result = 0;
    u32 opened = 0;
    u32 closed = 0;

    while (*at) {
        if (*at == '/' && *(at + 1) == '*') {
            opened++;
        } else if (*at == '*' && *(at + 1) == '/') {
            closed++;
        } if (opened == closed) {
            result = at + 2;
            return result;
        }
        at++;
    }

    result = at;
    printf("Error: Unbalanced multiline comment found\n");

    return result;
}

std::string GetDirectory(const std::string& path) {
#if defined(PLATFORM_WINDOWS)
    const char* separators = "\\/";
#elif defined(PLATFORM_LINUX)
    const char* separators = "/";
#endif

    auto index = path.find_last_of(separators);
    if (index == std::string::npos) {
        return std::string("");
    } else {
        return path.substr(0, index + 1);
    }
}


void* ReadEntireFile(const char* filename, u32* read) {
    u32 bytesRead = 0;
    void* bitmap = nullptr;
    LARGE_INTEGER fileSize = {};
    HANDLE fileHandle = CreateFile(filename, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, 0);
    if (fileHandle != INVALID_HANDLE_VALUE) {
        if (GetFileSizeEx(fileHandle, &fileSize)) {
            if (fileSize.QuadPart > 0xffffffff) {
                printf("Can`t read >4GB file.");
                CloseHandle(fileHandle);
                *read = 0;
                return nullptr;
            }
            bitmap = malloc(fileSize.QuadPart);
            if (bitmap) {
                DWORD read;
                if (!ReadFile(fileHandle, bitmap, (DWORD)fileSize.QuadPart, &read, 0) && !(read == (DWORD)fileSize.QuadPart)) {
                    printf("Failed to read file.");
                    free(bitmap);
                    bitmap = nullptr;
                } else {
                    bytesRead = (u32)fileSize.QuadPart;
                }
            }
        }
        CloseHandle(fileHandle);
    }
    *read = bytesRead;
    return bitmap;
}

char* ReadEntireFileAsText(const char* filename, u32* bytesRead) {
    char* result = 0;
    *bytesRead = 0;
    LARGE_INTEGER fileSize = {};
    HANDLE fileHandle = CreateFile(filename, GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING, 0, 0);

    if (fileHandle != INVALID_HANDLE_VALUE) {
        if (GetFileSizeEx(fileHandle, &fileSize)) {
            if (fileSize.QuadPart < 0xffffffff) {
                void* data = malloc(fileSize.QuadPart + 1);
                if (data) {
                    DWORD read;
                    if (ReadFile(fileHandle, data, (DWORD)fileSize.QuadPart, &read, 0) && (read == (DWORD)fileSize.QuadPart)) {
                        result = (char*)data;
                        ((byte*)data)[fileSize.QuadPart] = 0;
                        *bytesRead = (u32)fileSize.QuadPart + 1;
                    } else {
                        free(data);
                    }
                }
            }
        }
        CloseHandle(fileHandle);
    }
    return  result;
}
