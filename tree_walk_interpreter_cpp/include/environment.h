#pragma once
#include "token.h"
#include "ast.h"
#include <memory>
#include <stdexcept>
#include <string>
#include <unordered_map>

class RuntimeError : public std::runtime_error {
public:
    Token token;
    RuntimeError(const Token& t, const std::string& message)
        : std::runtime_error(message), token(t) {}
};

class Environment : public std::enable_shared_from_this<Environment> {
public:
    explicit Environment(std::shared_ptr<Environment> enclosing = nullptr);

    void define(const std::string& name, const Value& value);
    void assign(const Token& name, const Value& value);
    Value get(const Token& name) const;

    std::shared_ptr<Environment> enclosing;

private:
    std::unordered_map<std::string, Value> values;
};
