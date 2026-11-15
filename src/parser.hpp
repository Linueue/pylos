#pragma once

#include "lexer.hpp"
#include "ast.hpp"

class Parser {
public:
    Parser(Lexer& lexer);

    BlockExpr* Parse();

private:
    BlockExpr* parseBlock();

private:
    Lexer& m_Lexer;
    ASTContext m_Context;
};
