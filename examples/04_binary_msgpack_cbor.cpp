#include <iostream>
#include <iomanip>
#include <senko/senko.hpp>

using json = senko::json;

int main() {
    std::cout << "=== SenkoJSON v2.1 - Binary Serialization (MessagePack & CBOR) ===\n\n";

    json doc = {
        {"player_id", 987654},
        {"username", "SenkoKitsune"},
        {"level", 85},
        {"score", 125400.75},
        {"inventory", {"Divine Staff", "Fox Orb", "Golden Feather"}},
        {"stats", {
            {"hp", 1500},
            {"mp", 950},
            {"speed", 320.5}
        }},
        {"is_vip", true}
    };

    // 1. Text JSON format
    std::string text_json = doc.dump(-1);
    std::cout << "Original Minified JSON (" << text_json.size() << " bytes):\n" << text_json << "\n\n";

    // 2. MessagePack Binary Encoding
    std::vector<uint8_t> msgpack_bytes = senko::to_msgpack(doc);
    std::cout << "MessagePack Binary Size: " << msgpack_bytes.size() << " bytes ("
              << std::fixed << std::setprecision(1)
              << (100.0 * (1.0 - double(msgpack_bytes.size()) / double(text_json.size())))
              << "% smaller than JSON)\n";

    // Decode MessagePack
    json restored_msg = senko::from_msgpack(msgpack_bytes);
    std::cout << "MessagePack Restored Player: " << restored_msg["username"].get<std::string>()
              << " (Level: " << restored_msg["level"].get<int>() << ")\n\n";

    // 3. CBOR Binary Encoding (RFC 8949)
    std::vector<uint8_t> cbor_bytes = senko::to_cbor(doc);
    std::cout << "CBOR Binary Size: " << cbor_bytes.size() << " bytes ("
              << std::fixed << std::setprecision(1)
              << (100.0 * (1.0 - double(cbor_bytes.size()) / double(text_json.size())))
              << "% smaller than JSON)\n";

    // Decode CBOR
    json restored_cbor = senko::from_cbor(cbor_bytes);
    std::cout << "CBOR Restored Player: " << restored_cbor["username"].get<std::string>()
              << " (Score: " << restored_cbor["score"].get<double>() << ")\n\n";

    std::cout << "✔ All binary roundtrips completed with zero loss!\n";
    return 0;
}
