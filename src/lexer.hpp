#pragma once

#include <string>

#include "token.hpp"
#include "utils/expected.hpp"

enum class LexerState;

struct LexerDiagnostic {
    std::string reason;
    size_t line;
    size_t column;
};

class Lexer {
public:
    Lexer(const std::string& source);
    ~Lexer();

    Expected<Token, LexerDiagnostic> Next();
    Expected<Token, LexerDiagnostic> Peek();

private:
    Expected<Token, LexerDiagnostic> scanToken();
    Unexpected<LexerDiagnostic> raiseError(std::string reason);
    Token makeToken(const std::string_view lexeme, TokenType tok);
    void transitionTo(LexerState state);
    void advance();

private:
    const std::string m_Source;
    LexerState m_State;
    size_t m_Start   = 0;
    size_t m_Current = 0;
    size_t m_Line    = 1;
    size_t m_Column  = 1;
};
