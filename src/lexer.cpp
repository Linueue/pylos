#include "lexer.hpp"

#include <unordered_map>

enum class LexerState {
    Start, Finish, Error, Identifier, IdentifierUnderscore, Integer, IPv4a1, IPv4a2, IPv4a3, SlashCIDR, CIDR, Assign, Plus, Minus, Multiply, Divide, LeftCurly, RightCurly, LeftParen, RightParen, Comma, Dot, Colon,
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

static TokenType mapToKeyword(const std::string_view lexeme) {
    static const std::unordered_map<std::string_view, TokenType> map = {
        { "val"    , TokenType::Val },
        { "network", TokenType::Network },
        { "switch" , TokenType::Switch },
        { "if"     , TokenType::If },
        { "else"   , TokenType::Else },
        { "for"    , TokenType::For },
    };

    auto m = map.find(lexeme);

    if(m != map.end())
        return m->second;

    return TokenType::Identifier;
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
    if(m_Current >= m_Source.size())
        return makeToken("", TokenType::EndOfFile);

    while(m_Current < m_Source.size() && (m_Source[m_Current] == ' ' || m_Source[m_Current] == '\n'))
        advance();

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
                    case '=':
                        transitionTo(LexerState::Assign);
                        break;
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
                    case '{':
                        transitionTo(LexerState::LeftCurly);
                        break;
                    case '}':
                        transitionTo(LexerState::RightCurly);
                        break;
                    case '(':
                        transitionTo(LexerState::LeftParen);
                        break;
                    case ')':
                        transitionTo(LexerState::RightParen);
                        break;
                    case ',':
                        transitionTo(LexerState::Comma);
                        break;
                    case '.':
                        transitionTo(LexerState::Dot);
                        break;
                    case ':':
                        transitionTo(LexerState::Colon);
                        break;
                    default:
                        advance();
                        return raiseError("Invalid token");
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
                tokType = mapToKeyword(std::string_view(m_Source).substr(m_Start, m_Current - m_Start));
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
                return raiseError("Invalid identifier, identifier must not start, nor end with _.");
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
            case LexerState::Assign:
                tokType = TokenType::Assign;
                transitionTo(LexerState::Finish);
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
            case LexerState::LeftCurly:
                tokType = TokenType::LeftCurly;
                transitionTo(LexerState::Finish);
                break;
            case LexerState::RightCurly:
                tokType = TokenType::RightCurly;
                transitionTo(LexerState::Finish);
                break;
            case LexerState::LeftParen:
                tokType = TokenType::LeftParen;
                transitionTo(LexerState::Finish);
                break;
            case LexerState::RightParen:
                tokType = TokenType::RightParen;
                transitionTo(LexerState::Finish);
                break;
            case LexerState::Comma:
                tokType = TokenType::Comma;
                transitionTo(LexerState::Finish);
                break;
            case LexerState::Dot:
                tokType = TokenType::Dot;
                transitionTo(LexerState::Finish);
                break;
            case LexerState::Colon:
                tokType = TokenType::Colon;
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
