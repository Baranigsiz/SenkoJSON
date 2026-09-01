#include <iostream>
#include <senko/senko.hpp>

using json = senko::json;
using namespace senko::literals;

int main() {
    std::cout << "=== SenkoJSON Example 08: RFC 7396 JSON Merge Patch ===\n\n";

    // 1. Initial Document
    json user_profile = R"({
        "username": "senko_fan",
        "email": "senko@example.com",
        "settings": {
            "theme": "dark",
            "notifications": true,
            "font_size": 14
        },
        "tags": ["developer", "gamer"]
    })"_json;

    std::cout << "Original Profile:\n" << user_profile.dump(2) << "\n\n";

    // 2. RFC 7396 Merge Patch
    // - Setting a key updates or adds it
    // - Setting a key to null removes it
    // - Sub-objects are merged recursively
    json patch = R"({
        "email": "new_email@senko.org",
        "settings": {
            "font_size": 16,
            "notifications": null
        },
        "status": "online"
    })"_json;

    std::cout << "Merge Patch Document:\n" << patch.dump(2) << "\n\n";

    // 3. Apply Merge Patch in-place
    user_profile.merge_patch_in_place(patch);

    std::cout << "Updated Profile (after Merge Patch):\n" << user_profile.dump(2) << "\n\n";

    // 4. Comparison: RFC 6902 JSON Patch vs RFC 7396 Merge Patch
    std::cout << "Summary:\n";
    std::cout << " • RFC 6902: Array of operation objects (add, remove, replace, move, copy, test).\n";
    std::cout << " • RFC 7396: Single JSON document describing the desired delta (null to remove).\n";

    std::cout << "\n✔ Example 08 finished successfully!\n";
    return 0;
}
