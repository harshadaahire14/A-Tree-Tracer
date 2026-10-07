#pragma once
#include "ast.h"
#include "environment.h"
#include "tracer.h"
#include <memory>
#include <string>
#include <vector>

class Interpreter {
public:
    Interpreter();

    void interpret(const std::vector<StmtPtr>& statements);
    void setTracing(bool enabled);
    Tracer& tracer();

private:
    std::shared_ptr<Environment> globals;
    std::shared_ptr<Environment> environment;
    Tracer tracer_;

    void execute(const StmtPtr& stmt);
    Value evaluate(const ExprPtr& expr);
    void executeBlock(const std::vector<StmtPtr>& statements,
                      const std::shared_ptr<Environment>& newEnvironment);

    Value visitLiteral(const LiteralExpr& expr);
    Value visitGrouping(const GroupingExpr& expr);
    Value visitUnary(const UnaryExpr& expr);
    Value visitBinary(const BinaryExpr& expr);
    Value visitLogical(const LogicalExpr& expr);
    Value visitVariable(const VariableExpr& expr);
    Value visitAssign(const AssignExpr& expr);

    bool isTruthy(const Value& value) const;
    bool isEqual(const Value& a, const Value& b) const;
    bool isNumber(const Value& value) const;
    double asNumber(const Value& value, const Token& token) const;
    std::string stringify(const Value& value) const;
    std::string valueForTrace(const Value& value) const;
};
