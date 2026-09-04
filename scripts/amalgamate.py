#!/usr/bin/env python3
"""
Amalgamation script for SenkoJSON.
Combines all header files in include/senko/ into a single standalone header in single_include/senko/senko.hpp
"""

import os
import re

HEADERS_ORDER = [
    "fwd.hpp",
    "error.hpp",
    "value.hpp",
    "stl_adapters.hpp",
    "lexer.hpp",
    "parser.hpp",
    "serializer.hpp",
    "json_pointer.hpp",
    "jsonpath.hpp",
    "patch.hpp",
    "binary/msgpack.hpp",
    "binary/cbor.hpp",
    "macro.hpp",
    "schema.hpp",
    "sax.hpp",
    "jsonc.hpp",
    "jsonl.hpp"
]

HEADER_GUARD = """/**
 * SenkoJSON - Single Header Amalgamation
 * https://github.com/Baranigsiz/SenkoJSON
 * 
 * Version: 2.6.0
 * License: MIT
 * 
 * Lightning-fast, zero-overhead modern C++17/20 JSON library with MessagePack, CBOR, JSONPath, JSON Schema & SAX Streaming.
 */

#ifndef SENKO_SINGLE_AMALGAMATION_HPP
#define SENKO_SINGLE_AMALGAMATION_HPP

#define SENKO_VERSION_MAJOR 2
#define SENKO_VERSION_MINOR 6
#define SENKO_VERSION_PATCH 0
"""

FOOTER = """
namespace senko {
namespace literals {

inline value operator""_json(const char* str, size_t len) {
    return value::parse(std::string_view(str, len));
}

inline value operator""_json(unsigned long long val) {
    return value(static_cast<uint64_t>(val));
}

inline value operator""_json(long double val) {
    return value(static_cast<double>(val));
}

inline json_pointer operator""_json_pointer(const char* str, size_t len) {
    return json_pointer(std::string_view(str, len));
}

inline value operator""_jsonc(const char* str, size_t len) {
    return jsonc::parse(std::string_view(str, len));
}

} // namespace literals
} // namespace senko

#endif // SENKO_SINGLE_AMALGAMATION_HPP
"""

def amalgamate():
    base_dir = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
    include_dir = os.path.join(base_dir, "include", "senko")
    output_dir = os.path.join(base_dir, "single_include", "senko")
    output_file = os.path.join(output_dir, "senko.hpp")

    os.makedirs(output_dir, exist_ok=True)

    combined_content = [HEADER_GUARD]

    # Process headers
    for h in HEADERS_ORDER:
        h_path = os.path.join(include_dir, h)
        if not os.path.exists(h_path):
            print(f"Warning: header {h_path} not found")
            continue

        with open(h_path, "r", encoding="utf-8") as f:
            content = f.read()

        # Strip #pragma once
        content = re.sub(r'#pragma\s+once\b', '', content)

        # Strip local #include "..." and #include "../..."
        content = re.sub(r'#include\s+"[^"]+"', '', content)

        banner_name = h.replace('\\', '/')
        combined_content.append(f"\n// ========================================================\n// Header: {banner_name}\n// ========================================================\n")
        combined_content.append(content)

    combined_content.append(FOOTER)

    with open(output_file, "w", encoding="utf-8", newline="\n") as out:
        out.write("\n".join(combined_content))

    print(f"Successfully generated single-header: {output_file}")

if __name__ == "__main__":
    amalgamate()
