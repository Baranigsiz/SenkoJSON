#pragma once

#include <exception>
#include <string>
#include <sstream>
#include <string_view>
#include <algorithm>

namespace corejson {

/**
 * @brief Base exception class for all CoreJSON errors.
 */
class exception : public std::exception {
public:
    explicit exception(std::string message) : m_message(std::move(message)) {}
    const char* what() const noexcept override { return m_message.c_str(); }
protected:
    std::string m_message;
};

/**
 * @brief Exception thrown when JSON parsing fails (syntax error, unexpected token, invalid escape, etc.).
 */
class parse_error : public exception {
public:
    parse_error(std::string_view raw_msg, size_t line, size_t col, size_t byte_offset, std::string_view context_snippet = {})
        : exception(format_message(raw_msg, line, col, byte_offset, context_snippet)),
          m_line(line), m_column(col), m_byte_offset(byte_offset) {}

    size_t line() const noexcept { return m_line; }
    size_t column() const noexcept { return m_column; }
    size_t byte_offset() const noexcept { return m_byte_offset; }

private:
    size_t m_line;
    size_t m_column;
    size_t m_byte_offset;

    static std::string format_message(std::string_view raw_msg, size_t line, size_t col, size_t offset, std::string_view snippet) {
        std::ostringstream ss;
        ss << "[corejson::parse_error] " << raw_msg
           << " (line " << line << ", column " << col << ", offset " << offset << ")";
        if (!snippet.empty()) {
            ss << "\n    --> " << snippet;
            ss << "\n        " << std::string(col > 0 ? col - 1 : 0, ' ') << "^";
        }
        return ss.str();
    }
};

/**
 * @brief Exception thrown when accessing an element with an incompatible type.
 */
class type_error : public exception {
public:
    explicit type_error(std::string message)
        : exception("[corejson::type_error] " + std::move(message)) {}
};

/**
 * @brief Exception thrown when an array index or object key is out of bounds / not found in strict access.
 */
class out_of_range : public exception {
public:
    explicit out_of_range(std::string message)
        : exception("[corejson::out_of_range] " + std::move(message)) {}
};

/**
 * @brief Exception thrown for invalid JSON Pointer syntax or unresolved path.
 */
class pointer_error : public exception {
public:
    explicit pointer_error(std::string message)
        : exception("[corejson::pointer_error] " + std::move(message)) {}
};

} // namespace corejson
