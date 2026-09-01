#include <iostream>
#include <senko/senko.hpp>

using json = senko::json;
using namespace senko::literals;

int main() {
    std::cout << "=== SenkoJSON v2.0 - Basic Usage Demo ===\n\n";

    // 1. Parsing from raw string
    std::string_view raw = R"({
        "project": "SenkoJSON",
        "stars": 1250,
        "is_awesome": true,
        "tags": ["c++17", "header-only", "fast"],
        "author": {
            "name": "Baran",
            "github": "Baranigsiz"
        }
    })";

    json doc = json::parse(raw);

    std::cout << "Project: " << doc["project"].get<std::string>() << "\n";
    std::cout << "Stars: " << doc["stars"].get<int>() << "\n";
    std::cout << "Author: " << doc["author"]["name"].get<std::string>() << "\n";

    // 2. Modifying and adding values
    doc["stars"] = doc["stars"].get<int>() + 500;
    doc["version"] = "2.0.0";
    doc["tags"].push_back("zero-allocation");

    // 3. Using value_or for safe fallback access
    std::string license = doc.value_or("license", std::string("MIT"));
    std::cout << "License (with default fallback): " << license << "\n\n";

    // 4. Using User-Defined Literal
    json quick = "{\"status\": \"ok\", \"code\": 200}"_json;
    std::cout << "Quick literal parsed: " << quick["status"].get<std::string>() << "\n\n";

    // 5. Pretty Print
    std::cout << "--- Formatted Output (Indent 2) ---\n";
    std::cout << doc.dump(2) << "\n\n";

    // 6. Minified Output
    std::cout << "--- Minified Output ---\n";
    std::cout << doc.dump(-1) << "\n";

    return 0;
}
