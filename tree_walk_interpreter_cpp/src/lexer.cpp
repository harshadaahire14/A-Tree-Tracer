#include "lexer.h"
#include <cctype>
#include <unordered_map>
#include <stdexcept>

Lexer::Lexer(const std::string& source) : source(source) {}

bool Lexer::isAtEnd() const {
    return current >= source.size();
}

char Lexer::advance() {
    return source[current++];
}

char Lexer::peek() const {
    return isAtEnd() ? '\0' : source[current];
}

char Lexer::peekNext() const {
    return current + 1 >= source.size() ? '\0' : source[current + 1];
}

bool Lexer::match(char expected) {
    if (isAtEnd() || source[current] != expected) return false;
    ++current;
    return true;
}

void Lexer::addToken(TokenType type) {
    addToken(type, "");
}

void Lexer::addToken(TokenType type, const std::string& literal) {
    tokens.emplace_back(type, source.substr(start, current - start), literal, line);
}

void Lexer::stringLiteral() {
    while (peek() != '"' && !isAtEnd()) {
        if (peek() == '\n') ++line;
        advance();
    }

    if (isAtEnd()) {
        throw std::runtime_error("Unterminated string at line " + std::to_string(line));
    }

    advance(); // closing quote
    std::string value = source.substr(start + 1, current - start - 2);
    addToken(TokenType::STRING, value);
}

void Lexer::number() {
    while (std::isdigit(static_cast<unsigned char>(peek()))) advance();

    if (peek() == '.' && std::isdigit(static_cast<unsigned char>(peekNext()))) {
        advance();
        while (std::isdigit(static_cast<unsigned char>(peek()))) advance();
    }

    addToken(TokenType::NUMBER, source.substr(start, current - start));
}

void Lexer::identifier() {
    while (std::isalnum(static_cast<unsigned char>(peek())) || peek() == '_') {
        advance();
    }

    static const std::unordered_map<std::string, TokenType> keywords = {
        {"var", TokenType::VAR}, {"print", TokenType::PRINT},
        {"if", TokenType::IF}, {"else", TokenType::ELSE},
        {"while", TokenType::WHILE}, {"true", TokenType::TRUE},
        {"false", TokenType::FALSE}, {"nil", TokenType::NIL},
        {"and", TokenType::AND}, {"or", TokenType::OR}
    };

    std::string text = source.substr(start, current - start);
    auto it = keywords.find(text);
    addToken(it == keywords.end() ? TokenType::IDENTIFIER : it->second);
}

std::vector<Token> Lexer::scanTokens() {
    while (!isAtEnd()) {
        start = current;
        char c = advance();

        switch (c) {
            case '(': addToken(TokenType::LEFT_PAREN); break;
            case ')': addToken(TokenType::RIGHT_PAREN); break;
            case '{': addToken(TokenType::LEFT_BRACE); break;
            case '}': addToken(TokenType::RIGHT_BRACE); break;
            case ',': addToken(TokenType::COMMA); break;
            case ';': addToken(TokenType::SEMICOLON); break;
            case '+': addToken(TokenType::PLUS); break;
            case '-': addToken(TokenType::MINUS); break;
            case '*': addToken(TokenType::STAR); break;
            case '!': addToken(match('=') ? TokenType::BANG_EQUAL : TokenType::BANG); break;
            case '=': addToken(match('=') ? TokenType::EQUAL_EQUAL : TokenType::EQUAL); break;
            case '<': addToken(match('=') ? TokenType::LESS_EQUAL : TokenType::LESS); break;
            case '>': addToken(match('=') ? TokenType::GREATER_EQUAL : TokenType::GREATER); break;
            case '/':
                if (match('/')) {
                    while (peek() != '\n' && !isAtEnd()) advance();
                } else {
                    addToken(TokenType::SLASH);
                }
                break;
            case ' ':
            case '\r':
            case '\t':
                break;
            case '\n':
                ++line;
                break;
            case '"':
                stringLiteral();
                break;
            default:
                if (std::isdigit(static_cast<unsigned char>(c))) {
                    number();
                } else if (std::isalpha(static_cast<unsigned char>(c)) || c == '_') {
                    identifier();
                } else {
                    throw std::runtime_error(
                        "Unexpected character '" + std::string(1, c) +
                        "' at line " + std::to_string(line));
                }
        }
    }

    tokens.emplace_back(TokenType::END_OF_FILE, "", "", line);
    return tokens;
}
