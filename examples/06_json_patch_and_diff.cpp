#include <iostream>
#include <optional>
#include <map>
#include <vector>
#include <senko/senko.hpp>

using json = senko::json;

struct UserSettings {
    std::string theme;
    std::optional<std::string> avatar_url;
    std::vector<std::string> shortcuts;
    std::map<std::string, int> volume_levels;
};
SENKO_BIND(UserSettings, theme, avatar_url, shortcuts, volume_levels)

int main() {
    std::cout << "====================================================\n";
    std::cout << "  SenkoJSON - STL Adapters & RFC 6902 Patch / Diff  \n";
    std::cout << "====================================================\n\n";

    // 1. STL Adapters Demo (std::optional, std::map, std::vector)
    std::cout << "--- 1. STL Containers & std::optional Serialization ---\n";
    UserSettings settings{
        "dark",
        std::nullopt, // No avatar set
        {"Ctrl+S", "Ctrl+P"},
        {{"master", 80}, {"sfx", 100}}
    };

    json j_settings = settings;
    std::cout << "Serialized Settings:\n" << j_settings.dump(2) << "\n\n";

    // 2. RFC 6902 JSON Diff & Patch Demo
    std::cout << "--- 2. RFC 6902 JSON Diff & Patch ---\n";
    json original_config = {
        {"service", "PaymentGateway"},
        {"version", "1.0.0"},
        {"maintenance_mode", false},
        {"endpoints", {"/pay", "/refund"}},
        {"timeout_ms", 5000}
    };

    json target_config = {
        {"service", "PaymentGateway"},
        {"version", "1.1.0"}, // updated
        {"endpoints", {"/pay", "/refund", "/webhook"}}, // added endpoint
        {"timeout_ms", 3000}, // updated
        {"rate_limit", 1000} // new key
    };

    std::cout << "Original Config:\n" << original_config.dump(2) << "\n\n";
    std::cout << "Target Config:\n" << target_config.dump(2) << "\n\n";

    // Generate diff
    json patch = json::diff(original_config, target_config);
    std::cout << "Generated RFC 6902 Patch (Delta):\n" << patch.dump(2) << "\n\n";

    // Apply patch
    json patched_config = original_config.patch(patch);
    std::cout << "Patched Result matches Target? " 
              << (patched_config == target_config ? "YES (100% Match!)" : "NO") << "\n";

    return 0;
}
