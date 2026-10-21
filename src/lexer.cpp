#include "lexer.hpp"

enum class LexerState {
    Start, Finish, Error, Identifier, IdentifierUnderscore, Integer, IPv4a1, IPv4a2, IPv4a3, SlashCIDR, CIDR, Plus, Minus, Multiply, Divide,
};

Lexer::Lexer(const std::string& source)
    : m_Source(source) {
    m_State = LexerState::Start;
}

Lexer::~Lexer() {
}

Expected<Token, LexerDiagnostic> Lexer::Next() {
    return scanToken();
}

Expected<Token, LexerDiagnostic> Lexer::Peek() {
    // Saved
    size_t start = m_Start;
    size_t current = m_Current;
    size_t column = m_Column;
    size_t line = m_Line;

    auto tok = scanToken();

    m_Start = start;
    m_Current = current;
    m_Column = column;
    m_Line = line;
    return tok;
}

void Lexer::transitionTo(LexerState state) {
    m_State = state;
}

void Lexer::advance() {
    if(m_Current >= m_Source.size())
        return;

    m_Column++;

    if(m_Source[m_Current++] == '\n') {
        // \n should not count as column = 1
        m_Column = 0;
        m_Line++;
    }
}

static bool isAlphaNum(const char c) {
    return (std::isalpha(c) || std::isdigit(c));
}

Unexpected<LexerDiagnostic> Lexer::raiseError(std::string reason) {
    return LexerDiagnostic {
        .reason = reason,
        .line = m_Line,
        .column = m_Column,
    };
}

Token Lexer::makeToken(const std::string_view lexeme, TokenType tok) {
    return Token {
        .type = tok,
        .lexeme = lexeme,
        .line = m_Line,
        .column = m_Column,
    };
}

Expected<Token, LexerDiagnostic> Lexer::scanToken() {
    while(m_Current < m_Source.size() && m_Source[m_Current] == ' ')
        m_Current++;

    bool isFinished = false;
    bool isInvalid = false;
    TokenType tokType;
    m_State = LexerState::Start;

    m_Start = m_Current;

    while(!isFinished) {
        const char current = (m_Current < m_Source.size()) ? m_Source[m_Current] : '\0';

        switch(m_State) {
            case LexerState::Start:
                if(std::isalpha(current)) {
                    transitionTo(LexerState::Identifier);
                    advance();
                    break;
                }
                if(std::isdigit(current)) {
                    transitionTo(LexerState::Integer);
                    advance();
                    break;
                }
                switch(current) {
                    case '+':
                        transitionTo(LexerState::Plus);
                        break;
                    case '-':
                        transitionTo(LexerState::Minus);
                        break;
                    case '*':
                        transitionTo(LexerState::Multiply);
                        break;
                    case '/':
                        transitionTo(LexerState::Divide);
                        break;
                    case '\n':
                        transitionTo(LexerState::Error);
                        break;
                    default:
                        advance();
                        return raiseError("Invalid token.");
                }
                advance();
                break;
            case LexerState::Identifier:
                if(isAlphaNum(current)) {
                    advance();
                    break;
                }
                if(current == '_') {
                    transitionTo(LexerState::IdentifierUnderscore);
                    advance();
                    break;
                }
                tokType = TokenType::Identifier;
                transitionTo(LexerState::Finish);
                break;
            case LexerState::IdentifierUnderscore:
                if(isAlphaNum(current)) {
                    transitionTo(LexerState::Identifier);
                    advance();
                    break;
                }
                if(current == '_') {
                    advance();
                    break;
                }
                advance();
                return raiseError("Unexpected character.");
            case LexerState::Integer:
                if(current == '.') {
                    transitionTo(LexerState::IPv4a1);
                    advance();
                    break;
                }
                if(!std::isdigit(current)) {
                    tokType = TokenType::Integer;
                    transitionTo(LexerState::Finish);
                    break;
                }
                advance();
                break;
            // 192.[162].0.1
            case LexerState::IPv4a1:
                if(current == '.') {
                    transitionTo(LexerState::IPv4a2);
                    advance();
                    break;
                }
                if(!std::isdigit(current)) {
                    advance();
                    return raiseError("Expected digit, or '.'.");
                }
                advance();
                break;
            // 192.162.[0].1
            case LexerState::IPv4a2:
                if(current == '.') {
                    transitionTo(LexerState::IPv4a3);
                    advance();
                    break;
                }
                if(!std::isdigit(current)) {
                    advance();
                    return raiseError("Expected digit, or '.'.");
                }
                advance();
                break;
            // 192.162.0.[1]
            case LexerState::IPv4a3:
                if(current == '/') {
                    transitionTo(LexerState::SlashCIDR);
                    advance();
                    break;
                }
                if(!std::isdigit(current))
                {
                    tokType = TokenType::IPv4;
                    transitionTo(LexerState::Finish);
                    break;
                }
                advance();
                break;
            case LexerState::SlashCIDR:
                advance();

                if(std::isdigit(current))
                {
                    transitionTo(LexerState::CIDR);
                    break;
                }
                return raiseError("Expected CIDR.");
                break;
            case LexerState::CIDR:
                if(!std::isdigit(current))
                {
                    tokType = TokenType::IPv4CIDR;
                    transitionTo(LexerState::Finish);
                    break;
                }
                advance();
                break;
            case LexerState::Plus:
                tokType = TokenType::Plus;
                transitionTo(LexerState::Finish);
                break;
            case LexerState::Minus:
                tokType = TokenType::Minus;
                transitionTo(LexerState::Finish);
                break;
            case LexerState::Multiply:
                tokType = TokenType::Multiply;
                transitionTo(LexerState::Finish);
                break;
            case LexerState::Divide:
                tokType = TokenType::Divide;
                transitionTo(LexerState::Finish);
                break;
            case LexerState::Error:
                isInvalid = true;
            case LexerState::Finish:
                isFinished = true;
                break;
        }
    }

    std::string_view lexeme = std::string_view(m_Source).substr(m_Start, m_Current - m_Start);

    return makeToken(lexeme, tokType);
}
