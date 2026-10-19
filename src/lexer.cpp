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

Token Lexer::Next() {
    return scanToken();
}

Token Lexer::Peek() {
    // Saved
    size_t start = m_Start;
    size_t current = m_Current;
    size_t column = m_Column;
    size_t line = m_Line;

    const Token tok = scanToken();

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
    m_Current++;
    m_Column++;

    if(m_Source[m_Current] == '\n') {
        // \n should not count as column = 1
        m_Column = 0;
        m_Line++;
    }
}

static bool isAlphaNum(const char c) {
    return (std::isalpha(c) || std::isdigit(c));
}

Token Lexer::scanToken() {
    while(m_Source[m_Current] == ' ' && m_Current < m_Source.size())
        m_Current++;

    bool isFinished = false;
    bool isInvalid = false;
    TokenType tokType;
    m_State = LexerState::Start;

    m_Start = m_Current;

    while(!isFinished) {
        const char current = m_Source[m_Current];

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
                        advance();
                        break;
                    case '-':
                        transitionTo(LexerState::Minus);
                        advance();
                        break;
                    case '*':
                        transitionTo(LexerState::Multiply);
                        advance();
                        break;
                    case '/':
                        transitionTo(LexerState::Divide);
                        advance();
                        break;
                    case '\n':
                        transitionTo(LexerState::Error);
                        advance();
                        break;
                }
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
                transitionTo(LexerState::Error);
                advance();
                break;
            case LexerState::Integer:
                if(current == '.') {
                    transitionTo(LexerState::IPv4a1);
                    advance();
                    break;
                }
                if(!std::isdigit(current)) {
                    transitionTo(LexerState::Finish);
                    advance();
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
                    transitionTo(LexerState::Error);
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
                    transitionTo(LexerState::Error);
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
                transitionTo(LexerState::Error);
                break;
            case LexerState::CIDR:
                if(!std::isdigit(current))
                {
                    transitionTo(LexerState::Finish);
                    break;
                }
                advance();
                break;
            case LexerState::Plus:
                break;
            case LexerState::Minus:
                break;
            case LexerState::Multiply:
                m_State = LexerState::Finish;
                transitionTo(LexerState::Finish);
                break;
            case LexerState::Divide:
                transitionTo(LexerState::Finish);
                break;
            case LexerState::Error:
                isInvalid = true;
            case LexerState::Finish:
                isFinished = true;
                break;
        }
    }

    return Token();
}
