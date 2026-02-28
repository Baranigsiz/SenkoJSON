#include "corejson/Lexer.h"
#include <cctype>

namespace corejson {

Lexer::Lexer(const std::string& source) : m_source(source), m_position(0), m_line(1), m_column(1) {}

void Lexer::skipWhitespace() {
    while (m_position < m_source.length()) {
        char current = m_source[m_position];
        if (current == '\n') {
            m_line++;
            m_column = 1;
            m_position++;
        } else if (std::isspace(current)) {
            m_column++;
            m_position++;
        } else {
            break;
        }
    }
}

std::vector<Token> Lexer::tokenize() {
    std::vector<Token> tokens;

    while (m_position < m_source.length()) {
        skipWhitespace();
        if (m_position >= m_source.length()) break;

        char current = m_source[m_position];
        int startLine = m_line;
        int startColumn = m_column;

        if (current == '{') { tokens.push_back({TokenType::CurlyOpen, "{", startLine, startColumn}); m_position++; m_column++; }
        else if (current == '}') { tokens.push_back({TokenType::CurlyClose, "}", startLine, startColumn}); m_position++; m_column++; }
        else if (current == '[') { tokens.push_back({TokenType::SquareOpen, "[", startLine, startColumn}); m_position++; m_column++; }
        else if (current == ']') { tokens.push_back({TokenType::SquareClose, "]", startLine, startColumn}); m_position++; m_column++; }
        else if (current == ':') { tokens.push_back({TokenType::Colon, ":", startLine, startColumn}); m_position++; m_column++; }
        else if (current == ',') { tokens.push_back({TokenType::Comma, ",", startLine, startColumn}); m_position++; m_column++; }
        else if (current == '"') tokens.push_back(lexString());
        else if (std::isdigit(current) || current == '-') tokens.push_back(lexNumber());
        else if (std::isalpha(current)) tokens.push_back(lexKeyword());
        else {
            m_position++; m_column++;
        }
    }
    
    tokens.push_back({TokenType::EndOfFile, "", m_line, m_column});
    return tokens;
}

Token Lexer::lexString() {
    int startLine = m_line;
    int startColumn = m_column;
    m_position++; m_column++; // Skip opening "
    
    std::string result = "";
    while (m_position < m_source.length() && m_source[m_position] != '"') {
        if (m_source[m_position] == '\n') {
            m_line++;
            m_column = 1;
        } else {
            m_column++;
        }
        result += m_source[m_position];
        m_position++;
    }
    
    if (m_position < m_source.length()) {
        m_position++; m_column++; // Skip closing "
    }
    
    return {TokenType::String, result, startLine, startColumn};
}

Token Lexer::lexNumber() {
    int startLine = m_line;
    int startColumn = m_column;
    std::string result = "";
    while (m_position < m_source.length() && 
           (std::isdigit(m_source[m_position]) || m_source[m_position] == '.' || 
            m_source[m_position] == '-' || m_source[m_position] == 'e' || m_source[m_position] == 'E')) {
        result += m_source[m_position];
        m_position++;
        m_column++;
    }
    return {TokenType::Number, result, startLine, startColumn};
}

Token Lexer::lexKeyword() {
    int startLine = m_line;
    int startColumn = m_column;
    std::string result = "";
    while (m_position < m_source.length() && std::isalpha(m_source[m_position])) {
        result += m_source[m_position];
        m_position++;
        m_column++;
    }
    
    if (result == "true" || result == "false") {
        return {TokenType::Boolean, result, startLine, startColumn};
    } else if (result == "null") {
        return {TokenType::Null, result, startLine, startColumn};
    }
    
    return {TokenType::String, result, startLine, startColumn}; 
}

} // namespace corejson