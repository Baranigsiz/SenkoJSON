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
  <a href="#-quick-start">Quick Start</a> •
  <a href="#-binary-json-msgpack--cbor">Binary JSON</a> •
  <a href="#-json-patch--diff-rfc-6902">JSON Patch</a> •
  <a href="#-jsonpath-rfc-9535">JSONPath</a> •
  <a href="#-struct-reflection--serialization">Struct Reflection</a> •
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

- **⚡ Blazing Fast Single-Pass Parser:** Zero-copy token scanning without intermediate heap allocations. Over **1,800,000 parses/sec**.
- **📦 Single Header & Modular Delivery:** Use either the modular includes (`#include <senko/senko.hpp>`) or drop the single header (`single_include/senko/senko.hpp`) into your project.
- **🔄 RFC 6902 JSON Patch & Diff:** Calculate deltas with `json::diff(a, b)` and apply patches (`doc.patch(diff)`).
- **📦 Universal Binary JSON (MessagePack & CBOR):** Serialize and deserialize directly to/from binary buffers (`to_msgpack`, `from_msgpack`, `to_cbor`, `from_cbor`) for 20-50% smaller payloads and wire speed.
- **🔍 RFC 9535 JSONPath Engine:** SQL-like querying with wildcards (`[*]`), recursive descent (`$..key`), and conditional filters (`[?(@.price < 10)]`).
- **🎯 RFC 6901 JSON Pointer:** Query and mutate deep nested structures with `/store/book/0/author` syntax.
- **🧬 Struct Reflection & STL Adapters:** Serialize and deserialize structs (`SENKO_BIND`), `std::optional`, `std::map`, `std::vector`, `std::pair` out-of-the-box.
- **🧠 Memory-Efficient DOM:** Powered by compact `std::variant` tagged unions—no bloated node structures.
- **🌐 Full UTF-8 & Surrogate Pairs:** Strict RFC 8259 compliance with UTF-16 surrogate pairs (`\uD83D\uDE00` -> 😀).
- **🛠️ Permissive Config Mode:** Optional support for C/C++ style comments (`//`, `/* */`) and trailing commas for configuration files.
- **🛡️ Rock-Solid Reliability:** 100% test coverage across multiple platforms and compilers (GCC, Clang, MSVC).

---

## 📊 Benchmarks

*Measured on AMD Ryzen / Intel Core with GCC 15 / MSVC (Release -O3):*

| Operation | Payload Size | Latency (us/op) | Throughput (ops/s) | Bandwidth (MB/s) |
| :--- | :--- | :--- | :--- | :--- |
| **Parse Small JSON** | 106 Bytes | **0.55 µs** | **1,823,094 ops/s** | **166.91 MB/s** |
| **Parse Medium JSON** | 630 Bytes | **3.03 µs** | **329,999 ops/s** | **247.05 MB/s** |
| **Parse Large Numbers Array** | 5,000 Items | **509.88 µs** | **1,961 ops/s** | **110.84 MB/s** |
| **Dump Minified JSON** | 630 Bytes | **1.65 µs** | **606,342 ops/s** | **453.93 MB/s** |
| **Dump Pretty JSON (indent 4)** | 630 Bytes | **2.14 µs** | **467,303 ops/s** | **349.84 MB/s** |

---

## 💻 Quick Start

### 1. Basic Parsing & Manipulation

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
    data["version"] = "2.2.0";
    data["tags"].push_back("zero-allocation");

    // Safe fallback value
    std::string license = data.value_or("license", std::string("MIT"));

    // Serialize (Pretty Print or Minified)
    std::cout << data.dump(4) << std::endl;  // Formatted
    std::cout << data.dump(-1) << std::endl; // Minified

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

// 2. Recursive Descent: find all authors anywhere in the document
auto authors = store.jsonpath("$..author");

// 3. Conditional Filter: find books cheaper than $10
auto cheap_books = store.jsonpath("$.store.book[?(@.price < 10.0)].title");
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

## 🔄 JSON Patch & Diff (RFC 6902)

Calculate delta patches between documents and apply updates seamlessly:

```cpp
#include <iostream>
#include <senko/senko.hpp>

using json = senko::json;

int main() {
    json original = {
        {"service", "Payments"},
        {"version", "1.0.0"},
        {"endpoints", {"/pay", "/refund"}}
    };

    json updated = {
        {"service", "Payments"},
        {"version", "1.1.0"},
        {"endpoints", {"/pay", "/refund", "/webhook"}},
        {"status", "active"}
    };

    // 1. Calculate delta patch
    json patch = json::diff(original, updated);
    std::cout << "Delta Patch:\n" << patch.dump(2) << "\n";

    // 2. Apply patch to update original document in-place
    original.patch_in_place(patch);

    // original is now identical to updated!
    return 0;
}
```

---

## 📦 Extended STL Types

Out-of-the-box support for `std::optional`, `std::map`, `std::unordered_map`, `std::vector`, `std::pair`, `std::set`:

```cpp
#include <optional>
#include <map>
#include <vector>
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
    return 0;
}
```

---

## 🎯 JSON Pointer (RFC 6901)

Effortlessly query deep structures:

```cpp
#include <senko/senko.hpp>
using namespace senko::literals;

json doc = json::parse(R"({
    "store": {
        "books": [
            {"title": "The C++ Programming Language", "author": "Bjarne Stroustrup"}
        ]
    }
})");

// Query via RFC 6901 Pointer
std::string author = doc["/store/books/0/author"_json_pointer].get<std::string>();

// Mutate via Pointer
doc["/store/books/0/author"_json_pointer] = "B. Stroustrup";
```

---

## 📦 Integration

### Method 1: CMake FetchContent (Recommended)

Add this to your `CMakeLists.txt`:

```cmake
include(FetchContent)

FetchContent_Declare(
    SenkoJSON
    GIT_REPOSITORY https://github.com/Baranigsiz/SenkoJSON.git
    GIT_TAG        v2.2.0
)
FetchContent_MakeAvailable(SenkoJSON)

target_link_libraries(my_project PRIVATE senko::senko)
```

### Method 2: Single Header Drop-In

Simply copy [`single_include/senko/senko.hpp`](single_include/senko/senko.hpp) into your project's include folder and include it:

```cpp
#include "senko.hpp"
using json = senko::json;
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
│   ├── json_pointer.hpp           # RFC 6901 JSON Pointer implementation
│   ├── jsonpath.hpp               # RFC 9535 JSONPath query engine
│   ├── patch.hpp                  # RFC 6902 JSON Patch & Diff engine
│   ├── binary/
│   │   ├── msgpack.hpp            # MessagePack binary encoder/decoder
│   │   └── cbor.hpp               # CBOR (RFC 8949) binary encoder/decoder
│   ├── macro.hpp                  # Struct reflection macros
│   └── senko.hpp                  # Master header
├── single_include/senko/          # Standalone single-header distribution
│   └── senko.hpp
├── examples/                      # Interactive code examples (01 - 06)
├── tests/                         # Comprehensive unit test suite (30 test cases)
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
./build/senko_benchmarks
```

---

## 📄 License

This project is licensed under the **MIT License** - see the [LICENSE](LICENSE) file for details.

Developed with ❤️ by [Baran](https://github.com/Baranigsiz).