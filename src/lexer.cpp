#include "lexer.hpp"
#include "token.hpp"

#include <unordered_map>
#include <unordered_set>

enum class LexerState {
    Start, Finish, Identifier, IdentifierUnderscore, Integer, IPv4a1, IPv4a2, IPv4a3, SlashCIDR, CIDR, Assign, Plus, Minus, Multiply, Divide, LeftCurly, RightCurly, LeftBracket, RightBracket, LeftParen, RightParen, Comma, Dot, Colon, String, EscapeString, Comment, MultiLineComment, MultiLineEndComment, LogicalNot, LogicalAnd, LogicalOr, RelationalEqual, RelationalNotEqual, RelationalGreaterThan, RelationalLessThan,
};

Lexer::Lexer(const std::string& source)
    : m_Source(source) {
    m_State = LexerState::Start;
}

Lexer::~Lexer() {
}

Expected<Token, LexerDiagnostic> Lexer::Next() {
    while(true) {
        auto tok = scanToken();

        if(!tok)
            return tok;

        if((*tok) == TokenType::Comment)
            continue;

        return tok;
    }
}

Expected<Token, LexerDiagnostic> Lexer::Peek() {
    // Saved
    size_t start = m_Start;
    size_t current = m_Current;
    size_t column = m_Column;
    size_t line = m_Line;

    auto tok = Next();

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
        { "value"  , TokenType::Val },
        { "network", TokenType::Network },
        { "switch" , TokenType::Switch },
        { "port"   , TokenType::Port },
        { "if"     , TokenType::If },
        { "else"   , TokenType::Else },
        { "for"    , TokenType::For },
        { "true"   , TokenType::BooleanLiteral },
        { "false"  , TokenType::BooleanLiteral },
    };

    auto m = map.find(lexeme);

    if(m != map.end())
        return m->second;

    return TokenType::Identifier;
}

static TokenType mapToValueTypeHint(const std::string_view lexeme) {
    static const std::unordered_set<std::string_view> map = {
        "String",
        "Integer",
        "Int",
        "Boolean",
        "Bool",
        "IPAddress",
        "Subnet",
        "Device",
        "VLAN",
    };

    auto m = map.find(lexeme);

    if(m != map.end())
        return TokenType::ValueTypeHint;

    return TokenType::Identifier;
}

Unexpected<LexerDiagnostic> Lexer::raiseError(std::string reason) {
    return LexerDiagnostic {
        .reason = reason,
        .line = m_Line,
        .column = m_Column,
    };
}

Token Lexer::makeToken(const std::string_view lexeme, const size_t line, const size_t column, TokenType tok) {
    return Token {
        .type = tok,
        .lexeme = lexeme,
        .line = line,
        .column = column,
    };
}

static bool isWhitespace(const char c) {
    return (c == ' ' || c == '\n' || c == '\t' || c == '\r');
}

Expected<Token, LexerDiagnostic> Lexer::scanToken() {
    while(m_Current < m_Source.size() && isWhitespace(m_Source[m_Current]))
        advance();

    if(m_Current >= m_Source.size())
        return makeToken("", m_Line, m_Column, TokenType::EndOfFile);

    bool isFinished = false;
    TokenType tokType;
    m_State = LexerState::Start;
    const size_t line = m_Line;
    const size_t column = m_Column;

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
                    case '!':
                        transitionTo(LexerState::LogicalNot);
                        break;
                    case '&':
                        transitionTo(LexerState::LogicalAnd);
                        break;
                    case '>':
                        transitionTo(LexerState::RelationalGreaterThan);
                        break;
                    case '<':
                        transitionTo(LexerState::RelationalLessThan);
                        break;
                    case '|':
                        transitionTo(LexerState::LogicalOr);
                        break;
                    case '{':
                        transitionTo(LexerState::LeftCurly);
                        break;
                    case '}':
                        transitionTo(LexerState::RightCurly);
                        break;
                    case '[':
                        transitionTo(LexerState::LeftBracket);
                        break;
                    case ']':
                        transitionTo(LexerState::RightBracket);
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
                    case '\"':
                        transitionTo(LexerState::String);
                        break;
                    default:
                        advance();
                        return raiseError("Invalid token");
                }
                advance();
                break;
            case LexerState::Identifier: {
                if(isAlphaNum(current)) {
                    advance();
                    break;
                }
                if(current == '_') {
                    transitionTo(LexerState::IdentifierUnderscore);
                    advance();
                    break;
                }
                std::string_view lexeme = std::string_view(m_Source).substr(m_Start, m_Current - m_Start);
                tokType = mapToKeyword(lexeme);
                if(tokType == TokenType::Identifier)
                    tokType = mapToValueTypeHint(lexeme);
                transitionTo(LexerState::Finish);
                break;
            }
            case LexerState::IdentifierUnderscore:
                if(isAlphaNum(current)) {
                    transitionTo(LexerState::Identifier);
                    advance();
                    break;
                }
                if(current == '_') {
                    advance();
                    break;
                
                return raiseError("Invalid identifier, identifier must not start, nor end with _");
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
                    return raiseError("Expected digit, or '.'");
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
                    return raiseError("Expected digit, or '.'");
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

                if(current == '.')
                    return raiseError("Invalid IPv4 address");

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
            case LexerState::String:
                if(current == '\"') {
                    advance();
                    tokType = TokenType::StringLiteral;
                    transitionTo(LexerState::Finish);
                    break;
                }
                if(current == '\n')
                    return raiseError("Unterminated string literal");
                if(current == '\\')
                    transitionTo(LexerState::EscapeString);
                advance();
                break;
            case LexerState::EscapeString:
                switch(current) {
                    case 'n':
                    case 't':
                    case 'r':
                    case '\\':
                    case '\'':
                    case '\"':
                    case '0':
                        advance();
                        transitionTo(LexerState::String);
                        break;
                    default:
                        return raiseError("Invalid escape sequence");
                }
                break;
            case LexerState::Assign:
                if(current == '=')
                {
                    transitionTo(LexerState::RelationalEqual);
                    advance();
                    break;
                }

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
                switch(current) {
                    case '/':
                        advance();
                        transitionTo(LexerState::Comment);
                        break;
                    case '*':
                        advance();
                        transitionTo(LexerState::MultiLineComment);
                        break;
                    default:
                        tokType = TokenType::Divide;
                        transitionTo(LexerState::Finish);
                        break;
                }
                break;
            case LexerState::LeftCurly:
                tokType = TokenType::LeftCurly;
                transitionTo(LexerState::Finish);
                break;
            case LexerState::RightCurly:
                tokType = TokenType::RightCurly;
                transitionTo(LexerState::Finish);
                break;
            case LexerState::LeftBracket:
                tokType = TokenType::LeftBracket;
                transitionTo(LexerState::Finish);
                break;
            case LexerState::RightBracket:
                tokType = TokenType::RightBracket;
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
            case LexerState::Comment:
                tokType = TokenType::Comment;
                if(current == '\n')
                    transitionTo(LexerState::Finish);
                advance();
                break;
            case LexerState::MultiLineComment:
                tokType = TokenType::Comment;
                if(current == '*')
                    transitionTo(LexerState::MultiLineEndComment);
                else if(current == '\0')
                    return raiseError("Unterminated multiline comment");
                advance();
                break;
            case LexerState::MultiLineEndComment:
                tokType = TokenType::Comment;
                if(current == '/')
                    transitionTo(LexerState::Finish);
                else
                    transitionTo(LexerState::MultiLineComment);
                advance();
                break;
            case LexerState::LogicalNot:
                if(current == '=')
                {
                    transitionTo(LexerState::RelationalNotEqual);
                    advance();
                    break;
                }

                tokType = TokenType::Logical;
                transitionTo(LexerState::Finish);
                break;
            case LexerState::LogicalAnd:
                tokType = TokenType::Logical;

                if(current != '&')
                    return raiseError("Expected &&");

                transitionTo(LexerState::Finish);
                advance();
                break;
            case LexerState::LogicalOr:
                tokType = TokenType::Logical;

                if(current != '|')
                    return raiseError("Expected ||");

                transitionTo(LexerState::Finish);
                advance();
                break;
            case LexerState::RelationalGreaterThan:
                tokType = TokenType::Relational;

                // Consume greater than equal
                if(current == '=')
                    advance();

                transitionTo(LexerState::Finish);
                break;
            case LexerState::RelationalLessThan:
                tokType = TokenType::Relational;

                // Consume less than equal
                if(current == '=')
                    advance();

                transitionTo(LexerState::Finish);
                break;
            case LexerState::RelationalEqual:
            case LexerState::RelationalNotEqual:
                tokType = TokenType::Relational;
                transitionTo(LexerState::Finish);
                break;
            case LexerState::Finish:
                isFinished = true;
                break;
        }
    }

    std::string_view lexeme = std::string_view(m_Source).substr(m_Start, m_Current - m_Start);

    return makeToken(lexeme, line, column, tokType);
}
