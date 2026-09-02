#include <iostream>
#include <senko/senko.hpp>

using json = senko::json;
using namespace senko::literals;

int main() {
    std::cout << "=== SenkoJSON - JSONC Configuration Demo ===\n\n";

    // 1. JSON with C/C++ single-line and multi-line comments & trailing commas
    std::string config_content = R"({
        // Server Network Configuration
        "server": {
            "host": "0.0.0.0",
            "port": 8080, /* Default HTTP port */
            "max_connections": 10000,
        },
        // Enabled middleware modules:
        "modules": [
            "cors",
            "rate_limiter",
            "compression", // Trailing comma supported!
        ],
        /* Developer debug mode settings */
        "debug": true,
    })";

    // Parse using senko::jsonc::parse
    json cfg = senko::jsonc::parse(config_content);

    std::cout << "Server Host: " << cfg["server"]["host"].get<std::string>() << "\n";
    std::cout << "Server Port: " << cfg["server"]["port"].get<int>() << "\n";
    std::cout << "Debug Mode:  " << (cfg["debug"].get<bool>() ? "Enabled" : "Disabled") << "\n";
    std::cout << "Modules Count: " << cfg["modules"].size() << "\n\n";

    // 2. Direct user-defined literal ""_jsonc
    auto fast_cfg = R"({
        // Quick config snippet
        "app_name": "GameEngine",
        "fps_limit": 144,
    })"_jsonc;

    std::cout << "App Name: " << fast_cfg["app_name"].get<std::string>() << "\n";
    std::cout << "FPS Limit: " << fast_cfg["fps_limit"].get<int>() << "\n";

    return 0;
}
