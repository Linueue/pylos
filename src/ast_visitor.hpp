#pragma once

#include "ast.hpp"

/*
* The ASTVisitor class that must be derived to use
* This is inspired by Clang's CRTP (Curiously Recurring Template Pattern) Visitor pattern
* This is from OOP's Visitor Pattern (https://en.wikipedia.org/wiki/Visitor_pattern)
* The idea of CRTP is to get rid of runtime Dispatch which can induce some performance costs
* To be able to traverse the entire AST
* We can then extend this to do other functionalities, such as printing, evaluating, etc.
*/
template <typename Derived>
class ASTVisitor {
public:
    void Dispatch(const ASTNode* node) {
        switch(node->type) {
            #define AST_NODE(name) \
            case NodeType::name: \
                getDerived().Visit(static_cast<const name*>(node)); \
                break;
            #include "ast_nodes.inc"
            #undef AST_NODE
        }
    }

    void Visit(const ASTNode*) {}

    #define AST_NODE(name) \
    void Visit(const name* node) { getDerived().visit(static_cast<const ASTNode*>(node)); }
    #include "ast_nodes.inc"
    #undef AST_NODE

private:
    Derived& getDerived() {
        return static_cast<Derived&>(*this);
    }
};

class PrintVisitor : public ASTVisitor<PrintVisitor> {
public:
    void Visit(const IntegerLiteral* node);
    void Visit(const BooleanLiteral* node);
    void Visit(const StringLiteral* node);
    void Visit(const IPAddressLiteral* node);
    void Visit(const CIDRLiteral* node);
    void Visit(const BinaryExpr* node);
    void Visit(const VariableDecl* node);
    void Visit(const VariableRef* node);
    void Visit(const BlockExpr* node);

private:
    void printTabs();

private:
    int32_t m_Tabs = 0;
};
