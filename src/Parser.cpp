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
            throw std::runtime_error("Unexpected JSON format!");
    }
}

JsonNode Parser::parseObject() {
    JsonNode node;
    node.type = JsonType::Object;
    
    advance(); // Skip '{'

    while (current().type != TokenType::CurlyClose && current().type != TokenType::EndOfFile) {
        // Get the key
        if (current().type != TokenType::String) throw std::runtime_error("Object key must be a string!");
        std::string key = current().value;
        advance();

        // Skip the colon
        if (current().type != TokenType::Colon) throw std::runtime_error("Expected ':' after key!");
        advance();

        // Get the value (Recursion happens here)
        node.object_values[key] = parseValue();

        // Skip comma if it exists
        if (current().type == TokenType::Comma) {
            advance();
        }
    }
    
    advance(); // Skip '}'
    return node;
}

JsonNode Parser::parseArray() {
    JsonNode node;
    node.type = JsonType::Array;
    
    advance(); // Skip '['

    while (current().type != TokenType::SquareClose && current().type != TokenType::EndOfFile) {
        // Add value to the array
        node.array_values.push_back(parseValue());

        // Skip comma if it exists
        if (current().type == TokenType::Comma) {
            advance();
        }
    }
    
    advance(); // Skip ']'
    return node;
}

} // namespace corejson