#pragma once

#include <string_view>

#include "types.hpp"

#define TOKEN_LIST() \
    X(Identifier), \
    X(Val), \
    X(ValueTypeHint), \
    X(Network), \
    X(Switch), \
    X(Port), \
    X(If), \
    X(Else), \
    X(For), \
    X(Integer), \
    X(IPv4), \
    X(IPv4CIDR), \
    X(StringLiteral), \
    X(Assign), \
    X(Plus), \
    X(Minus), \
    X(Multiply), \
    X(Divide), \
    X(Modulo), \
    X(LeftCurly), \
    X(RightCurly), \
    X(LeftBracket), \
    X(RightBracket), \
    X(LeftParen), \
    X(RightParen), \
    X(Comma), \
    X(Dot), \
    X(Range), \
    X(Colon), \
    X(Comment), \
    X(BooleanLiteral), \
    X(Logical), \
    X(Relational), \
    X(EndOfFile), \

#define X(name) name
enum class TokenType {
    TOKEN_LIST()
};
#undef X

struct Token {
    TokenType type;
    std::string_view lexeme;
    size_t line, column;

    int AsInt() const;
    IPv4 AsIPv4() const;
    Cidr AsCIDR() const;

    std::string_view GetTokenString() const;
    static std::string_view GetTokenString(TokenType type);

    inline bool operator==(const TokenType other) const { return type == other; }
};
