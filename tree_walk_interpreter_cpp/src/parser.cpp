#include "parser.h"
#include <initializer_list>
#include <sstream>

Parser::Parser(const std::vector<Token>& tokens) : tokens(tokens) {}

std::vector<StmtPtr> Parser::parse() {
    std::vector<StmtPtr> statements;
    while (!isAtEnd()) {
        try {
            statements.push_back(declaration());
        } catch (const ParseError&) {
            synchronize();
        }
    }
    return statements;
}

StmtPtr Parser::declaration() {
    if (match({TokenType::VAR})) return varDeclaration();
    return statement();
}

StmtPtr Parser::varDeclaration() {
    Token name = consume(TokenType::IDENTIFIER, "Expected variable name.");

    ExprPtr initializer = std::make_shared<LiteralExpr>(Value{std::monostate{}});
    if (match({TokenType::EQUAL})) initializer = expression();

    consume(TokenType::SEMICOLON, "Expected ';' after variable declaration.");
    return std::make_shared<VarStmt>(name, initializer);
}

StmtPtr Parser::statement() {
    if (match({TokenType::PRINT})) return printStatement();
    if (match({TokenType::IF})) return ifStatement();
    if (match({TokenType::WHILE})) return whileStatement();
    if (match({TokenType::LEFT_BRACE})) return block();
    return expressionStatement();
}

StmtPtr Parser::printStatement() {
    ExprPtr value = expression();
    consume(TokenType::SEMICOLON, "Expected ';' after value.");
    return std::make_shared<PrintStmt>(value);
}

StmtPtr Parser::ifStatement() {
    consume(TokenType::LEFT_PAREN, "Expected '(' after 'if'.");
    ExprPtr condition = expression();
    consume(TokenType::RIGHT_PAREN, "Expected ')' after if condition.");

    StmtPtr thenBranch = statement();
    StmtPtr elseBranch = nullptr;
    if (match({TokenType::ELSE})) elseBranch = statement();

    return std::make_shared<IfStmt>(condition, thenBranch, elseBranch);
}

StmtPtr Parser::whileStatement() {
    consume(TokenType::LEFT_PAREN, "Expected '(' after 'while'.");
    ExprPtr condition = expression();
    consume(TokenType::RIGHT_PAREN, "Expected ')' after condition.");

    return std::make_shared<WhileStmt>(condition, statement());
}

StmtPtr Parser::expressionStatement() {
    ExprPtr expr = expression();
    consume(TokenType::SEMICOLON, "Expected ';' after expression.");
    return std::make_shared<ExpressionStmt>(expr);
}

StmtPtr Parser::block() {
    std::vector<StmtPtr> statements;
    while (!check(TokenType::RIGHT_BRACE) && !isAtEnd()) {
        statements.push_back(declaration());
    }

    consume(TokenType::RIGHT_BRACE, "Expected '}' after block.");
    return std::make_shared<BlockStmt>(std::move(statements));
}

ExprPtr Parser::expression() {
    return assignment();
}

ExprPtr Parser::assignment() {
    ExprPtr expr = logicalOr();

    if (match({TokenType::EQUAL})) {
        Token equals = previous();
        ExprPtr value = assignment();

        if (auto variable = std::dynamic_pointer_cast<VariableExpr>(expr)) {
            return std::make_shared<AssignExpr>(variable->name, value);
        }

        throw ParseError("Invalid assignment target at line " +
                         std::to_string(equals.line));
    }

    return expr;
}

ExprPtr Parser::logicalOr() {
    ExprPtr expr = logicalAnd();
    while (match({TokenType::OR})) {
        Token op = previous();
        expr = std::make_shared<LogicalExpr>(expr, op, logicalAnd());
    }
    return expr;
}

ExprPtr Parser::logicalAnd() {
    ExprPtr expr = equality();
    while (match({TokenType::AND})) {
        Token op = previous();
        expr = std::make_shared<LogicalExpr>(expr, op, equality());
    }
    return expr;
}

ExprPtr Parser::equality() {
    ExprPtr expr = comparison();
    while (match({TokenType::BANG_EQUAL, TokenType::EQUAL_EQUAL})) {
        Token op = previous();
        expr = std::make_shared<BinaryExpr>(expr, op, comparison());
    }
    return expr;
}

ExprPtr Parser::comparison() {
    ExprPtr expr = term();
    while (match({TokenType::GREATER, TokenType::GREATER_EQUAL,
                  TokenType::LESS, TokenType::LESS_EQUAL})) {
        Token op = previous();
        expr = std::make_shared<BinaryExpr>(expr, op, term());
    }
    return expr;
}

ExprPtr Parser::term() {
    ExprPtr expr = factor();
    while (match({TokenType::MINUS, TokenType::PLUS})) {
        Token op = previous();
        expr = std::make_shared<BinaryExpr>(expr, op, factor());
    }
    return expr;
}

ExprPtr Parser::factor() {
    ExprPtr expr = unary();
    while (match({TokenType::SLASH, TokenType::STAR})) {
        Token op = previous();
        expr = std::make_shared<BinaryExpr>(expr, op, unary());
    }
    return expr;
}

ExprPtr Parser::unary() {
    if (match({TokenType::BANG, TokenType::MINUS})) {
        Token op = previous();
        return std::make_shared<UnaryExpr>(op, unary());
    }
    return primary();
}

ExprPtr Parser::primary() {
    if (match({TokenType::FALSE})) return std::make_shared<LiteralExpr>(Value{false});
    if (match({TokenType::TRUE})) return std::make_shared<LiteralExpr>(Value{true});
    if (match({TokenType::NIL})) return std::make_shared<LiteralExpr>(Value{std::monostate{}});

    if (match({TokenType::NUMBER})) {
        return std::make_shared<LiteralExpr>(
            Value{std::stod(previous().literal)});
    }

    if (match({TokenType::STRING})) {
        return std::make_shared<LiteralExpr>(Value{previous().literal});
    }

    if (match({TokenType::IDENTIFIER})) {
        return std::make_shared<VariableExpr>(previous());
    }

    if (match({TokenType::LEFT_PAREN})) {
        ExprPtr expr = expression();
        consume(TokenType::RIGHT_PAREN, "Expected ')' after expression.");
        return std::make_shared<GroupingExpr>(expr);
    }

    throw ParseError("Expected expression at line " +
                     std::to_string(peek().line));
}

bool Parser::match(std::initializer_list<TokenType> types) {
    for (TokenType type : types) {
        if (check(type)) {
            advance();
            return true;
        }
    }
    return false;
}

bool Parser::check(TokenType type) const {
    if (isAtEnd()) return type == TokenType::END_OF_FILE;
    return peek().type == type;
}

Token Parser::advance() {
    if (!isAtEnd()) ++current;
    return previous();
}

bool Parser::isAtEnd() const {
    return peek().type == TokenType::END_OF_FILE;
}

Token Parser::peek() const {
    return tokens[current];
}

Token Parser::previous() const {
    return tokens[current - 1];
}

Token Parser::consume(TokenType type, const std::string& message) {
    if (check(type)) return advance();

    throw ParseError(message + " Line " + std::to_string(peek().line));
}

void Parser::synchronize() {
    if (isAtEnd()) return;
    advance();

    while (!isAtEnd()) {
        if (previous().type == TokenType::SEMICOLON) return;

        switch (peek().type) {
            case TokenType::VAR:
            case TokenType::PRINT:
            case TokenType::IF:
            case TokenType::WHILE:
            case TokenType::LEFT_BRACE:
                return;
            default:
                advance();
        }
    }
}
