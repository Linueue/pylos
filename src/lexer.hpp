#pragma once

#include <string>

#include "token.hpp"
#include "utils/expected.hpp"

enum class LexerState;

// Lexer diagnostic information
struct LexerDiagnostic {
    std::string reason;
    size_t line;
    size_t column;
};

// The Lexer that takes in a source of std::string
// An on demand lexer that tokenizes each token each Next() function
// at a time
class Lexer {
public:
    Lexer(const std::string& source);

    // Returns the next token, and consumes it
    // 
    // @return
    //  Token on success
    //  LexerDiagnostic on error such as an invalid token
    Expected<Token, LexerDiagnostic> Next();
    // Returns the next token without consuming it
    // This is used for lookahead
    //
    // @return
    //  Token on success
    //  LexerDiagnostic on error such as an invalid token
    Expected<Token, LexerDiagnostic> Peek();

private:
    // Internal scanToken that is used to actually tokenizes the source
    Expected<Token, LexerDiagnostic> scanToken();
    Unexpected<LexerDiagnostic> raiseError(std::string reason);
    // Creates the token
    Token makeToken(const std::string_view lexeme, const size_t line, const size_t column, TokenType tok);
    // Equivalent to setting m_State to the argument state
    // A setter which helps with readability
    void transitionTo(LexerState state);
    // Advances the current tokenization index as well as the line, and column
    void advance();

private:
    const std::string m_Source;
    LexerState m_State;
    size_t m_Start   = 0;
    size_t m_Current = 0;
    size_t m_Line    = 1;
    size_t m_Column  = 1;
};
