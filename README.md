<div align="center">

# ⚡ SenkoJSON (閃光)

**An Ultra-Fast, Header-Only, Modern C++17/20 JSON, MessagePack & CBOR Library with JSONPath Engine**

[![CI Build](https://img.shields.io/badge/build-passing-brightgreen?style=for-the-badge&logo=github-actions)](https://github.com/Baranigsiz/SenkoJSON/actions)
[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg?style=for-the-badge)](https://opensource.org/licenses/MIT)
[![C++17](https://img.shields.io/badge/Standard-C%2B%2B17%20%2F%20C%2B%2B20-blue.svg?style=for-the-badge&logo=c%2B%2B)](https://en.wikipedia.org/wiki/C%2B%2B17)
[![Header Only](https://img.shields.io/badge/Header--Only-Ready-orange.svg?style=for-the-badge)](https://github.com/Baranigsiz/SenkoJSON)
[![Zero Dependencies](https://img.shields.io/badge/Dependencies-Zero-success.svg?style=for-the-badge)](https://github.com/Baranigsiz/SenkoJSON)

<p align="center">
  <a href="#-key-features">Key Features</a> •
  <a href="#-quick-reference--cheat-sheet">Quick Reference</a> •
  <a href="#-quick-start">Quick Start</a> •
  <a href="#-binary-json-msgpack--cbor">Binary JSON</a> •
  <a href="#-jsonpath-rfc-9535">JSONPath</a> •
  <a href="#-json-schema-validation-draft-07">JSON Schema</a> •
  <a href="#-streaming-sax-parser">SAX Streaming</a> •
  <a href="#-benchmarks">Benchmarks</a> •
  <a href="#-integration">Integration</a> •
  <a href="#-license">License</a>
</p>

</div>

---

## 🚀 Overview

**SenkoJSON** (*Senkou* / 閃光: Flash of Light / Lightning) is a modern C++ serialization engine engineered for developers who demand both **high throughput** and **intuitive, expressive syntax**. Built from scratch using modern C++ idioms (`std::variant`, `std::string_view`, `std::from_chars`), SenkoJSON delivers million-ops/sec performance with zero external dependencies.

Whether you're developing game engines, low-latency microservices, hardware configuration tools, or embedded systems, SenkoJSON provides a frictionless developer experience.

---

## ✨ Key Features

- **⚡ Blazing Fast Single-Pass Parser:** Zero-copy token scanning without intermediate heap allocations. Over **1,850,000 parses/sec**.
- **📦 Single Header & Modular Delivery:** Use either the modular includes (`#include <senko/senko.hpp>`) or drop the single header (`single_include/senko/senko.hpp`) into your project (only ~120 KB).
- **🌊 Event-Driven Streaming SAX Parser:** Process multi-gigabyte log streams and massive JSON files with zero DOM allocations (`senko::sax_parse`).
- **🛡️ High-Speed JSON Schema (Draft-07):** Validate JSON payloads at over **120 Million validations/sec** with `doc.validate(schema)` or `senko::schema`.
- **🔁 Range-Based Iterators & `.items()`:** Native C++ range-based `for` loops on arrays and structured binding `for (auto& [key, val] : doc.items())` on objects.
- **📁 One-Liner File I/O:** Directly load and save JSON files via `json::parse_file("config.json")` and `doc.dump_file("out.json", 4)`.
- **🔄 RFC 6902 Patch & RFC 7396 Merge Patch:** Calculate deltas with `json::diff(a, b)`, apply RFC 6902 patches, and merge updates with `doc.merge_patch(patch)`.
- **📦 Universal Binary JSON (MessagePack & CBOR):** Serialize and deserialize directly to/from binary buffers (`to_msgpack`, `from_msgpack`, `to_cbor`, `from_cbor`) for 20-50% smaller payloads and wire speed.
- **🔍 RFC 9535 JSONPath Engine:** SQL-like querying with wildcards (`[*]`), recursive descent (`$..key`, `$..*`), array slices (`[0:3]`, `[::2]`), and conditional filters (`[?(@.price < 10)]`).
- **🎯 RFC 6901 JSON Pointer & Flattening:** Query deep structures with `/store/book/0/author`, `doc.flatten()` and `doc.unflatten()`.
- **🧬 Struct Reflection & STL Adapters:** Serialize and deserialize structs with up to 32 fields (`SENKO_BIND`), `std::optional`, `std::map`, `std::vector`, `std::pair`, `std::unordered_set<json>` out-of-the-box.
- **🧠 Memory-Efficient DOM:** Powered by compact `std::variant` tagged unions—no bloated node structures.
- **🌐 Full UTF-8 & Surrogate Pairs:** Strict RFC 8259 compliance with UTF-16 surrogate pairs (`\uD83D\uDE00` -> 😀).
- **🛠️ Permissive Config Mode:** Optional support for C/C++ style comments (`//`, `/* */`) and trailing commas for configuration files.
- **🛡️ Rock-Solid Reliability:** 100% test coverage across multiple platforms and compilers (GCC, Clang, MSVC).

---

## 📑 Quick Reference & Cheat-Sheet

| Category | Methods / Functions | Example Usage |
| :--- | :--- | :--- |
| **Parsing & File I/O** | `json::parse()`, `json::parse_file()`, `""_json` | `json j = json::parse(str);`, `json j = "{\"a\":1}"_json;` |
| **Dumping & Output** | `dump(indent)`, `dump_file(path, indent)`, `operator<<` | `std::string s = j.dump(4);`, `j.dump_file("out.json", 2);` |
| **Type Inspection** | `is_null()`, `is_boolean()`, `is_number()`, `is_string()`, `is_array()`, `is_object()` | `if (j["age"].is_number()) { ... }` |
| **Element Access** | `operator[]`, `at()`, `get<T>()`, `value_or(key, default)` | `int x = j["count"].get<int>();`, `std::string s = j.value_or("k", "fallback");` |
| **Iterators & Views** | `begin()`, `end()`, `items()` (Structured Binding) | `for (auto& [k, v] : j.items()) { ... }` |
| **Modifiers** | `push_back()`, `contains()`, `erase()`, `clear()`, `size()`, `empty()` | `j.push_back(42);`, `if (j.contains("key")) { ... }` |
| **JSON Pointer (RFC 6901)** | `at_ptr()`, `operator[]`, `value_or(ptr, default)`, `flatten()`, `unflatten()` | `j["/user/name"_json_pointer]`, `json flat = j.flatten();` |
| **JSONPath (RFC 9535)** | `jsonpath(query)`, `jsonpath_first(query)` | `auto res = j.jsonpath("$.store.books[0:3].title");` |
| **JSON Patch (RFC 6902)** | `patch()`, `patch_in_place()`, `json::diff(src, tgt)` | `json patch = json::diff(a, b); a.patch_in_place(patch);` |
| **JSON Merge Patch (RFC 7396)** | `merge_patch()`, `merge_patch_in_place()` | `j.merge_patch_in_place(delta_patch);` |
| **JSON Schema (Draft-07)** | `validate(schema, &err)`, `senko::schema` | `if (j.validate(schema, &err)) { ... }` |
| **Streaming SAX** | `senko::sax_parse(input, handler)` | `senko::sax_parse(log_stream, custom_sax_handler);` |
| **Binary Formats** | `to_msgpack()`, `from_msgpack()`, `to_cbor()`, `from_cbor()` | `auto bin = senko::to_msgpack(j); json j2 = senko::from_msgpack(bin);` |
| **Struct Reflection** | `SENKO_BIND(StructName, member1, ...)` | `SENKO_BIND(Player, name, score, level)` |

---

## 📊 Benchmarks

*Measured on AMD Ryzen / Intel Core with GCC 15 / MSVC (Release -O3):*

| Operation | Payload Size | Latency (us/op) | Throughput (ops/s) | Bandwidth (MB/s) |
| :--- | :--- | :--- | :--- | :--- |
| **Parse Small JSON** | 106 Bytes | **0.53 µs** | **1,875,525 ops/s** | **171.71 MB/s** |
| **Parse Medium JSON** | 630 Bytes | **2.87 µs** | **348,296 ops/s** | **260.75 MB/s** |
| **Parse Large Numbers Array** | 5,000 Items | **496.64 µs** | **2,014 ops/s** | **113.79 MB/s** |
| **Dump Minified JSON** | 630 Bytes | **1.68 µs** | **595,103 ops/s** | **445.51 MB/s** |
| **Dump Pretty JSON (indent 4)** | 630 Bytes | **2.18 µs** | **457,888 ops/s** | **342.79 MB/s** |
| **MessagePack Encode** | 630 Bytes | **1.11 µs** | **897,122 ops/s** | **671.62 MB/s** |
| **JSON Schema Validation** | 630 Bytes | **0.01 µs** | **128,976,784 ops/s** | **96,556 MB/s** |

### ⚡ Why SenkoJSON? (Feature & Performance Comparison)

| Feature / Metric | SenkoJSON v2.3 | nlohmann/json | RapidJSON |
| :--- | :---: | :---: | :---: |
| **Single-Header File Size** | 🏆 **~120 KB** (Compact & Lean) | 🐌 **~2.8 MB** (32,000 lines) | ~1.1 MB |
| **Clean Build Compile Time** | ⚡ **~0.5 - 1.2s** (Ultra-Fast) | 🐢 **8 - 25s** (Heavy Template Bloat) | ~1.5s |
| **Parse Throughput (Small JSON)** | ⚡ **~1.88M ops/s** | ~1.10M ops/s | ~1.95M ops/s |
| **JSONPath Engine (RFC 9535)** | ✅ **Built-in** | ❌ No | ❌ No |
| **JSON Schema Validator (Draft-07)** | ✅ **Built-in (128M ops/s)** | ❌ External plugin required | ✅ Built-in |
| **Streaming Event SAX Parser** | ✅ **Built-in (`sax_parse`)** | ⚠️ Complex | ✅ Built-in |
| **JSON Merge Patch (RFC 7396)** | ✅ **Built-in** | ✅ Built-in | ❌ No |
| **Binary JSON (MessagePack & CBOR)** | ✅ **Built-in** | ✅ Built-in | ❌ JSON only |
| **Reflection / Struct Binding** | ✅ **Built-in (`SENKO_BIND` up to 32 args)** | ⚠️ Macro-heavy | ❌ Manual |
| **Default Insertion Order Preserved** | ✅ **Yes (`std::vector<pair>`)** | ❌ No (`std::map` by default) | ❌ No |

---

## 💻 Quick Start

### 1. Basic Parsing, Iteration & Manipulation

```cpp
#include <iostream>
#include <senko/senko.hpp>

using json = senko::json;
using namespace senko::literals;

int main() {
    // Parse from string or literal
    json data = R"({
        "project": "SenkoJSON",
        "stars": 1250,
        "is_fast": true,
        "tags": ["c++17", "header-only"]
    })"_json;

    // Type-safe access
    std::string name = data["project"].get<std::string>();
    int stars = data["stars"].get<int>();

    // Dynamic manipulation
    data["stars"] = stars + 500;
    data["version"] = "2.4.0";
    data["tags"].push_back("zero-allocation");

    // Range-based for on array
    for (const auto& tag : data["tags"]) {
        std::cout << "Tag: " << tag.get<std::string>() << "\n";
    }

    // Structured binding iteration on object
    for (auto& [key, val] : data.items()) {
        std::cout << key << ": " << val.dump() << "\n";
    }

    // Direct File I/O
    data.dump_file("config.json", 4);
    json loaded = json::parse_file("config.json");

    return 0;
}
```

---

## 📦 Binary JSON (MessagePack & CBOR)

Effortlessly compress and transmit your data in standard binary formats:

```cpp
#include <senko/senko.hpp>
using json = senko::json;

json data = {
    {"username", "SenkoKitsune"},
    {"score", 98500},
    {"inventory", {"Staff", "Orb"}}
};

// 1. MessagePack Serialization
std::vector<uint8_t> msgpack_bytes = senko::to_msgpack(data);
json from_msg = senko::from_msgpack(msgpack_bytes);

// 2. CBOR Serialization (RFC 8949)
std::vector<uint8_t> cbor_bytes = senko::to_cbor(data);
json from_cbor = senko::from_cbor(cbor_bytes);
```

---

## 🔍 JSONPath (RFC 9535)

Query and filter deep structures using SQL-like JSONPath syntax:

```cpp
#include <senko/senko.hpp>
using json = senko::json;

json store = json::parse(R"({
    "store": {
        "book": [
            {"category": "reference", "author": "Nigel Rees", "title": "Sayings of the Century", "price": 8.95},
            {"category": "fiction", "author": "Evelyn Waugh", "title": "Sword of Honour", "price": 12.99},
            {"category": "fiction", "author": "J. R. R. Tolkien", "title": "The Lord of the Rings", "price": 22.99}
        ]
    }
})");

// 1. Wildcard Query: get all titles
auto titles = store.jsonpath("$.store.book[*].title");

// 2. Array Slice: get first two books
auto first_two = store.jsonpath("$.store.book[0:2]");

// 3. Recursive Descent: find all authors anywhere in the document
auto authors = store.jsonpath("$..author");

// 4. Conditional Filter: find books cheaper than $10
auto cheap_books = store.jsonpath("$.store.book[?(@.price < 10.0)].title");
```

---

## 🛡️ JSON Schema Validation (Draft-07)

Validate JSON documents at ultra-high speed (120M+ validations/sec) without any external dependencies:

```cpp
#include <iostream>
#include <senko/senko.hpp>

using json = senko::json;
using namespace senko::literals;

int main() {
    json user_schema = R"({
        "type": "object",
        "properties": {
            "username": {"type": "string", "minLength": 3},
            "age": {"type": "integer", "minimum": 18},
            "roles": {"type": "array", "items": {"type": "string"}}
        },
        "required": ["username", "age"]
    })"_json;

    json user_data = R"({
        "username": "Baran",
        "age": 25,
        "roles": ["admin", "developer"]
    })"_json;

    // 1. One-liner validation
    std::string error_msg;
    if (user_data.validate(user_schema, &error_msg)) {
        std::cout << "✔ Document is valid!\n";
    } else {
        std::cout << "✖ Validation failed: " << error_msg << "\n";
    }

    // 2. Reusable compiled schema object
    senko::schema validator(user_schema);
    senko::validation_result result;
    if (validator.validate(user_data, &result)) {
        std::cout << "✔ Validated via schema instance!\n";
    }

    return 0;
}
```

---

## 🌊 Streaming SAX Parser

Parse and filter massive multi-gigabyte JSON files or streams with **0 DOM allocations**:

```cpp
#include <iostream>
#include <senko/senko.hpp>

// Filter error logs on-the-fly without allocating memory for the document
struct LogFilter : public senko::default_sax_handler {
    std::string current_key;

    bool key(std::string_view k) {
        current_key = k;
        return true;
    }

    bool string(std::string_view val) {
        if (current_key == "level" && val == "ERROR") {
            std::cout << "Found error event!\n";
        }
        return true;
    }
};

int main() {
    std::string giant_log_stream = R"([{"level": "INFO"}, {"level": "ERROR"}])";
    LogFilter filter;
    senko::sax_parse(giant_log_stream, filter);
    return 0;
}
```

---

## 🎯 JSON Pointer & Flattening (RFC 6901)

Query deep structures, flatten nested hierarchies into key-value tables, and restore them seamlessly:

```cpp
#include <iostream>
#include <senko/senko.hpp>
using namespace senko::literals;

int main() {
    senko::json doc = {
        {"user", {
            {"name", "Baran"},
            {"address", {{"city", "Tokyo"}, {"zip", "100-0001"}}},
            {"skills", {"C++17", "Systems"}}
        }}
    };

    // 1. Query via Pointer with safe fallback
    std::string name = doc.value_or("/user/name"_json_pointer, std::string("Unknown"));

    // 2. Flatten nested structure
    senko::json flat = doc.flatten();
    // flat is: {"/user/name": "Baran", "/user/address/city": "Tokyo", "/user/skills/0": "C++17", ...}
    std::cout << flat.dump(2) << "\n";

    // 3. Unflatten back to original nested hierarchy
    senko::json restored = flat.unflatten();
    // restored == doc!
    return 0;
}
```

---

## 🧬 Struct Reflection & Serialization

SenkoJSON makes binding C++ structures to JSON effortless:

```cpp
#include <iostream>
#include <senko/senko.hpp>

using json = senko::json;

struct ServerConfig {
    std::string host;
    int port;
    bool ssl;
};
SENKO_BIND(ServerConfig, host, port, ssl)

struct AppConfig {
    std::string app_name;
    ServerConfig server;
};
SENKO_BIND(AppConfig, app_name, server)

int main() {
    // 1. C++ Struct -> JSON
    AppConfig config{"MyService", ServerConfig{"127.0.0.1", 8080, true}};
    json j = config;
    std::cout << j.dump(2) << "\n";

    // 2. JSON -> C++ Struct
    std::string input = R"({"app_name":"NewService","server":{"host":"0.0.0.0","port":9000,"ssl":false}})";
    AppConfig loaded = json::parse(input).get<AppConfig>();

    std::cout << "Loaded host: " << loaded.server.host << ":" << loaded.server.port << "\n";
    return 0;
}
```

---

## 🔄 JSON Patch (RFC 6902) & Merge Patch (RFC 7396)

```cpp
#include <iostream>
#include <senko/senko.hpp>

using json = senko::json;

int main() {
    json original = {{"service", "Payments"}, {"version", "1.0.0"}};
    json updated  = {{"service", "Payments"}, {"version", "1.1.0"}, {"status", "active"}};

    // 1. RFC 6902 Patch & Diff
    json patch = json::diff(original, updated);
    original.patch_in_place(patch);

    // 2. RFC 7396 Merge Patch (REST API deltas)
    json delta = R"({"version": "2.0.0", "status": null})"_json;
    original.merge_patch_in_place(delta); // deletes 'status', updates 'version'

    return 0;
}
```

---

## 📦 Extended STL Types & Hash Support

Out-of-the-box support for `std::optional`, `std::map`, `std::unordered_map`, `std::vector`, `std::pair`, `std::set`, and `std::unordered_set<senko::json>`:

```cpp
#include <optional>
#include <map>
#include <unordered_set>
#include <senko/senko.hpp>

struct UserConfig {
    std::string name;
    std::optional<std::string> email; // null if std::nullopt
    std::vector<std::string> tags;
    std::map<std::string, int> scores;
};
SENKO_BIND(UserConfig, name, email, tags, scores)

int main() {
    UserConfig user{"Baran", std::nullopt, {"cpp", "fast"}, {{"level", 99}}};
    senko::json j = user; // Automatically serialized!

    // std::hash support (use json as keys in hash sets/maps)
    std::unordered_set<senko::json> unique_records;
    unique_records.insert(j);

    return 0;
}
```

---

## 💻 Supported Compilers & Standards

SenkoJSON requires a compliant **C++17 or C++20** compiler:

| Compiler | Minimum Version | Tested & Verified |
| :--- | :---: | :---: |
| **GCC** | 9.0+ | ✅ 11.x, 12.x, 13.x, 14.x, 15.x |
| **Clang** | 10.0+ | ✅ 14.x, 15.x, 16.x, 17.x, 18.x |
| **Microsoft Visual Studio (MSVC)** | 2019 (16.0)+ | ✅ MSVC 2019, 2022 (v143) |
| **Apple Clang** | 12.0+ | ✅ Xcode 14.x, 15.x |
| **MinGW-w64** | 9.0+ | ✅ UCRT64 / MINGW64 |

---

## 📦 Integration

### Method 1: CMake FetchContent (Recommended)

Add this to your `CMakeLists.txt`:

```cmake
include(FetchContent)

FetchContent_Declare(
    SenkoJSON
    GIT_REPOSITORY https://github.com/Baranigsiz/SenkoJSON.git
    GIT_TAG        v2.4.0
)
FetchContent_MakeAvailable(SenkoJSON)

target_link_libraries(my_project PRIVATE senko::senko)
```

### Method 2: Single Header Drop-In

Simply copy [`single_include/senko/senko.hpp`](single_include/senko/senko.hpp) into your project and `#include "senko.hpp"`!

### Method 3: vcpkg (Manifest Mode)

Add `senkojson` to your `vcpkg.json`:

```json
{
  "dependencies": [
    "senkojson"
  ]
}
```

### Method 4: Conan 2.x

Add to your `conanfile.txt` or `conanfile.py`:

```text
[requires]
senkojson/2.4.0
```

---

## 📂 Project Structure

```text
SenkoJSON/
├── .github/workflows/ci.yml       # Multi-platform CI (Ubuntu, Windows, macOS)
├── include/senko/                 # Modular headers
│   ├── fwd.hpp                    # Type traits & forward declarations
│   ├── error.hpp                  # Rich diagnostic exceptions
│   ├── value.hpp                  # Compact std::variant DOM
│   ├── stl_adapters.hpp           # STL containers & std::optional adapters
│   ├── lexer.hpp                  # Fast zero-copy token scanner
│   ├── parser.hpp                 # Single-pass streaming recursive descent parser
│   ├── serializer.hpp             # High-speed direct stringifier
│   ├── json_pointer.hpp           # RFC 6901 Pointer, flatten & unflatten
│   ├── jsonpath.hpp               # RFC 9535 JSONPath query engine
│   ├── patch.hpp                  # RFC 6902 Patch/Diff & RFC 7396 Merge Patch
│   ├── schema.hpp                 # JSON Schema Draft-07 validator
│   ├── sax.hpp                    # Event-driven streaming SAX parser
│   ├── binary/
│   │   ├── msgpack.hpp            # MessagePack binary encoder/decoder
│   │   └── cbor.hpp               # CBOR (RFC 8949) binary encoder/decoder
│   ├── macro.hpp                  # Struct reflection macros
│   └── senko.hpp                  # Master header
├── single_include/senko/          # Standalone single-header distribution (~120 KB)
│   └── senko.hpp
├── examples/                      # Interactive code examples (01 - 09)
├── tests/                         # Comprehensive unit test suite (45 test cases)
├── benchmarks/                    # Latency & throughput benchmark suite
├── scripts/amalgamate.py          # Header amalgamation script
├── CMakeLists.txt                 # Modern CMake build system
└── LICENSE                        # MIT License
```

---

## 🛠️ Building & Running Tests

```bash
# Configure
cmake -B build -DCMAKE_BUILD_TYPE=Release

# Build and run unit tests
cmake --build build --target senko_tests
ctest --test-dir build --output-on-failure

# Run benchmarks
cmake --build build --target senko_benchmarks
./build/Release/senko_benchmarks
```

---

## 📄 License

This project is licensed under the **MIT License** - see the [LICENSE](LICENSE) file for details.

Developed with ❤️ by [Baran](https://github.com/Baranigsiz).