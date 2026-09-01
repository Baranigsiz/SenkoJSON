<div align="center">

# ⚡ SenkoJSON (閃光)

**A Lightning-Fast, Header-Only, Modern C++17/20 JSON Library for High-Performance Applications**

[![CI Build](https://img.shields.io/badge/build-passing-brightgreen?style=for-the-badge&logo=github-actions)](https://github.com/Baranigsiz/SenkoJSON/actions)
[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg?style=for-the-badge)](https://opensource.org/licenses/MIT)
[![C++17](https://img.shields.io/badge/Standard-C%2B%2B17%20%2F%20C%2B%2B20-blue.svg?style=for-the-badge&logo=c%2B%2B)](https://en.wikipedia.org/wiki/C%2B%2B17)
[![Header Only](https://img.shields.io/badge/Header--Only-Ready-orange.svg?style=for-the-badge)](https://github.com/Baranigsiz/SenkoJSON)
[![Zero Dependencies](https://img.shields.io/badge/Dependencies-Zero-success.svg?style=for-the-badge)](https://github.com/Baranigsiz/SenkoJSON)

<p align="center">
  <a href="#-key-features">Key Features</a> •
  <a href="#-quick-start">Quick Start</a> •
  <a href="#-benchmarks">Benchmarks</a> •
  <a href="#-struct-reflection">Struct Reflection</a> •
  <a href="#-json-pointer-rfc-6901">JSON Pointer</a> •
  <a href="#-integration">Integration</a> •
  <a href="#-license">License</a>
</p>

</div>

---

## 🚀 Overview

**SenkoJSON** (*Senkou* / 閃光: Flash of Light / Lightning) is a modern C++ library engineered for developers who demand both **high throughput** and **intuitive, expressive syntax**. Built from scratch using modern C++ idioms (`std::variant`, `std::string_view`, `std::from_chars`), SenkoJSON delivers million-ops/sec performance with zero external dependencies.

Whether you're developing game engines, low-latency microservices, hardware configuration tools, or embedded systems, SenkoJSON provides a frictionless developer experience.

---

## ✨ Key Features

- **⚡ Blazing Fast Single-Pass Parser:** Zero-copy token scanning without intermediate heap allocations. Over **1,000,000 parses/sec**.
- **📦 Single Header & Modular Delivery:** Use either the modular includes (`#include <senko/senko.hpp>`) or drop the single header (`single_include/senko/senko.hpp`) into your project.
- **🎨 Ergonomic DOM API:** Intuitive access like `doc["user"]["name"] = "Alice"`, custom literals (`""_json`), and streaming operators.
- **🧠 Memory-Efficient DOM:** Powered by compact `std::variant` tagged unions—no bloated node structures.
- **🧬 Zero-Boilerplate Struct Reflection:** Serialize and deserialize complex nested C++ structs with a single macro: `SENKO_BIND(Type, ...)`.
- **🎯 RFC 6901 JSON Pointer:** Query and mutate deep nested structures with `/store/book/0/author` syntax.
- **🌐 Full UTF-8 & Surrogate Pairs:** Strict RFC 8259 compliance with UTF-16 surrogate pairs (`\uD83D\uDE00` -> 😀).
- **🛠️ Permissive Config Mode:** Optional support for C/C++ style comments (`//`, `/* */`) and trailing commas for configuration files.
- **🛡️ Rock-Solid Reliability:** 100% test coverage across multiple platforms and compilers (GCC, Clang, MSVC).

---

## 📊 Benchmarks

*Measured on AMD Ryzen / Intel Core with GCC 15 / MSVC (Release -O3):*

| Operation | Payload Size | Latency (us/op) | Throughput (ops/s) | Bandwidth (MB/s) |
| :--- | :--- | :--- | :--- | :--- |
| **Parse Small JSON** | 106 Bytes | **0.97 µs** | **1,031,187 ops/s** | **93.42 MB/s** |
| **Parse Medium JSON** | 630 Bytes | **4.90 µs** | **203,995 ops/s** | **152.72 MB/s** |
| **Parse Large Numbers Array** | 5,000 Items | **908.46 µs** | **1,101 ops/s** | **62.21 MB/s** |
| **Dump Minified JSON** | 630 Bytes | **7.34 µs** | **136,245 ops/s** | **102.00 MB/s** |
| **Dump Pretty JSON (indent 4)** | 630 Bytes | **9.48 µs** | **105,446 ops/s** | **78.94 MB/s** |

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
    data["version"] = "2.0.0";
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
    GIT_REPOSITORY https://github.com/Baranigsiz/CoreJSON.git
    GIT_TAG        v2.0.0
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
│   ├── lexer.hpp                  # Fast zero-copy token scanner
│   ├── parser.hpp                 # Single-pass streaming recursive descent parser
│   ├── serializer.hpp             # High-speed stringifier with RFC 8259 escaping
│   ├── json_pointer.hpp           # RFC 6901 JSON Pointer implementation
│   ├── macro.hpp                  # Struct reflection macros
│   └── senko.hpp                  # Master header
├── single_include/senko/          # Standalone single-header distribution
│   └── senko.hpp
├── examples/                      # Interactive code examples
├── tests/                         # Comprehensive unit test suite
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