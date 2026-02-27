#include <iostream>
#include <fstream>
#include <sstream>
#include "corejson/Lexer.h"
#include "corejson/Parser.h"

int main() {
    std::string inputFile = "apex_config.json";
    std::string outputFile = "optimized_config.json";

    // 1. READ FROM FILE (Dosyadan Okuma)
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
        // 2. PARSE (Okunan metni C++ ağacına çevir)
        std::cout << "[INFO] Parsing configuration file...\n";
        corejson::Lexer lexer(jsonText);
        std::vector<corejson::Token> tokens = lexer.tokenize();
        corejson::Parser parser(tokens);
        corejson::JsonNode root = parser.parse();

        // 3. MODIFY DATA IN RAM (Ağaçtaki verileri C++ ile değiştir)
        std::cout << "[INFO] Optimizing game settings...\n";
        root.object_values["performance"].object_values["max_fps"] = corejson::JsonNode(240.0);
        root.object_values["performance"].object_values["reduce_lag"] = corejson::JsonNode(true);

        // 4. WRITE TO NEW FILE (Güncel ağacı metne çevirip yeni dosyaya kaydet)
        std::cout << "[INFO] Saving optimized configuration to " << outputFile << "...\n";
        std::ofstream fileOut(outputFile);
        if (fileOut.is_open()) {
            fileOut << root.dump(4); // 4 spaces indentation
            fileOut.close();
            std::cout << "[SUCCESS] Process completed!\n";
        }

    } catch (const std::exception& e) {
        std::cout << "[ERROR] " << e.what() << "\n";
    }

    return 0;
}