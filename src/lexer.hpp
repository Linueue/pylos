#pragma once

#include <string>

#include "token.hpp"

enum class LexerState;

class Lexer {
public:
    Lexer(const std::string& source);
    ~Lexer();

    Token Next();
    Token Peek();

private:
    Token scanToken();
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
