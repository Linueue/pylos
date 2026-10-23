#include <catch2/catch_test_macros.hpp>

#include <lexer.hpp>

static Token defaultToken = {
    .type = TokenType::EndOfFile,
    .lexeme = "",
    .line = 0,
    .column = 0,
};

TEST_CASE("Lexer tokenize IPv4", "[lexer][IPv4]") {
    Lexer lexer("192.168.0.12");

    REQUIRE(lexer.Next().Or(defaultToken) == TokenType::IPv4);
}

TEST_CASE("Lexer tokenize IPv4CIDR", "[lexer][IPv4CIDR]") {
    Lexer lexer("192.168.0.12/23");

    REQUIRE(lexer.Next().Or(defaultToken) == TokenType::IPv4CIDR);
}

TEST_CASE("Lexer tokenize identifier", "[lexer][identifier]") {
    Lexer lexer("hel_lo53");

    REQUIRE(lexer.Next().Or(defaultToken) == TokenType::Identifier);
}

TEST_CASE("Lexer tokenize assign", "[lexer][assign]") {
    Lexer lexer("val ip = 192.168.100.5/23");

    REQUIRE(lexer.Next().Or(defaultToken) == TokenType::Val);
    REQUIRE(lexer.Next().Or(defaultToken) == TokenType::Identifier);
    REQUIRE(lexer.Next().Or(defaultToken) == TokenType::Assign);
    REQUIRE(lexer.Next().Or(defaultToken) == TokenType::IPv4CIDR);
}
