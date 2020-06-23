#include "../Common.h"
//#include "utils.cpp"
#include "string.h"

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


bool IsSpace(char c) {
    bool result;
    result = (c == ' '  || c == '\n' || c == '\r' || c == '\t' || c == '\v' || c == '\f');
    return result;
}

bool IsDelimeter(char c) {
    bool result;
    result = (IsSpace(c) || c == ':' || c == '/' || c == '{' || c == '}' || c == '[' || c == ']' || c == 0);
    return result;
}


struct Lexer {
    const char* source;
    u32 at;
    const char* atPtr;
    u32 line;
    u32 column;
    u32 anchor;
    const char* anchorPtr;
    u32 anchorLine;
    u32 anchorColumn;
    bool end;
    bool anchorEnd;
};

void InitLexer(Lexer* lexer, const char* source) {
    lexer->source = source;
    lexer->atPtr = source;
};

bool LexerAdvance(Lexer* lexer) {
    bool advanced = false;
    if (!lexer->end) {
        assert(lexer->source[lexer->at] != 0);
        auto current = lexer->source[lexer->at];
        if (current == '\n') {
            lexer->line++;
            lexer->column = 0;
        } else {
            lexer->column++;
        }
        lexer->at++;
        lexer->atPtr++;
        if (lexer->source[lexer->at] == 0) lexer->end = true;
        advanced = true;
    }
    return advanced;
}

char LexerGetCurrChar(Lexer* lexer) {
    char result = lexer->source[lexer->at];
    return result;
}

char LexerGetPrevChar(Lexer* lexer) {
    char result = 0;
    if (lexer->at) {
        result = lexer->source[lexer->at];
    }
    return result;
}

char LexerGetNextChar(Lexer* lexer){
    char result = 0;
    if (!lexer->end) {
        result = lexer->source[lexer->at];
    }
    return result;
}

void LexerEatSpace(Lexer* lexer) {
    while (true) {
        if (lexer->end) break;
        auto curr = LexerGetCurrChar(lexer);
        if (!IsSpace(curr)) {
            break;
        }
        LexerAdvance(lexer);
    }
}

void LexerSetAnchor(Lexer* lexer) {
    lexer->anchor = lexer->at;
    lexer->anchorPtr = lexer->atPtr;
    lexer->anchorLine = lexer->line;
    lexer->anchorColumn = lexer->column;
    lexer->anchorEnd = lexer->end;
}

void LexerResetToAnchor(Lexer* lexer) {
    lexer->at = lexer->anchor;
    lexer->atPtr = lexer->anchorPtr;
    lexer->line = lexer->anchorLine;
    lexer->column = lexer->anchorColumn;
    lexer->end = lexer->anchorEnd;
}

char* LexerExtract(Lexer* lexer) {
    char* result = nullptr;
    if (lexer->at > lexer->anchor) {
        auto size = lexer->at - lexer->anchor + 1;
        result = (char*)malloc(size);
        memcpy(result, lexer->source + lexer->anchor, size - 1);
        result[size - 1] = 0;
    }
    return result;
}

enum struct LexemClass {
    Error = 0, Operator, Identifier
};

struct Token {
    bool matches;
    LexemClass lexemClass;
    u32 line;
    u32 column;
    char* value;
};

struct MatchResult {
    bool matches;
    Token token;
};

MatchResult LexerMatchOperator(Lexer* lexer, const char* op) {
    bool matches = true;
    while (!lexer->end && *op) {
        auto curr = LexerGetCurrChar(lexer);
        if (*op != curr) {
            matches = false;
            break;
        }
        LexerAdvance(lexer);
        op++;
    }
    MatchResult result {};
    if (matches) {
        result.matches = matches;
        result.token.lexemClass = LexemClass::Operator;
        result.token.value = LexerExtract(lexer);
        result.token.line = lexer->anchorLine;
        result.token.column = lexer->anchorColumn;
        assert(result.token.value);
    }
    return result;
}

MatchResult LexerMatchIdentifier(Lexer* lexer) {
    bool matches = false;
    if (isDelimeter(LexerGetPrevChar(lexer))) {
        matches = true;
        while (!lexer->end) {
            auto curr = LexerGetCurrChar(lexer);
            if (!IsLe)
            if (IsDelimeter(curr)) break;
            if (*op != curr) {
                matches = false;
                break;
            }
            LexerAdvance(lexer);
            op++;
        }
    }
    if (matches && !IsDelimeter(LexerGetCurrChar(lexer))) matches = false;
    MatchResult result {};
    if (matches) {
        result.matches = matches;
        result.token.lexemClass = LexemClass::Keyword;
        result.token.value = LexerExtract(lexer);
        result.token.line = lexer->anchorLine;
        result.token.column = lexer->anchorColumn;
        assert(result.token.value);
    }
    return result;
}

Token LexerConsumeError(Lexer* lexer) {
    while (true) {
        if (lexer->end) break;
        auto curr = LexerGetCurrChar(lexer);
        if (IsDelimeter(curr)) {
            break;
        }
        LexerAdvance(lexer);
    }
    Token result {};
    result.matches = true;
    result.lexemClass = LexemClass::Error;
    result.line = lexer->anchorLine;
    result.column = lexer->anchorColumn;
    result.value = LexerExtract(lexer);
    return result;
}

void LexerTokenize(Lexer* lexer) {
    while (true) {
        LexerEatSpace(lexer);
        if (lexer->end) break;
        LexerSetAnchor(lexer);

        bool found = false;

        {
            auto result = LexerMatchOperator(lexer, ":/");
            if (result.matches) {
                printf("%lu:%lu: Lexem operator: %s\n", result.token.line, result.token.column, result.token.value);
                found = true;
            }
        }

        {
            auto result = LexerMatchIdentifier(lexer);
            if (result.matches) {
                printf("%lu:%lu: Lexem identifier: %s\n", result.token.line, result.token.column, result.token.value);
                found = true;
            }
        }

        if (!found) {
            auto result = LexerConsumeError(lexer);
            printf("%lu:%lu: Error: %s\n", result.line, result.column, result.value);
        }
    }
}

const char* test = R"(
:/PlayerConfig
height 1.8
p {0, 15, 0}
acceleration 70.0
friction 10.0
jumpAcceleration 420.0
runAcceleration 140.0

:/RenderSettings
shadowMapResolution 2048
stableShadows true
)";

int main(int argCount, char** args) {
    Lexer lexer {};
    InitLexer(&lexer, test);
    LexerTokenize(&lexer);
}
