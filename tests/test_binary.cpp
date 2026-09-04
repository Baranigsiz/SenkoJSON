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

TEST_CASE("Binary - DoS and Oversized Length Protection") {
    // Malicious MessagePack payload claiming 4GB array length in a 5-byte buffer (0xDD = array 32)
    std::vector<uint8_t> malicious_msgpack = {0xDD, 0xFF, 0xFF, 0xFF, 0xFF};
    CHECK_THROWS(senko::from_msgpack(malicious_msgpack));

    // Malicious CBOR payload claiming 4GB array length in a 5-byte buffer (0x9A = array 32-bit length)
    std::vector<uint8_t> malicious_cbor = {0x9A, 0xFF, 0xFF, 0xFF, 0xFF};
    CHECK_THROWS(senko::from_cbor(malicious_cbor));

    // Deep nesting stack overflow protection in CBOR (600 nested arrays of 1 element)
    std::vector<uint8_t> deep_cbor(600, 0x81); // 0x81 = array of length 1
    deep_cbor.push_back(0x00); // inner integer 0
    CHECK_THROWS(senko::from_cbor(deep_cbor));

    // Deep nesting stack overflow protection in MessagePack (600 nested arrays of 1 element)
    std::vector<uint8_t> deep_msgpack(600, 0x91); // 0x91 = fixarray of length 1
    deep_msgpack.push_back(0x00); // inner integer 0
    CHECK_THROWS(senko::from_msgpack(deep_msgpack));
}

TEST_CASE("Binary - MessagePack Bin Data Types (bin 8, bin 16)") {
    // 0xC4 = bin 8, len = 5, followed by "hello"
    std::vector<uint8_t> bin8_data = {0xC4, 5, 'h', 'e', 'l', 'l', 'o'};
    json j8 = senko::from_msgpack(bin8_data);
    CHECK(j8.is_string());
    CHECK_EQ(j8.get<std::string>(), "hello");

    // 0xC5 = bin 16, len = 4, followed by "data"
    std::vector<uint8_t> bin16_data = {0xC5, 0x00, 0x04, 'd', 'a', 't', 'a'};
    json j16 = senko::from_msgpack(bin16_data);
    CHECK(j16.is_string());
    CHECK_EQ(j16.get<std::string>(), "data");
}

TEST_CASE("Binary - CBOR Half-Precision Float16 (RFC 8949)") {
    // 0xF9 3C 00 -> 1.0
    std::vector<uint8_t> f16_one = {0xF9, 0x3C, 0x00};
    json j_one = senko::from_cbor(f16_one);
    CHECK(j_one.is_number_float());
    CHECK_EQ(j_one.get<double>(), 1.0);

    // 0xF9 C0 00 -> -2.0
    std::vector<uint8_t> f16_neg_two = {0xF9, 0xC0, 0x00};
    json j_neg_two = senko::from_cbor(f16_neg_two);
    CHECK_EQ(j_neg_two.get<double>(), -2.0);

    // 0xF9 00 00 -> 0.0
    std::vector<uint8_t> f16_zero = {0xF9, 0x00, 0x00};
    json j_zero = senko::from_cbor(f16_zero);
    CHECK_EQ(j_zero.get<double>(), 0.0);

    // 0xF9 35 55 -> 0.333251953125 (RFC 8949 example)
    std::vector<uint8_t> f16_third = {0xF9, 0x35, 0x55};
    json j_third = senko::from_cbor(f16_third);
    CHECK(std::abs(j_third.get<double>() - 0.333251953125) < 1e-6);

    // 0xF9 7B FF -> 65504.0 (max half float)
    std::vector<uint8_t> f16_max = {0xF9, 0x7B, 0xFF};
    json j_max = senko::from_cbor(f16_max);
    CHECK_EQ(j_max.get<double>(), 65504.0);

    // 0xF9 7C 00 -> +infinity
    std::vector<uint8_t> f16_inf = {0xF9, 0x7C, 0x00};
    json j_inf = senko::from_cbor(f16_inf);
    CHECK(std::isinf(j_inf.get<double>()));
    CHECK(j_inf.get<double>() > 0.0);

    // 0xF9 7E 00 -> NaN
    std::vector<uint8_t> f16_nan = {0xF9, 0x7E, 0x00};
    json j_nan = senko::from_cbor(f16_nan);
    CHECK(std::isnan(j_nan.get<double>()));
}



