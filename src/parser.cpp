#include "parser.hpp"

Parser::Parser(Lexer& lexer)
    : m_Lexer(lexer) {}

BlockExpr* Parser::Parse() {
    BlockExpr* block = parseBlock();

    return block;
}
