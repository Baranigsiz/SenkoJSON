#pragma once
#include <string>

namespace corejson {

// All meaningful parts we can encounter in JSON
enum class TokenType {
    String,       // "name"
    Number,       // 42 or 3.14
    Boolean,      // true or false
    Null,         // null
    CurlyOpen,    // {
    CurlyClose,   // }
    SquareOpen,   // [
    SquareClose,  // ]
    Colon,        // :
    Comma,        // ,
    EndOfFile     // Used when the text ends
};

// Structure to hold the type and value of the token
struct Token {
    TokenType type;
    std::string value; 
};

} // namespace corejson