#pragma once
#include <string>

enum class TokenType {
    // Single-character
    LEFT_PAREN, RIGHT_PAREN,
    LEFT_BRACE, RIGHT_BRACE,
    COMMA, SEMICOLON,

    // One/two-character operators
    PLUS, MINUS, STAR, SLASH,
    BANG, BANG_EQUAL,
    EQUAL, EQUAL_EQUAL,
    GREATER, GREATER_EQUAL,
    LESS, LESS_EQUAL,

    // Literals
    IDENTIFIER, STRING, NUMBER,

    // Keywords
    VAR, PRINT, IF, ELSE, WHILE,
    TRUE, FALSE, NIL,
    AND, OR,

    END_OF_FILE
};

struct Token {
    TokenType type;
    std::string lexeme;
    std::string literal;
    int line;

    Token(TokenType t, std::string l, std::string lit, int ln)
        : type(t), lexeme(std::move(l)), literal(std::move(lit)), line(ln) {}
};

std::string tokenTypeName(TokenType type);
