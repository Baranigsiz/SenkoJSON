#include "corejson/Lexer.h"
#include <cctype>

namespace corejson {

Lexer::Lexer(std::string_view source) : m_source(source), m_position(0), m_line(1), m_column(1) {}

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
        if (m_source[m_position] == '\\' && m_position + 1 < m_source.length()) {
            char next = m_source[m_position + 1];
            if (next == 'u' && m_position + 5 < m_source.length()) {
                // Handle \uXXXX escape sequences
                try {
                    std::string hexStr(m_source.substr(m_position + 2, 4));
                    uint32_t codepoint = std::stoul(hexStr, nullptr, 16);
                    
                    // Simple UTF-8 encoding for Unicode codepoints
                    if (codepoint <= 0x7F) {
                        result += static_cast<char>(codepoint);
                    } else if (codepoint <= 0x7FF) {
                        result += static_cast<char>(0xC0 | (codepoint >> 6));
                        result += static_cast<char>(0x80 | (codepoint & 0x3F));
                    } else {
                        result += static_cast<char>(0xE0 | (codepoint >> 12));
                        result += static_cast<char>(0x80 | ((codepoint >> 6) & 0x3F));
                        result += static_cast<char>(0x80 | (codepoint & 0x3F));
                    }
                    m_position += 6;
                    m_column += 6;
                    continue;
                } catch (...) {
                    // Fallback to literal if hex is invalid
                }
            } else {
                // Handle basic escapes like \", \\, \/
                switch (next) {
                    case '"': result += '"'; break;
                    case '\\': result += '\\'; break;
                    case '/': result += '/'; break;
                    case 'b': result += '\b'; break;
                    case 'f': result += '\f'; break;
                    case 'n': result += '\n'; break;
                    case 'r': result += '\r'; break;
                    case 't': result += '\t'; break;
                    default: result += next; break;
                }
                m_position += 2;
                m_column += 2;
                continue;
            }
        }

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
    size_t startPos = m_position;
    while (m_position < m_source.length() && 
           (std::isdigit(m_source[m_position]) || m_source[m_position] == '.' || 
            m_source[m_position] == '-' || m_source[m_position] == 'e' || m_source[m_position] == 'E')) {
        m_position++;
        m_column++;
    }
    std::string_view value = m_source.substr(startPos, m_position - startPos);
    return {TokenType::Number, std::string(value), startLine, startColumn};
}

Token Lexer::lexKeyword() {
    int startLine = m_line;
    int startColumn = m_column;
    size_t startPos = m_position;
    while (m_position < m_source.length() && std::isalpha(m_source[m_position])) {
        m_position++;
        m_column++;
    }
    
    std::string_view value = m_source.substr(startPos, m_position - startPos);
    
    if (value == "true" || value == "false") {
        return {TokenType::Boolean, std::string(value), startLine, startColumn};
    } else if (value == "null") {
        return {TokenType::Null, std::string(value), startLine, startColumn};
    }
    
    return {TokenType::String, std::string(value), startLine, startColumn}; 
}

} // namespace corejson