#include "token.hpp"
#include <catch2/catch_test_macros.hpp>

#include <lexer.hpp>

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

        UNSCOPED_INFO("Expected: "
            << Token::GetTokenString(tok)
            << " == "
            << actualTok.GetTokenString()
        );

        REQUIRE(actualTok == tok);
    }
}

TEST_CASE("Lexer tokenize IPv4", "[lexer][IPv4]") {
    Lexer lexer("192.168.0.12");

    tokenRequire(lexer, {TokenType::IPv4Literal});
}

TEST_CASE("Lexer tokenize IPv4CIDRLiteral", "[lexer][IPv4CIDR]") {
    Lexer lexer("192.168.0.12/23");

    tokenRequire(lexer, {TokenType::IPv4CIDRLiteral});
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
        TokenType::IPv4CIDRLiteral,
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
        TokenType::IPv4CIDRLiteral,

        TokenType::Val,
        TokenType::Identifier,
        TokenType::Assign,
        TokenType::Vlan,
        TokenType::LeftParen,

        TokenType::Identifier,
        TokenType::Assign,
        TokenType::IntegerLiteral,
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
        TokenType::IntegerLiteral,
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
            mode = Access(hrVlan)
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
        TokenType::IntegerLiteral,
        TokenType::Relational,
        TokenType::IntegerLiteral,
        TokenType::Logical,
        TokenType::IntegerLiteral,
        TokenType::Multiply,
        TokenType::IntegerLiteral,
        TokenType::Relational,
        TokenType::IntegerLiteral,
        TokenType::LeftCurly,

        TokenType::Port,
        TokenType::Identifier,
        TokenType::Divide,
        TokenType::IntegerLiteral,
        TokenType::LeftCurly,

        TokenType::Identifier,
        TokenType::Assign,
        TokenType::Access,
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
