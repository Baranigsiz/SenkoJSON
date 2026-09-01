#include "test_framework.hpp"
#include <senko/senko.hpp>

using json = senko::json;

TEST_CASE("Unicode - Basic \\uXXXX Escapes") {
    json j = json::parse(R"({"euro": "\u20AC", "copyright": "\u00A9"})");
    CHECK_EQ(j["euro"].get<std::string>(), "\xE2\x82\xAC"); // UTF-8 for €
    CHECK_EQ(j["copyright"].get<std::string>(), "\xC2\xA9"); // UTF-8 for ©
}

TEST_CASE("Unicode - UTF-16 Surrogate Pairs (Emojis & 4-byte UTF-8)") {
    // \uD83D\uDE00 is Grinning Face emoji 😀 (U+1F600)
    json j = json::parse(R"({"emoji": "\uD83D\uDE00", "rocket": "\uD83D\uDE80"})");
    CHECK_EQ(j["emoji"].get<std::string>(), "\xF0\x9F\x98\x80"); // 😀
    CHECK_EQ(j["rocket"].get<std::string>(), "\xF0\x9F\x9A\x80"); // 🚀
}

TEST_CASE("Unicode - Direct UTF-8 Multi-language Strings") {
    std::string multi_lang = R"({
        "turkish": "Türkçe Karakterler: ğüşiöç ĞÜŞİÖÇ",
        "japanese": "日本語テスト",
        "arabic": "مرحبا بالعالم",
        "cyrillic": "Привет мир"
    })";

    json j = json::parse(multi_lang);
    CHECK_EQ(j["turkish"].get<std::string>(), "Türkçe Karakterler: ğüşiöç ĞÜŞİÖÇ");
    CHECK_EQ(j["japanese"].get<std::string>(), "日本語テスト");

    // Round-trip dump & parse
    std::string dumped = j.dump(2);
    json reparsed = json::parse(dumped);
    CHECK_EQ(reparsed["turkish"].get<std::string>(), j["turkish"].get<std::string>());
    CHECK_EQ(reparsed["japanese"].get<std::string>(), j["japanese"].get<std::string>());
}
