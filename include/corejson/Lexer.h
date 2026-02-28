#pragma once
#include <string>
#include <string_view> // NEW: For zero-copy string handling
#include <vector>
#include "Token.h"

namespace corejson {

class Lexer {
public:
    // Constructor now takes string_view
    Lexer(std::string_view source);

    // Main function to parse the whole text and return a list of tokens
    std::vector<Token> tokenize();

private:
    std::string_view m_source; // Optimization: Using view instead of copy
    size_t m_position;    // Our pointer to track where we are in the text
    int m_line;           // TRACKING: Current line
    int m_column;         // TRACKING: Current column
    
    // Helper functions
    Token lexString();
    Token lexNumber();
    Token lexKeyword();
    void skipWhitespace(); // Added to centralize whitespace and line-break tracking
};

} // namespace corejson