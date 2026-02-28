# CoreJSON 🚀

A high-performance, modular, and header-friendly JSON parser and serializer built from scratch in modern C++ (C++17). 

CoreJSON is designed to be lightweight and highly readable, making it perfect for custom engines, hardware configuration tools, or fetching API data for backend projects. It does not rely on any massive external dependencies; it just uses standard C++ libraries.

## ✨ Features

- **Lexer/Tokenizer:** Efficiently scans raw strings and categorizes tokens.
- **Recursive Descent Parser:** Builds a fully functional in-memory Tree/DOM structure.
- **Data Models (`JsonNode`):** Safely handles Strings, Numbers, Booleans, Nulls, Arrays, and deeply nested Objects.
- **Serializer (`dump`):** Converts the manipulated C++ memory tree back into a beautifully formatted JSON string.
- **File I/O Ready:** Easily read configuration files from disk and write optimized data back.

## 📂 Project Structure

```text
CoreJSON/
├── include/           # Header files
│   └── corejson/
│       ├── Token.h
│       ├── Lexer.h
│       ├── JsonNode.h
│       └── Parser.h
├── src/               # Source files
│   ├── Lexer.cpp
│   ├── JsonNode.cpp
│   ├── Parser.cpp
│   └── main.cpp       # Example Usage
├── tests/             # Experimental & Test files
└── apex_config.json   # Sample config file
```

## 🛠️ Getting Started

### Prerequisites
Make sure you have a C++17 compatible compiler installed (like `g++` via MSYS2 for Windows, or `clang++`).

### Compilation
You can quickly compile the project via the terminal:

```bash
g++ src/main.cpp src/Lexer.cpp src/JsonNode.cpp src/Parser.cpp -I include -o corejson_app
```

### Run
```bash
./corejson_app
```

## 💻 Example Usage

```cpp
#include <iostream>
#include "corejson/Lexer.h"
#include "corejson/Parser.h"

int main() {
    std::string rawJson = R"({ "project": "AfterAnime", "version": 1.0, "active": true })";
    
    // 1. Tokenize
    corejson::Lexer lexer(rawJson);
    auto tokens = lexer.tokenize();

    // 2. Parse into Tree
    corejson::Parser parser(tokens);
    corejson::JsonNode root = parser.parse();

    // 3. Manipulate Data
    root.object_values["version"] = corejson::JsonNode(2.0);

    // 4. Output Formatted JSON
    std::cout << root.dump(4) << "\n";

    return 0;
}
```

## 🚀 Future Roadmap
- [x] Add line/column specific error tracking for malformed JSONs.
- [x] Optimize memory allocation using `std::string_view`.
- [x] Add minified output generation.
- [x] Full UTF-8 character support.

---
*Built from scratch to understand the deep architecture behind data serialization.*