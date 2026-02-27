#include "corejson/Lexer.h"
#include <cctype>

namespace corejson {

Lexer::Lexer(const std::string& source) : m_source(source), m_position(0) {}

std::vector<Token> Lexer::tokenize() {
    std::vector<Token> tokens;

    while (m_position < m_source.length()) {
        char current = m_source[m_position];

        // 1. Ignore spaces and line breaks
        if (std::isspace(current)) {
            m_position++;
            continue;
        }

        // 2. Catch single-character symbols
        if (current == '{') { tokens.push_back({TokenType::CurlyOpen, "{"}); m_position++; }
        else if (current == '}') { tokens.push_back({TokenType::CurlyClose, "}"}); m_position++; }
        else if (current == '[') { tokens.push_back({TokenType::SquareOpen, "["}); m_position++; }
        else if (current == ']') { tokens.push_back({TokenType::SquareClose, "]"}); m_position++; }
        else if (current == ':') { tokens.push_back({TokenType::Colon, ":"}); m_position++; }
        else if (current == ',') { tokens.push_back({TokenType::Comma, ","}); m_position++; }
        
        // 3. If you see a double quote, it's a String!
        else if (current == '"') {
            tokens.push_back(lexString());
        }
        // 4. If you see a digit or minus sign, start reading a number
        else if (std::isdigit(current) || current == '-') {
            tokens.push_back(lexNumber());
        }
        // 5. If you see a letter, start reading a keyword (true/false/null)
        else if (std::isalpha(current)) {
            tokens.push_back(lexKeyword());
        }
        else {
            m_position++; // Safely skip unknown characters
        }
    }
    
    // Add End Of File marker
    tokens.push_back({TokenType::EndOfFile, ""});
    return tokens;
}

Token Lexer::lexString() {
    m_position++;
    std::string result = "";
    while (m_position < m_source.length() && m_source[m_position] != '"') {
        result += m_source[m_position];
        m_position++;
    }
    m_position++;
    return {TokenType::String, result};
}

Token Lexer::lexNumber() {
    std::string result = "";
    while (m_position < m_source.length() && 
           (std::isdigit(m_source[m_position]) || m_source[m_position] == '.' || 
            m_source[m_position] == '-' || m_source[m_position] == 'e' || m_source[m_position] == 'E')) {
        result += m_source[m_position];
        m_position++;
    }
    return {TokenType::Number, result};
}

Token Lexer::lexKeyword() {
    std::string result = "";
    while (m_position < m_source.length() && std::isalpha(m_source[m_position])) {
        result += m_source[m_position];
        m_position++;
    }
    
    if (result == "true" || result == "false") {
        return {TokenType::Boolean, result};
    } else if (result == "null") {
        return {TokenType::Null, result};
    }
    
    return {TokenType::String, result}; // Fallback
}

} // namespace corejson