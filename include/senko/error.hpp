#pragma once

#include <exception>
#include <string>
#include <sstream>
#include <string_view>
#include <algorithm>

namespace senko {

/**
 * @brief Base exception class for all Senko errors.
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
    explicit parse_error(std::string msg)
        : exception("[senko::parse_error] " + std::move(msg)),
          m_line(0), m_column(0), m_byte_offset(0) {}

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
        ss << "[senko::parse_error] " << raw_msg
           << " at line " << line << ", column " << col << " (byte offset " << offset << ")";
        if (!snippet.empty()) {
            std::string l_str = std::to_string(line);
            std::string pad = l_str.size() < 4 ? std::string(4 - l_str.size(), ' ') : "";
            ss << "\n      |\n";
            ss << " " << pad << l_str << " | " << snippet << "\n";
            ss << "      | " << std::string(col > 0 ? col - 1 : 0, ' ') << "^~~~";
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
        : exception("[senko::type_error] " + std::move(message)) {}
};

/**
 * @brief Exception thrown when an array index or object key is out of bounds / not found in strict access.
 */
class out_of_range : public exception {
public:
    explicit out_of_range(std::string message)
        : exception("[senko::out_of_range] " + std::move(message)) {}
};

/**
 * @brief Exception thrown for invalid JSON Pointer syntax or unresolved path.
 */
class pointer_error : public exception {
public:
    explicit pointer_error(std::string message)
        : exception("[senko::pointer_error] " + std::move(message)) {}
};

/**
 * @brief Exception thrown when serializing or writing to files fails.
 */
class serializer_error : public exception {
public:
    explicit serializer_error(std::string message)
        : exception("[senko::serializer_error] " + std::move(message)) {}
};

} // namespace senko
