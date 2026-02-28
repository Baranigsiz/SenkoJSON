#include <iostream>
#include <fstream>
#include <sstream>
#include "corejson/Lexer.h"
#include "corejson/Parser.h"

int main() {
    std::string inputFile = "test_malformed.json";

    std::ifstream fileIn(inputFile);
    if (!fileIn.is_open()) {
        std::cout << "[ERROR] Could not open " << inputFile << "!\n";
        return 1;
    }
    
    std::stringstream buffer;
    buffer << fileIn.rdbuf();
    std::string jsonText = buffer.str();
    fileIn.close();

    try {
        corejson::Lexer lexer(jsonText);
        std::vector<corejson::Token> tokens = lexer.tokenize();
        corejson::Parser parser(tokens);
        corejson::JsonNode root = parser.parse();
        std::cout << "[SUCCESS] Parsed correctly!\n";
    } catch (const std::exception& e) {
        std::cout << "[EXPECTED ERROR] " << e.what() << "\n";
    }

    return 0;
}
