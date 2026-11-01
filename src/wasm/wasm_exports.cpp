#include "lexer.hpp"

struct LexerResult {
    const char* type;
    uint32_t line;
    uint32_t column;
    const char* lexeme_ptr;
    uint32_t lexeme_len;
};

extern "C" {
    void* lexer_create(const char* src, size_t len) {
        auto lexer = new Lexer(std::string(src, len));
        return (void*)lexer;
    }

    int32_t lexer_next(void* handle, LexerResult* outResult, char* errorBuf, const size_t errorMaxSize) {
        Lexer* lexer = (Lexer*)handle;
        auto tok = lexer->Next();
        
        if(tok.IsError()) {
            LexerDiagnostic diagnostic = tok.Error();

            outResult->type = 0;
            outResult->line = diagnostic.line;
            outResult->column = diagnostic.column;
            outResult->lexeme_ptr = nullptr;
            outResult->lexeme_len = 0;

            if(diagnostic.reason.size() > errorMaxSize)
                return -1;

            std::memcpy(errorBuf, diagnostic.reason.data(), diagnostic.reason.size());
            errorBuf[diagnostic.reason.size()] = '\0';
            return -1;
        }

        outResult->type = (*tok).GetTokenString().data();
        outResult->line = (*tok).line;
        outResult->column = (*tok).column;
        outResult->lexeme_ptr = (*tok).lexeme.data();
        outResult->lexeme_len = (*tok).lexeme.size();

        return 0;
    }

    int32_t lexer_destroy(void* handle) {
        Lexer* lexer = (Lexer*)handle;
        delete lexer;
        return 0;
    }
}
