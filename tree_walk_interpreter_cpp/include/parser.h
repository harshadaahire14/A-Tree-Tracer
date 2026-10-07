#pragma once
#include "ast.h"
#include <stdexcept>
#include <vector>

class Parser {
public:
    explicit Parser(const std::vector<Token>& tokens);
    std::vector<StmtPtr> parse();

private:
    const std::vector<Token>& tokens;
    size_t current = 0;

    StmtPtr declaration();
    StmtPtr statement();
    StmtPtr printStatement();
    StmtPtr varDeclaration();
    StmtPtr ifStatement();
    StmtPtr whileStatement();
    StmtPtr expressionStatement();
    StmtPtr block();

    ExprPtr expression();
    ExprPtr assignment();
    ExprPtr logicalOr();
    ExprPtr logicalAnd();
    ExprPtr equality();
    ExprPtr comparison();
    ExprPtr term();
    ExprPtr factor();
    ExprPtr unary();
    ExprPtr primary();

    bool match(std::initializer_list<TokenType> types);
    bool check(TokenType type) const;
    Token advance();
    bool isAtEnd() const;
    Token peek() const;
    Token previous() const;
    Token consume(TokenType type, const std::string& message);
    void synchronize();

    struct ParseError : std::runtime_error {
        using std::runtime_error::runtime_error;
    };
};
