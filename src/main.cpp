#include <iostream>
#include <senko/senko.hpp>

using json = senko::json;
using namespace senko::literals;

int main() {
    std::cout << "====================================================\n";
    std::cout << "         ⚡ SenkoJSON (閃光) Showcase Demo          \n";
    std::cout << "====================================================\n\n";

    // 1. Parse JSON Literal
    json doc = R"({
        "project": "SenkoJSON",
        "version": "2.2.0",
        "speed": "Ultra-Fast",
        "stars": 1500,
        "features": ["JSONPath", "JSON Patch", "MessagePack", "CBOR", "Iterators"],
        "server": {
            "host": "127.0.0.1",
            "port": 8080,
            "ssl": true
        }
    })"_json;

    std::cout << "[1] Loaded Document:\n" << doc.dump(2) << "\n\n";

    // 2. Structured Binding & Iterators
    std::cout << "[2] Server Config Entries:\n";
    for (const auto& [key, val] : doc["server"].items()) {
        std::cout << "  • " << key << ": " << val.dump() << "\n";
    }

    std::cout << "\n[3] Features Array:\n";
    for (const auto& feature : doc["features"]) {
        std::cout << "  ✔ " << feature.get<std::string>() << "\n";
    }

    // 3. JSONPath Query
    std::cout << "\n[4] JSONPath Query ($.features[0:3]):\n";
    auto top3_features = doc.jsonpath("$.features[0:3]");
    for (const auto& f : top3_features) {
        std::cout << "  -> " << f.get<std::string>() << "\n";
    }

    // 4. RFC 7396 Merge Patch
    std::cout << "\n[5] Applying RFC 7396 Merge Patch:\n";
    json patch = R"({
        "stars": 2000,
        "server": {
            "port": 9000,
            "ssl": null
        }
    })"_json;
    doc.merge_patch_in_place(patch);
    std::cout << doc.dump(2) << "\n\n";

    // 5. Binary Formats (MessagePack & CBOR)
    auto msgpack_data = senko::to_msgpack(doc);
    auto cbor_data = senko::to_cbor(doc);
    std::string json_str = doc.dump(-1);

    std::cout << "[6] Payload Size Comparison:\n";
    std::cout << "  • Raw JSON Size:     " << json_str.size() << " bytes\n";
    std::cout << "  • MessagePack Size:  " << msgpack_data.size() << " bytes\n";
    std::cout << "  • CBOR Size:         " << cbor_data.size() << " bytes\n";

    std::cout << "\n✔ SenkoJSON Demo Finished Successfully!\n";
    return 0;
}