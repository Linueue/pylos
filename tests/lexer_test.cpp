#include "token.hpp"
#include <catch2/catch_test_macros.hpp>

#include <lexer.hpp>

static Token defaultToken = {
    .type = TokenType::EndOfFile,
    .lexeme = "",
    .line = 0,
    .column = 0,
};

void tokenRequire(Lexer& lexer, const std::vector<TokenType> tokens) {
    for(const auto tok : tokens) {
        auto nextTok = lexer.Next();

        if(!nextTok) {
            auto diagnostic = nextTok.Error();

            UNSCOPED_INFO("Lexer error: "
                << diagnostic.reason
                << " at line " << diagnostic.line
                << ", column " << diagnostic.column
            );
            REQUIRE(!nextTok.IsError());
            return;
        }

        auto actualTok = nextTok.Value();
        REQUIRE(actualTok == tok);
    }
}

TEST_CASE("Lexer tokenize IPv4", "[lexer][IPv4]") {
    Lexer lexer("192.168.0.12");

    tokenRequire(lexer, {TokenType::IPv4});
}

TEST_CASE("Lexer tokenize IPv4CIDR", "[lexer][IPv4CIDR]") {
    Lexer lexer("192.168.0.12/23");

    tokenRequire(lexer, {TokenType::IPv4CIDR});
}

TEST_CASE("Lexer tokenize identifier", "[lexer][identifier]") {
    Lexer lexer("hel_lo53");

    tokenRequire(lexer, {TokenType::Identifier});
}

TEST_CASE("Lexer tokenize assign", "[lexer][assign]") {
    Lexer lexer("val ip: Subnet = 192.168.100.5/23");

    tokenRequire(lexer, {
        TokenType::Val,
        TokenType::Identifier,
        TokenType::Colon,
        TokenType::ValueTypeHint,
        TokenType::Assign,
        TokenType::IPv4CIDR,
    });
}

TEST_CASE("Lexer tokenize scopes", "[lexer][scopes]") {
    Lexer lexer(R"(network Campus {
        val hr: Subnet = 192.168.10.0/24
        val hrVlan = vlan(id = 10, subnet = hr, gateway = hr.host(1))
    })");

    tokenRequire(lexer, {
        TokenType::Network,
        TokenType::Identifier,
        TokenType::LeftCurly,

        TokenType::Val,
        TokenType::Identifier,
        TokenType::Colon,
        TokenType::ValueTypeHint,
        TokenType::Assign,
        TokenType::IPv4CIDR,

        TokenType::Val,
        TokenType::Identifier,
        TokenType::Assign,
        TokenType::Identifier,
        TokenType::LeftParen,

        TokenType::Identifier,
        TokenType::Assign,
        TokenType::Integer,
        TokenType::Comma,

        TokenType::Identifier,
        TokenType::Assign,
        TokenType::Identifier,
        TokenType::Comma,

        TokenType::Identifier,
        TokenType::Assign,
        TokenType::Identifier,
        TokenType::Dot,
        TokenType::Identifier,
        TokenType::LeftParen,
        TokenType::Integer,
        TokenType::RightParen,

        TokenType::RightParen,

        TokenType::RightCurly,
    });
}

TEST_CASE("Lexer tokenize if statements", "[lexer][statements]") {
    Lexer lexer(R"(val hasGuest: Bool = false

    // Ignore this comment
    // This is a test
    /*
        This is also a test
    */

    if hasGuest && 12 > 5 || 23 * 5 >= 5 {
        port Gi0/2 {
            mode = Access(vlan)
            enabled = true
            label = "Guest"
        }
    })");

    tokenRequire(lexer, {
        TokenType::Val,
        TokenType::Identifier,
        TokenType::Colon,
        TokenType::ValueTypeHint,
        TokenType::Assign,
        TokenType::BooleanLiteral,

        TokenType::If,
        TokenType::Identifier,
        TokenType::Logical,
        TokenType::Integer,
        TokenType::Relational,
        TokenType::Integer,
        TokenType::Logical,
        TokenType::Integer,
        TokenType::Multiply,
        TokenType::Integer,
        TokenType::Relational,
        TokenType::Integer,
        TokenType::LeftCurly,

        TokenType::Port,
        TokenType::Identifier,
        TokenType::Divide,
        TokenType::Integer,
        TokenType::LeftCurly,

        TokenType::Identifier,
        TokenType::Assign,
        TokenType::Identifier,
        TokenType::LeftParen,
        TokenType::Identifier,
        TokenType::RightParen,

        TokenType::Identifier,
        TokenType::Assign,
        TokenType::BooleanLiteral,

        TokenType::Identifier,
        TokenType::Assign,
        TokenType::StringLiteral,
        TokenType::RightCurly,

        TokenType::RightCurly,
    });
}
