#pragma once

#include <string_view>

#include "types.hpp"

#define TOKEN_LIST() \
    X(Identifier), \
    X(Integer), \
    X(IPv4), \
    X(IPv4CIDR), \
    X(Plus), \
    X(Minus), \
    X(Multiply), \
    X(Divide), \
    X(LeftCurly), \
    X(RightCurly), \
    X(LeftParen), \
    X(RightParen), \
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

    inline bool operator==(const TokenType other) const { return type == other; }
};
