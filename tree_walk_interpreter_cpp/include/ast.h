#pragma once
#include "token.h"
#include <memory>
#include <string>
#include <variant>
#include <vector>

using ExprPtr = std::shared_ptr<struct Expr>;
using StmtPtr = std::shared_ptr<struct Stmt>;

using Value = std::variant<std::monostate, double, bool, std::string>;

struct Expr {
    virtual ~Expr() = default;
};

struct LiteralExpr : Expr {
    Value value;
    explicit LiteralExpr(Value v) : value(std::move(v)) {}
};

struct VariableExpr : Expr {
    Token name;
    explicit VariableExpr(Token n) : name(std::move(n)) {}
};

struct GroupingExpr : Expr {
    ExprPtr expression;
    explicit GroupingExpr(ExprPtr e) : expression(std::move(e)) {}
};

struct UnaryExpr : Expr {
    Token op;
    ExprPtr right;
    UnaryExpr(Token o, ExprPtr r) : op(std::move(o)), right(std::move(r)) {}
};

struct BinaryExpr : Expr {
    ExprPtr left;
    Token op;
    ExprPtr right;
    BinaryExpr(ExprPtr l, Token o, ExprPtr r)
        : left(std::move(l)), op(std::move(o)), right(std::move(r)) {}
};

struct LogicalExpr : Expr {
    ExprPtr left;
    Token op;
    ExprPtr right;
    LogicalExpr(ExprPtr l, Token o, ExprPtr r)
        : left(std::move(l)), op(std::move(o)), right(std::move(r)) {}
};

struct AssignExpr : Expr {
    Token name;
    ExprPtr value;
    AssignExpr(Token n, ExprPtr v) : name(std::move(n)), value(std::move(v)) {}
};

struct Stmt {
    virtual ~Stmt() = default;
};

struct ExpressionStmt : Stmt {
    ExprPtr expression;
    explicit ExpressionStmt(ExprPtr e) : expression(std::move(e)) {}
};

struct PrintStmt : Stmt {
    ExprPtr expression;
    explicit PrintStmt(ExprPtr e) : expression(std::move(e)) {}
};

struct VarStmt : Stmt {
    Token name;
    ExprPtr initializer;
    VarStmt(Token n, ExprPtr i) : name(std::move(n)), initializer(std::move(i)) {}
};

struct BlockStmt : Stmt {
    std::vector<StmtPtr> statements;
    explicit BlockStmt(std::vector<StmtPtr> s) : statements(std::move(s)) {}
};

struct IfStmt : Stmt {
    ExprPtr condition;
    StmtPtr thenBranch;
    StmtPtr elseBranch;
    IfStmt(ExprPtr c, StmtPtr t, StmtPtr e)
        : condition(std::move(c)), thenBranch(std::move(t)), elseBranch(std::move(e)) {}
};

struct WhileStmt : Stmt {
    ExprPtr condition;
    StmtPtr body;
    WhileStmt(ExprPtr c, StmtPtr b) : condition(std::move(c)), body(std::move(b)) {}
};
