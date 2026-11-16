#include "ast_visitor.hpp"

#include <iostream>

void PrintVisitor::Visit(const IntegerLiteral* node) {
    std::cout << "IntegerLiteral(" << node->value << "),\n";
}

void PrintVisitor::Visit(const BooleanLiteral* node) {
    std::cout << "BooleanLiteral(" << node->value << "),\n";
}

void PrintVisitor::Visit(const StringLiteral* node) {
    std::cout << "StringLiteral(" << node->value << "),\n";
}

void PrintVisitor::Visit(const IPAddressLiteral* node) {
    std::cout << "IPAddressLiteral(" << node->value.AsString() << "),\n";
}

void PrintVisitor::Visit(const CIDRLiteral* node) {
    std::cout << "CIDRLiteral(" << node->value.ip.AsString() << "/" << (int32_t)node->value.cidr << "),\n";
}

void PrintVisitor::Visit(const BinaryExpr* node) {
    std::cout << "BinaryExpr(\n";

    m_Tabs++;
    printTabs();
    std::cout << "lhs = ";
    Dispatch(node->lhs);

    printTabs();
    std::cout << "rhs = ";
    Dispatch(node->rhs);

    printTabs();
    std::cout << "op = " << node->AsString() << ",\n";
    m_Tabs--;

    printTabs();
    std::cout << "),\n";
}

void PrintVisitor::Visit(const VariableDecl* node) {
    std::cout << "VariableDecl(\n";

    m_Tabs++;
    printTabs();

    std::cout << "name = " << node->name << ",\n";

    printTabs();
    std::cout << "expr = ";
    Dispatch(node->expr);

    m_Tabs--;
    printTabs();
    std::cout << "),\n";
}

void PrintVisitor::Visit(const VariableRef* node) {
    std::cout << "VariableRef(" << node->name << "),\n";
}

void PrintVisitor::Visit(const BlockExpr* node) {
    std::cout << "BlockExpr(statements = [\n";
    m_Tabs++;
    for(auto stmt : node->statements) {
        printTabs();
        Dispatch(stmt);
    }
    m_Tabs--;
    printTabs();
    std::cout << "]),\n";
}

void PrintVisitor::printTabs() {
    for(int32_t i = 0; i < m_Tabs; i++)
        std::cout << "  ";
}
