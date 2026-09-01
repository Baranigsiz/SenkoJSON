#include "test_framework.hpp"
#include <senko/senko.hpp>

using json = senko::json;

TEST_CASE("Errors - Syntax Errors & Diagnostics") {
    // Empty input
    CHECK_THROWS(json::parse(""));
    CHECK_THROWS(json::parse("   "));

    // Unterminated string
    CHECK_THROWS(json::parse("{\"key\": \"unterminated}"));

    // Missing colon
    CHECK_THROWS(json::parse("{\"key\" 123}"));

    // Trailing comma (when disabled by default in strict mode)
    CHECK_THROWS(json::parse("{\"key\": 123,}"));
    CHECK_THROWS(json::parse("[1, 2, 3,]"));

    // Unclosed braces
    CHECK_THROWS(json::parse("{\"key\": 123"));
    CHECK_THROWS(json::parse("[1, 2, 3"));

    // Trailing garbage
    CHECK_THROWS(json::parse("{\"key\": 123} extra_garbage"));

    // Invalid numbers
    CHECK_THROWS(json::parse("0123")); // leading zero
    CHECK_THROWS(json::parse("+123")); // plus sign not allowed in standard JSON
    CHECK_THROWS(json::parse("1."));   // missing digits after dot
}

TEST_CASE("Errors - Permissive Config Mode (Comments & Trailing Comma)") {
    std::string config_with_comments = R"({
        // Application configuration
        "port": 8080, /* Default HTTP port */
        "workers": 4, // Trailing comma below:
        "tags": ["web", "api",],
    })";

    // Strict mode should throw
    CHECK_THROWS(json::parse(config_with_comments, false, false));

    // Permissive mode should succeed
    CHECK_NOTHROW({
        json cfg = json::parse(config_with_comments, true, true);
        CHECK_EQ(cfg["port"].get<int>(), 8080);
        CHECK_EQ(cfg["workers"].get<int>(), 4);
        CHECK_EQ(cfg["tags"].size(), 2);
    });
}
