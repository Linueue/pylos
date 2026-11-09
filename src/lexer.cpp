#include "lexer.hpp"
#include "token.hpp"

#include <unordered_map>
#include <unordered_set>

// The internal lexer state
// Not to be confused with the TokenType
// As the LexerState have more states it's using for tokenization, such as String, or StringEscape
enum class LexerState {
    Start, Finish, Identifier, IdentifierUnderscore, Integer, IPv4a1, IPv4a1Digits, IPv4a2, IPv4a2Digits, IPv4a3, IPv4a3Digits, SlashCIDR, CIDR, Assign, Plus, Minus, Multiply, Divide, Modulo, LeftCurly, RightCurly, LeftBracket, RightBracket, LeftParen, RightParen, Comma, Dot, Range, Colon, String, EscapeString, Comment, MultiLineComment, MultiLineEndComment, LogicalNot, LogicalAnd, LogicalOr, RelationalEqual, RelationalNotEqual, RelationalGreaterThan, RelationalLessThan, EndOfStatement,
};

Lexer::Lexer(const std::string& source)
    : m_Source(source) {
}

Expected<Token, LexerDiagnostic> Lexer::Next() {
    // This omits Comment tokens as they are not necessary for parsing
    while(true) {
        auto tok = scanToken();

        // If token has error, return it immediately
        if(tok.IsError())
            return tok;

        // If the token is successful, test whether it's a comment
        // If it's a comment, ignore it
        if((*tok) == TokenType::Comment)
            continue;

        return tok;
    }
}

Expected<Token, LexerDiagnostic> Lexer::Peek() {
    // Saves the internal states
    // This ensures that the next token is not consumed
    // That is, when multiple consecutive calles to Peek()
    // will return the same token until Next() is explicitly called
    // This is done by saving the states, then simulating
    // the Next() call, then restoring the saved states
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
        m_Column = 1;
        m_Line++;
    }
}

static bool isAlphaNum(const char c) {
    return (std::isalpha(c) || std::isdigit(c));
}

// Maps the lexeme to a token keyword using a HashMap
static TokenType mapToKeyword(const std::string_view lexeme) {
    static const std::unordered_map<std::string_view, TokenType> map = {
        { "val"       , TokenType::Val },
        { "value"     , TokenType::Val },
        { "network"   , TokenType::Network },
        { "switch"    , TokenType::Switch },
        { "router"    , TokenType::Router },
        { "interface" , TokenType::Interface },
        { "port"      , TokenType::Port },
        { "if"        , TokenType::If },
        { "else"      , TokenType::Else },
        { "for"       , TokenType::For },
        { "while"     , TokenType::While },
        { "do"        , TokenType::Do },
        { "in"        , TokenType::In },
        { "Access"    , TokenType::Access },
        { "Trunk"     , TokenType::Trunk },
        { "vlan"      , TokenType::Vlan },
        { "true"      , TokenType::BooleanLiteral },
        { "false"     , TokenType::BooleanLiteral },
    };

    auto m = map.find(lexeme);

    // If m cannot find the lexeme in the HashMap
    // it will return map.end(), otherwise it will return the Pair
    // Which has the m->first, and m->second
    if(m != map.end())
        return m->second;

    // Otherwise, this is an identifier
    // While a DFA will work here, it will take more states than necessary
    return TokenType::Identifier;
}

// Maps to the ValueTypeHint token when found in the HashSet
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
        "Interface",
    };

    auto m = map.find(lexeme);

    if(m != map.end())
        return TokenType::ValueTypeHint;

    return TokenType::Identifier;
}

// Returns Unexpected<LexerDiagnostic>
// This is a helper function to make it easier to create the struct
Unexpected<LexerDiagnostic> Lexer::raiseError(std::string reason) {
    return LexerDiagnostic {
        .reason = reason,
        .line = m_Line,
        .column = m_Column,
    };
}

// A helper function to make it easier to construct the Token struct
Token Lexer::makeToken(const std::string_view lexeme, const size_t line, const size_t column, TokenType tok) {
    return Token {
        .type = tok,
        .lexeme = lexeme,
        .line = line,
        .column = column,
    };
}

// Checks whether c is a whitespace such as spaces, or new line character
static bool isWhitespace(const char c) {
    return (c == ' ' || c == '\t' || c == '\n' || c == '\r');
}

// Does the actual tokenization
Expected<Token, LexerDiagnostic> Lexer::scanToken() {
    // If it is a whitespace, advance immediately
    // We only tokenize non whitespace characters
    while(m_Current < m_Source.size() && isWhitespace(m_Source[m_Current]))
        advance();

    // If the current index position grows outside the source size
    // return the TokenType::EndOfFile token
    if(m_Current >= m_Source.size())
        return makeToken("", m_Line, m_Column, TokenType::EndOfFile);
    
    // Preparation for the actual tokenization
    bool isFinished = false;
    TokenType tokType;
    m_State = LexerState::Start;
    const size_t line = m_Line;
    const size_t column = m_Column;

    m_Start = m_Current;

    while(!isFinished) {
        // Prevent string overflow
        const char current = (m_Current < m_Source.size()) ?
            m_Source[m_Current] : '\0';

        switch(m_State) {
            case LexerState::Start:
                // If it starts with an alphabet, assume that it's an identifier
                if(std::isalpha(current)) {
                    transitionTo(LexerState::Identifier);
                    advance();
                    break;
                }
                // If it starts with a digit, assume that it's an integer
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
                    case '%':
                        transitionTo(LexerState::Modulo);
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
                    case ';':
                        transitionTo(LexerState::EndOfStatement);
                        break;
                    default:
                        advance();
                        return raiseError("Invalid token");
                }
                advance();
                break;
            // Identifier rules:
            //  - It must not start with a digit
            //  - Underscore is allowed, but it must not start nor end with it
            //  - Alphabet, digit, and underscore is only allowed
            case LexerState::Identifier: {
                if(isAlphaNum(current)) {
                    advance();
                    break;
                }
                // Transition to IdentifierUnderscore as it's not a valid final state
                if(current == '_') {
                    transitionTo(LexerState::IdentifierUnderscore);
                    advance();
                    break;
                }
                // Extract the lexeme
                std::string_view lexeme = std::string_view(m_Source).substr(m_Start, m_Current - m_Start);
                tokType = mapToKeyword(lexeme);
                // If the returned token is Identifier, try mapping it to ValueTypeHint
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
                }
                
                return raiseError("Invalid identifier as it must not start, nor end with _");
            case LexerState::Integer:
                if(current == '.') {
                    // Peeks 1 character ahead if it's a digit
                    // If it it's not, then it must be an integer literal
                    // If it is a digit, then it must be an IP Address
                    bool isDigitNext = (
                        m_Current + 1 < m_Source.size() &&
                        std::isdigit(m_Source[m_Current + 1])
                    );

                    if(!isDigitNext) {
                        tokType = TokenType::IntegerLiteral;
                        transitionTo(LexerState::Finish);
                        break;
                    }
                    transitionTo(LexerState::IPv4a1);
                    advance();
                    break;
                }
                if(!std::isdigit(current)) {
                    tokType = TokenType::IntegerLiteral;
                    transitionTo(LexerState::Finish);
                    break;
                }
                advance();
                break;
            // 192.[162].0.1
            // IPv4a[n] is the dots, IPv4a[n]Digits is the actual digit to prevent 1.../23 being a valid IP address
            case LexerState::IPv4a1:
                if(!std::isdigit(current))
                    return raiseError("Expected digit");
                advance();
                transitionTo(LexerState::IPv4a1Digits);
                break;
            case LexerState::IPv4a1Digits:
                if(current == '.') {
                    transitionTo(LexerState::IPv4a2);
                    advance();
                    break;
                }
                if(!std::isdigit(current))
                    return raiseError("Expected digit, or '.'");
                advance();
                break;
            // 192.162.[0].1
            case LexerState::IPv4a2:
                if(!std::isdigit(current))
                    return raiseError("Expected digit");
                advance();
                transitionTo(LexerState::IPv4a2Digits);
                break;
            case LexerState::IPv4a2Digits:
                if(current == '.') {
                    transitionTo(LexerState::IPv4a3);
                    advance();
                    break;
                }
                if(!std::isdigit(current))
                    return raiseError("Expected digit, or '.'");
                advance();
                break;
            // 192.162.0.[1]
            case LexerState::IPv4a3:
                if(!std::isdigit(current))
                    return raiseError("Expected digit");
                advance();
                transitionTo(LexerState::IPv4a3Digits);
                break;
            case LexerState::IPv4a3Digits:
                if(current == '/') {
                    transitionTo(LexerState::SlashCIDR);
                    advance();
                    break;
                }

                if(current == '.')
                    return raiseError("Invalid IPv4 address");

                if(!std::isdigit(current)) {
                    tokType = TokenType::IPv4Literal;
                    transitionTo(LexerState::Finish);
                    break;
                }
                advance();
                break;
            case LexerState::SlashCIDR:
                if(!std::isdigit(current))
                    return raiseError("Expected CIDR");

                advance();
                transitionTo(LexerState::CIDR);
                break;
            case LexerState::CIDR:
                if(!std::isdigit(current))
                {
                    tokType = TokenType::IPv4CIDRLiteral;
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
                if(current == '\n' || current == '\0')
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
            case LexerState::Modulo:
                tokType = TokenType::Modulo;
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
                if(current == '.') {
                    transitionTo(LexerState::Range);
                    advance();
                    break;
                }

                tokType = TokenType::Dot;
                transitionTo(LexerState::Finish);
                break;
            case LexerState::Range:
                tokType = TokenType::Range;
                transitionTo(LexerState::Finish);
                break;
            case LexerState::Colon:
                tokType = TokenType::Colon;
                transitionTo(LexerState::Finish);
                break;
            case LexerState::Comment:
                tokType = TokenType::Comment;
                if(current == '\n' || current == '\0')
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
                else if(current != '*')
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
            case LexerState::EndOfStatement:
                tokType = TokenType::EndOfStatement;
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
