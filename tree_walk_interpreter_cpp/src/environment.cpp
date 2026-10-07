#include "environment.h"

Environment::Environment(std::shared_ptr<Environment> enclosing)
    : enclosing(std::move(enclosing)) {}

void Environment::define(const std::string& name, const Value& value) {
    values[name] = value;
}

void Environment::assign(const Token& name, const Value& value) {
    auto it = values.find(name.lexeme);
    if (it != values.end()) {
        it->second = value;
        return;
    }

    if (enclosing) {
        enclosing->assign(name, value);
        return;
    }

    throw RuntimeError(name, "Undefined variable '" + name.lexeme + "'.");
}

Value Environment::get(const Token& name) const {
    auto it = values.find(name.lexeme);
    if (it != values.end()) return it->second;

    if (enclosing) return enclosing->get(name);

    throw RuntimeError(name, "Undefined variable '" + name.lexeme + "'.");
}
