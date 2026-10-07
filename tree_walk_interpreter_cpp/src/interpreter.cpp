#include "interpreter.h"
#include <cmath>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <type_traits>

Interpreter::Interpreter() {
    globals = std::make_shared<Environment>();
    environment = globals;
}

void Interpreter::setTracing(bool enabled) {
    tracer_.setEnabled(enabled);
}

Tracer& Interpreter::tracer() {
    return tracer_;
}

void Interpreter::interpret(const std::vector<StmtPtr>& statements) {
    try {
        for (const auto& stmt : statements) execute(stmt);
    } catch (const RuntimeError& error) {
        std::cerr << "Runtime error at line " << error.token.line
                  << ": " << error.what() << '\n';
    }
}

void Interpreter::execute(const StmtPtr& stmt) {
    if (auto s = std::dynamic_pointer_cast<ExpressionStmt>(stmt)) {
        tracer_.enter("ExpressionStmt");
        Value value = evaluate(s->expression);
        tracer_.exit("ExpressionStmt", valueForTrace(value));
        return;
    }

    if (auto s = std::dynamic_pointer_cast<PrintStmt>(stmt)) {
        tracer_.enter("PrintStmt");
        Value value = evaluate(s->expression);
        std::cout << stringify(value) << '\n';
        tracer_.exit("PrintStmt", valueForTrace(value));
        return;
    }

    if (auto s = std::dynamic_pointer_cast<VarStmt>(stmt)) {
        tracer_.enter("VarStmt: " + s->name.lexeme);
        Value value = evaluate(s->initializer);
        environment->define(s->name.lexeme, value);
        tracer_.event(s->name.lexeme + " = " + valueForTrace(value));
        tracer_.exit("VarStmt");
        return;
    }

    if (auto s = std::dynamic_pointer_cast<BlockStmt>(stmt)) {
        tracer_.enter("BlockStmt");
        executeBlock(s->statements, std::make_shared<Environment>(environment));
        tracer_.exit("BlockStmt");
        return;
    }

    if (auto s = std::dynamic_pointer_cast<IfStmt>(stmt)) {
        tracer_.enter("IfStmt");
        Value condition = evaluate(s->condition);
        tracer_.event("condition = " + valueForTrace(condition));
        if (isTruthy(condition)) {
            execute(s->thenBranch);
        } else if (s->elseBranch) {
            execute(s->elseBranch);
        }
        tracer_.exit("IfStmt");
        return;
    }

    if (auto s = std::dynamic_pointer_cast<WhileStmt>(stmt)) {
        tracer_.enter("WhileStmt");
        int iterations = 0;
        while (isTruthy(evaluate(s->condition))) {
            execute(s->body);
            ++iterations;
        }
        tracer_.event("iterations = " + std::to_string(iterations));
        tracer_.exit("WhileStmt");
        return;
    }
}

void Interpreter::executeBlock(const std::vector<StmtPtr>& statements,
                               const std::shared_ptr<Environment>& newEnvironment) {
    auto previous = environment;
    try {
        environment = newEnvironment;
        for (const auto& stmt : statements) execute(stmt);
    } catch (...) {
        environment = previous;
        throw;
    }
    environment = previous;
}

Value Interpreter::evaluate(const ExprPtr& expr) {
    if (auto e = std::dynamic_pointer_cast<LiteralExpr>(expr)) {
        return visitLiteral(*e);
    }
    if (auto e = std::dynamic_pointer_cast<GroupingExpr>(expr)) {
        return visitGrouping(*e);
    }
    if (auto e = std::dynamic_pointer_cast<UnaryExpr>(expr)) {
        return visitUnary(*e);
    }
    if (auto e = std::dynamic_pointer_cast<BinaryExpr>(expr)) {
        return visitBinary(*e);
    }
    if (auto e = std::dynamic_pointer_cast<LogicalExpr>(expr)) {
        return visitLogical(*e);
    }
    if (auto e = std::dynamic_pointer_cast<VariableExpr>(expr)) {
        return visitVariable(*e);
    }
    if (auto e = std::dynamic_pointer_cast<AssignExpr>(expr)) {
        return visitAssign(*e);
    }

    throw std::runtime_error("Unknown expression node.");
}

Value Interpreter::visitLiteral(const LiteralExpr& expr) {
    tracer_.enter("Literal");
    tracer_.exit("Literal", valueForTrace(expr.value));
    return expr.value;
}

Value Interpreter::visitGrouping(const GroupingExpr& expr) {
    tracer_.enter("Grouping");
    Value value = evaluate(expr.expression);
    tracer_.exit("Grouping", valueForTrace(value));
    return value;
}

Value Interpreter::visitUnary(const UnaryExpr& expr) {
    tracer_.enter("Unary '" + expr.op.lexeme + "'");
    Value right = evaluate(expr.right);
    Value result;

    if (expr.op.type == TokenType::MINUS) {
        result = -asNumber(right, expr.op);
    } else if (expr.op.type == TokenType::BANG) {
        result = !isTruthy(right);
    }

    tracer_.exit("Unary", valueForTrace(result));
    return result;
}

Value Interpreter::visitBinary(const BinaryExpr& expr) {
    tracer_.enter("Binary '" + expr.op.lexeme + "'");
    Value left = evaluate(expr.left);
    Value right = evaluate(expr.right);
    Value result;

    switch (expr.op.type) {
        case TokenType::PLUS:
            if (isNumber(left) && isNumber(right)) {
                result = std::get<double>(left) + std::get<double>(right);
            } else if (std::holds_alternative<std::string>(left) &&
                       std::holds_alternative<std::string>(right)) {
                result = std::get<std::string>(left) + std::get<std::string>(right);
            } else {
                throw RuntimeError(expr.op, "Operands must both be numbers or strings.");
            }
            break;

        case TokenType::MINUS:
            result = asNumber(left, expr.op) - asNumber(right, expr.op);
            break;
        case TokenType::STAR:
            result = asNumber(left, expr.op) * asNumber(right, expr.op);
            break;
        case TokenType::SLASH: {
            double divisor = asNumber(right, expr.op);
            if (std::abs(divisor) < 1e-12)
                throw RuntimeError(expr.op, "Cannot divide by zero.");
            result = asNumber(left, expr.op) / divisor;
            break;
        }
        case TokenType::GREATER:
            result = asNumber(left, expr.op) > asNumber(right, expr.op);
            break;
        case TokenType::GREATER_EQUAL:
            result = asNumber(left, expr.op) >= asNumber(right, expr.op);
            break;
        case TokenType::LESS:
            result = asNumber(left, expr.op) < asNumber(right, expr.op);
            break;
        case TokenType::LESS_EQUAL:
            result = asNumber(left, expr.op) <= asNumber(right, expr.op);
            break;
        case TokenType::EQUAL_EQUAL:
            result = isEqual(left, right);
            break;
        case TokenType::BANG_EQUAL:
            result = !isEqual(left, right);
            break;
        default:
            throw RuntimeError(expr.op, "Unsupported binary operator.");
    }

    tracer_.exit("Binary", valueForTrace(result));
    return result;
}

Value Interpreter::visitLogical(const LogicalExpr& expr) {
    tracer_.enter("Logical '" + expr.op.lexeme + "'");
    Value left = evaluate(expr.left);

    if (expr.op.type == TokenType::OR && isTruthy(left)) {
        tracer_.exit("Logical", valueForTrace(left));
        return left;
    }

    if (expr.op.type == TokenType::AND && !isTruthy(left)) {
        tracer_.exit("Logical", valueForTrace(left));
        return left;
    }

    Value right = evaluate(expr.right);
    tracer_.exit("Logical", valueForTrace(right));
    return right;
}

Value Interpreter::visitVariable(const VariableExpr& expr) {
    tracer_.enter("Variable '" + expr.name.lexeme + "'");
    Value value = environment->get(expr.name);
    tracer_.exit("Variable", valueForTrace(value));
    return value;
}

Value Interpreter::visitAssign(const AssignExpr& expr) {
    tracer_.enter("Assignment '" + expr.name.lexeme + "'");
    Value value = evaluate(expr.value);
    environment->assign(expr.name, value);
    tracer_.exit("Assignment", valueForTrace(value));
    return value;
}

bool Interpreter::isTruthy(const Value& value) const {
    if (std::holds_alternative<std::monostate>(value)) return false;
    if (std::holds_alternative<bool>(value)) return std::get<bool>(value);
    if (std::holds_alternative<double>(value)) return std::get<double>(value) != 0.0;
    return !std::get<std::string>(value).empty();
}

bool Interpreter::isEqual(const Value& a, const Value& b) const {
    if (a.index() != b.index()) return false;
    if (std::holds_alternative<std::monostate>(a)) return true;
    if (std::holds_alternative<double>(a)) return std::get<double>(a) == std::get<double>(b);
    if (std::holds_alternative<bool>(a)) return std::get<bool>(a) == std::get<bool>(b);
    return std::get<std::string>(a) == std::get<std::string>(b);
}

bool Interpreter::isNumber(const Value& value) const {
    return std::holds_alternative<double>(value);
}

double Interpreter::asNumber(const Value& value, const Token& token) const {
    if (!isNumber(value))
        throw RuntimeError(token, "Operand must be a number.");
    return std::get<double>(value);
}

std::string Interpreter::stringify(const Value& value) const {
    if (std::holds_alternative<std::monostate>(value)) return "nil";
    if (std::holds_alternative<bool>(value)) return std::get<bool>(value) ? "true" : "false";
    if (std::holds_alternative<std::string>(value)) return std::get<std::string>(value);

    double number = std::get<double>(value);
    std::ostringstream out;
    out << std::setprecision(15) << number;
    std::string s = out.str();

    if (s.find('.') != std::string::npos) {
        while (!s.empty() && s.back() == '0') s.pop_back();
        if (!s.empty() && s.back() == '.') s.pop_back();
    }
    return s;
}

std::string Interpreter::valueForTrace(const Value& value) const {
    return stringify(value);
}
