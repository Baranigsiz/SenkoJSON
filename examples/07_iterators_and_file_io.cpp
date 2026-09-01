#include <iostream>
#include <senko/senko.hpp>
#include <cstdio>

using json = senko::json;
using namespace senko::literals;

int main() {
    std::cout << "=== SenkoJSON Example 07: Iterators & File I/O ===\n\n";

    // 1. Range-based for loop over arrays
    json tags = {"c++17", "header-only", "zero-dependency", "ultra-fast"};
    std::cout << "1. Iterating array with range-based for:\n";
    for (const auto& tag : tags) {
        std::cout << "   - " << tag.get<std::string>() << "\n";
    }

    // 2. Structured binding iteration (.items()) over objects
    json database_config = {
        {"host", "localhost"},
        {"port", 5432},
        {"user", "admin"},
        {"pool_size", 20},
        {"ssl", true}
    };

    std::cout << "\n2. Iterating object key-value pairs with .items():\n";
    for (const auto& [key, val] : database_config.items()) {
        std::cout << "   • " << key << " => " << val.dump() << "\n";
    }

    // 3. Mutating items during iteration
    std::cout << "\n3. In-place modification during iteration:\n";
    for (auto& [key, val] : database_config.items()) {
        if (key == "port") {
            val = 5433; // changed port
        }
    }
    std::cout << "   Updated port: " << database_config["port"].get<int>() << "\n";

    // 4. File I/O: dump_file and parse_file
    const std::string config_file = "app_config_temp.json";
    std::cout << "\n4. Saving JSON directly to file '" << config_file << "'...\n";
    database_config.dump_file(config_file, 4);

    std::cout << "   Loading JSON back from file...\n";
    json loaded = json::parse_file(config_file);
    std::cout << "   Loaded host: " << loaded["host"].get<std::string>() << "\n";

    // Clean up
    std::remove(config_file.c_str());

    std::cout << "\n✔ Example 07 finished successfully!\n";
    return 0;
}
