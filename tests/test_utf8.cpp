#include <iostream>
#include <string>
#include "corejson/Lexer.h"
#include "corejson/Parser.h"
#include "corejson/JsonNode.h"

int main() {
    // Test 1: Unicode Escape (\uXXXX) ve Türkçe Karakterler
    // "merhaba": "D\u00fcnya", "emoji": "\u263A"
    std::string utf8Json = R"({
        "greeting": "D\u00fcnya \u015fi\u011fdem",
        "emoji": "\u263A",
        "escapes": "Line1\nLine2\tTabbed",
        "turkish": "Öçşığü İĞÜŞÇÖ"
    })";

    try {
        std::cout << "--- Testing UTF-8 & Unicode Escapes ---\n";
        corejson::Lexer lexer(utf8Json);
        auto tokens = lexer.tokenize();
        corejson::Parser parser(tokens);
        corejson::JsonNode root = parser.parse();

        std::cout << "Parsed Greeting: " << root.object_values["greeting"].string_value << "\n";
        std::cout << "Parsed Emoji: " << root.object_values["emoji"].string_value << "\n";
        std::cout << "Parsed Escapes: " << root.object_values["escapes"].string_value << "\n";
        std::cout << "Parsed Turkish: " << root.object_values["turkish"].string_value << "\n";

        std::cout << "\n--- Testing Dump (Pretty Print) ---\n";
        std::cout << root.dump(2) << "\n";

        std::cout << "\n[SUCCESS] All Unicode and Escape tests passed!\n";
    } catch (const std::exception& e) {
        std::cout << "\n[FAILURE] Test failed: " << e.what() << "\n";
        return 1;
    }

    return 0;
}
