#pragma once
#include <string>
#include <vector>
#include "Token.h"

namespace corejson {

class Lexer {
public:
    // Constructor takes the JSON string to read
    Lexer(const std::string& source);

    // Main function to parse the whole text and return a list of tokens
    std::vector<Token> tokenize();

private:
    std::string m_source; // We will store the JSON text here
    size_t m_position;    // Our pointer to track where we are in the text

    // Helper functions
    Token lexString();
    Token lexNumber();
    Token lexKeyword();
};

} // namespace corejson