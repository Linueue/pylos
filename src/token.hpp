#pragma once

#include <string_view>

#include "utils/expected.hpp"
#include "types.hpp"

/*
 * The tokens list using the X Macro technique
 *
 * This makes it possible to create an enum of Tokens
 * and a list of strings that can be accessed as g_TokenString[(int32_t)TokenType::Identifier]
 * by definint a TOKEN_LIST macro once
*/
#define TOKEN_LIST() \
    X(Identifier), \
    X(Val), \
    X(ValueTypeHint), \
    X(Network), \
    X(Switch), \
    X(Router), \
    X(Interface), \
    X(Port), \
    X(If), \
    X(Else), \
    X(For), \
    X(While), \
    X(Do), \
    X(In), \
    X(Access), \
    X(Trunk), \
    X(Vlan), \
    X(IntegerLiteral), \
    X(IPv4Literal), \
    X(IPv4CIDRLiteral), \
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
    X(EndOfStatement), \
    X(EndOfFile), \

#define X(name) name
enum class TokenType {
    TOKEN_LIST()
};
#undef X

/*
 * The Token struct with type, lexeme, line, and column
 * This provides utilities such as AsInt(), AsIPv4, and AsCIDR
 * It also provides functions such as GetTokenString that returns the string version
 * of the token type
 * Supports == operator against TokenType
 */
struct Token {
    TokenType type;
    std::string_view lexeme;
    size_t line, column;

    /*
     * Tries to convert to int32_t/IPv4/Cidr
     * Used in syntax analyzers
     * @return
     *  On success, returns T
     *  On failure, returns error description
     */
    Expected<int32_t, const char*> AsInt() const;
    Expected<IPv4, const char*> AsIPv4() const;
    Expected<Cidr, const char*> AsCIDR() const;

    std::string_view GetTokenString() const;
    static std::string_view GetTokenString(TokenType type);

    inline bool operator==(const TokenType other) const { return type == other; }
};
