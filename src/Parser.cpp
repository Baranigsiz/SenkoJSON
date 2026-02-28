#include "corejson/Parser.h"
#include <string>

namespace corejson {

Parser::Parser(const std::vector<Token>& tokens) : m_tokens(tokens), m_position(0) {}

Token Parser::current() const {
    if (m_position >= m_tokens.size()) return {TokenType::EndOfFile, ""};
    return m_tokens[m_position];
}

void Parser::advance() {
    if (m_position < m_tokens.size()) m_position++;
}

JsonNode Parser::parse() {
    return parseValue();
}

JsonNode Parser::parseValue() {
    Token token = current();

    switch (token.type) {
        case TokenType::String:
            advance();
            return JsonNode(token.value);
        case TokenType::Number:
            advance();
            return JsonNode(std::stod(token.value)); // Convert string to double
        case TokenType::Boolean:
            advance();
            return JsonNode(token.value == "true");
        case TokenType::Null:
            advance();
            return JsonNode();
        case TokenType::CurlyOpen:
            return parseObject();
        case TokenType::SquareOpen:
            return parseArray();
        default:
            throw std::runtime_error("Unexpected token '" + token.value + "' at line " + std::to_string(token.line) + ", column " + std::to_string(token.column));
    }
}

JsonNode Parser::parseObject() {
    JsonNode node;
    node.type = JsonType::Object;
    
    advance(); // Skip '{'

    if (current().type == TokenType::CurlyClose) {
        advance();
        return node;
    }

    while (true) {
        // Get the key
        if (current().type != TokenType::String) {
            throw std::runtime_error("Object key must be a string at line " + std::to_string(current().line) + ", column " + std::to_string(current().column));
        }
        std::string key = current().value;
        advance();

        // Skip the colon
        if (current().type != TokenType::Colon) {
            throw std::runtime_error("Expected ':' after key '" + key + "' at line " + std::to_string(current().line) + ", column " + std::to_string(current().column));
        }
        advance();

        // Get the value (Recursion happens here)
        node.object_values[key] = parseValue();

        // After a value, we expect a comma OR a closing brace
        if (current().type == TokenType::Comma) {
            advance();
            // Allow trailing comma? Standard JSON says NO, but let's be classic and expect another key or brace.
            if (current().type == TokenType::CurlyClose) break; 
        } else if (current().type == TokenType::CurlyClose) {
            break;
        } else {
            throw std::runtime_error("Expected ',' or '}' after value at line " + std::to_string(current().line) + ", column " + std::to_string(current().column));
        }
    }
    
    if (current().type != TokenType::CurlyClose) {
        throw std::runtime_error("Expected '}' at line " + std::to_string(current().line) + ", column " + std::to_string(current().column));
    }

    advance(); // Skip '}'
    return node;
}

JsonNode Parser::parseArray() {
    JsonNode node;
    node.type = JsonType::Array;
    
    advance(); // Skip '['

    if (current().type == TokenType::SquareClose) {
        advance();
        return node;
    }

    while (true) {
        // Add value to the array
        node.array_values.push_back(parseValue());

        // After a value, we expect a comma OR a closing bracket
        if (current().type == TokenType::Comma) {
            advance();
            if (current().type == TokenType::SquareClose) break;
        } else if (current().type == TokenType::SquareClose) {
            break;
        } else {
            throw std::runtime_error("Expected ',' or ']' after array element at line " + std::to_string(current().line) + ", column " + std::to_string(current().column));
        }
    }
    
    if (current().type != TokenType::SquareClose) {
        throw std::runtime_error("Expected ']' at line " + std::to_string(current().line) + ", column " + std::to_string(current().column));
    }

    advance(); // Skip ']'
    return node;
}

} // namespace corejson