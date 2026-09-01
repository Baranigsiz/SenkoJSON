#include "test_framework.hpp"
#include <senko/senko.hpp>

using json = senko::json;

TEST_CASE("Binary - MessagePack Roundtrip") {
    json doc = {
        {"name", "Senko"},
        {"age", 800},
        {"is_fox", true},
        {"rating", 9.95},
        {"tags", {"helpful", "kitsune", "fluffy"}},
        {"extra", nullptr}
    };

    // Encode to MessagePack
    std::vector<uint8_t> packed = senko::to_msgpack(doc);
    CHECK(!packed.empty());

    // Decode back from MessagePack
    json restored = senko::from_msgpack(packed);
    CHECK(restored.is_object());
    CHECK_EQ(restored["name"].get<std::string>(), "Senko");
    CHECK_EQ(restored["age"].get<int>(), 800);
    CHECK_EQ(restored["is_fox"].get<bool>(), true);
    CHECK(std::abs(restored["rating"].get<double>() - 9.95) < 0.01);
    CHECK_EQ(restored["tags"].size(), 3);
    CHECK_EQ(restored["tags"][1].get<std::string>(), "kitsune");
    CHECK(restored["extra"].is_null());
}

TEST_CASE("Binary - CBOR Roundtrip (RFC 8949)") {
    json doc = {
        {"server", "tokyo-01"},
        {"port", 443},
        {"ssl", true},
        {"ping_ms", 12.4},
        {"users", {1001, 1002, 1003}},
        {"meta", {
            {"region", "ap-northeast-1"},
            {"active", true}
        }}
    };

    // Encode to CBOR
    std::vector<uint8_t> cbor_data = senko::to_cbor(doc);
    CHECK(!cbor_data.empty());

    // Decode back from CBOR
    json restored = senko::from_cbor(cbor_data);
    CHECK(restored.is_object());
    CHECK_EQ(restored["server"].get<std::string>(), "tokyo-01");
    CHECK_EQ(restored["port"].get<int>(), 443);
    CHECK_EQ(restored["ssl"].get<bool>(), true);
    CHECK(std::abs(restored["ping_ms"].get<double>() - 12.4) < 0.01);
    CHECK_EQ(restored["users"].size(), 3);
    CHECK_EQ(restored["meta"]["region"].get<std::string>(), "ap-northeast-1");
}

TEST_CASE("Binary - Negative & Boundary Integers") {
    json doc = {
        {"neg_small", -15},
        {"neg_med", -1000},
        {"neg_large", -9876543210LL},
        {"u_large", 18446744073709551615ULL}
    };

    // MessagePack
    auto msg_bytes = senko::to_msgpack(doc);
    auto msg_res = senko::from_msgpack(msg_bytes);
    CHECK_EQ(msg_res["neg_small"].get<int>(), -15);
    CHECK_EQ(msg_res["neg_med"].get<int>(), -1000);
    CHECK_EQ(msg_res["neg_large"].get<int64_t>(), -9876543210LL);

    // CBOR
    auto cbor_bytes = senko::to_cbor(doc);
    auto cbor_res = senko::from_cbor(cbor_bytes);
    CHECK_EQ(cbor_res["neg_small"].get<int>(), -15);
    CHECK_EQ(cbor_res["neg_med"].get<int>(), -1000);
    CHECK_EQ(cbor_res["neg_large"].get<int64_t>(), -9876543210LL);
}
