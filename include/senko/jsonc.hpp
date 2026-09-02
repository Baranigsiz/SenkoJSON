#pragma once

#include "fwd.hpp"
#include "value.hpp"

#include <string_view>
#include <istream>
#include <string>

namespace senko {

/**
 * @brief JSONC (JSON with Comments & Trailing Commas) Parser Engine.
 * Ideal for application configuration, VS Code settings.json, tsconfig.json, and game configs.
 */
namespace jsonc {

/**
 * @brief Parses a JSONC string containing single-line comments (//), multi-line comments (/* * /), and trailing commas.
 */
inline value parse(std::string_view input) {
    return value::parse(input, /*allow_comments=*/true, /*allow_trailing_comma=*/true);
}

/**
 * @brief Parses a JSONC input stream.
 */
inline value parse(std::istream& is) {
    return value::parse(is, /*allow_comments=*/true, /*allow_trailing_comma=*/true);
}

/**
 * @brief Parses a JSONC file from disk.
 */
inline value parse_file(const std::string& filepath) {
    return value::parse_file(filepath, /*allow_comments=*/true, /*allow_trailing_comma=*/true);
}

} // namespace jsonc

namespace literals {

/**
 * @brief User-defined literal for parsing JSONC strings with comments and trailing commas.
 * Example: auto cfg = R"({ // comment\n "port": 8080, })"_jsonc;
 */
inline value operator""_jsonc(const char* str, size_t len) {
    return jsonc::parse(std::string_view(str, len));
}

} // namespace literals

} // namespace senko
