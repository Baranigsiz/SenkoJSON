#!/usr/bin/env python3
"""
Amalgamation script for CoreJSON.
Combines all header files in include/corejson/ into a single standalone header in single_include/corejson/corejson.hpp
"""

import os
import re

HEADERS_ORDER = [
    "fwd.hpp",
    "error.hpp",
    "value.hpp",
    "lexer.hpp",
    "parser.hpp",
    "serializer.hpp",
    "json_pointer.hpp",
    "macro.hpp"
]

HEADER_GUARD = """/**
 * CoreJSON - Single Header Amalgamation
 * https://github.com/Baranigsiz/CoreJSON
 * 
 * Version: 2.0.0
 * License: MIT
 * 
 * Modern, high-performance, header-friendly JSON library for C++17/20.
 */

#ifndef COREJSON_SINGLE_AMALGAMATION_HPP
#define COREJSON_SINGLE_AMALGAMATION_HPP

#define COREJSON_VERSION_MAJOR 2
#define COREJSON_VERSION_MINOR 0
#define COREJSON_VERSION_PATCH 0
"""

FOOTER = """
namespace corejson {
namespace literals {

inline value operator""_json(const char* str, size_t len) {
    return value::parse(std::string_view(str, len));
}

inline json_pointer operator""_json_pointer(const char* str, size_t len) {
    return json_pointer(std::string_view(str, len));
}

} // namespace literals
} // namespace corejson

#endif // COREJSON_SINGLE_AMALGAMATION_HPP
"""

def amalgamate():
    base_dir = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
    include_dir = os.path.join(base_dir, "include", "corejson")
    output_dir = os.path.join(base_dir, "single_include", "corejson")
    output_file = os.path.join(output_dir, "corejson.hpp")

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

        # Strip local #include "..."
        content = re.sub(r'#include\s+"[^"]+"', '', content)

        combined_content.append(f"\n// ========================================================\n// Header: {h}\n// ========================================================\n")
        combined_content.append(content)

    combined_content.append(FOOTER)

    with open(output_file, "w", encoding="utf-8", newline="\n") as out:
        out.write("\n".join(combined_content))

    print(f"Successfully generated single-header: {output_file}")

if __name__ == "__main__":
    amalgamate()
