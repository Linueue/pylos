#pragma once

#include <concepts>
#include <memory_resource>
#include <string_view>
#include <span>

#include "types.hpp"

struct ASTNode;

class ASTContext {
public:
    ASTContext() = default;

    template <typename T, typename... Args>
    requires std::constructible_from<T, Args...>
    T* Create(Args&&... args) {
        void* mem = m_Arena.allocate(sizeof(T), alignof(T));
        return new (mem) T(std::forward<Args>(args)...);
    }

    std::span<ASTNode*> CreateNodeList(const std::vector<ASTNode*>& nodes) {
        if(nodes.empty())
            return {};

        size_t bytes = nodes.size() * sizeof(ASTNode*);
        void* mem = m_Arena.allocate(bytes, alignof(ASTNode*));
        std::memcpy(mem, nodes.data(), bytes);

        return {(ASTNode**)mem, nodes.size()};
    }

private:
    std::pmr::monotonic_buffer_resource m_Arena;
};

enum class NodeType {
    Integer, Boolean, String, IPAddress, CIDR, Binary, VariableDecl, VariableRef, BlockExpr,
};

struct ASTNode {
    const NodeType type;
    explicit ASTNode(NodeType type) : type(type) {}
};

struct IntegerLiteral : ASTNode {
    int32_t value;

    explicit IntegerLiteral(int32_t value)
        : ASTNode(NodeType::Integer), value(value) {}
};

struct BooleanLiteral : ASTNode {
    bool value;

    explicit BooleanLiteral(bool value)
        : ASTNode(NodeType::Boolean), value(value) {}
};

struct StringLiteral : ASTNode {
    std::string_view value;

    explicit StringLiteral(std::string_view value)
        : ASTNode(NodeType::String), value(value) {}
};

struct IPAddressLiteral : ASTNode {
    IPv4 value;

    explicit IPAddressLiteral(IPv4 value)
        : ASTNode(NodeType::IPAddress), value(value) {}
};

struct CIDRLiteral : ASTNode {
    Cidr value;

    explicit CIDRLiteral(Cidr value)
        : ASTNode(NodeType::CIDR), value(value) {}
};

struct BinaryExpr : ASTNode {
    enum class Op {
        Add, Sub, Mul, Div,
    };
    Op op;
    ASTNode* lhs;
    ASTNode* rhs;

    explicit BinaryExpr(ASTNode* lhs, ASTNode* rhs, Op op)
        : ASTNode(NodeType::Binary), op(op), lhs(lhs), rhs(rhs) {}
};

struct VariableDecl : ASTNode {
    std::string_view name;
    ASTNode* expr;

    explicit VariableDecl(std::string_view name, ASTNode* expr)
        : ASTNode(NodeType::VariableDecl), name(name), expr(expr) {}
};

struct VariableRef : ASTNode {
    std::string_view name;

    explicit VariableRef(std::string_view name)
        : ASTNode(NodeType::VariableRef), name(name) {}
};

struct BlockExpr : ASTNode {
    std::span<ASTNode*> statements;

    explicit BlockExpr(std::span<ASTNode*> statements)
        : ASTNode(NodeType::BlockExpr), statements(statements) {}
};
