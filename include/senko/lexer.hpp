#pragma once

#include "fwd.hpp"
#include "error.hpp"
#include "value.hpp"

#include <string_view>
#include <string>
#include <cstdint>
#include <cctype>
#include <charconv>
#include <sstream>
#include <limits>
#include <cstring>

namespace senko {

enum class token_type : uint8_t {
    end_of_input = 0,
    curly_open,       // {
    curly_close,      // }
    bracket_open,     // [
    bracket_close,    // ]
    colon,            // :
    comma,            // ,
    string_lit,       // "..."
    number_lit,       // 123, -4.5e6
    kw_true,          // true
    kw_false,         // false
    kw_null           // null
};

class lexer {
public:
    explicit lexer(std::string_view source, bool allow_comments = false)
        : m_src(source), m_pos(0), m_line(1), m_col(1), m_allow_comments(allow_comments) {}

    size_t line() const noexcept { return m_line; }
    size_t column() const noexcept { return m_col; }
    size_t offset() const noexcept { return m_pos; }

    bool has_more() const noexcept { return m_pos < m_src.size(); }

    void skip_whitespace_and_comments() {
        if (!m_allow_comments) {
            while (m_pos < m_src.size()) {
                char c = m_src[m_pos];
                if (c == ' ' || c == '\t' || c == '\r') {
                    m_col++;
                    m_pos++;
                } else if (c == '\n') {
                    m_pos++;
                    m_line++;
                    m_col = 1;
                } else {
                    break;
                }
            }
            return;
        }

        while (m_pos < m_src.size()) {
            char c = m_src[m_pos];
            if (c == ' ' || c == '\t' || c == '\r') {
                advance_char();
            } else if (c == '\n') {
                m_pos++;
                m_line++;
                m_col = 1;
            } else if (c == '/' && m_pos + 1 < m_src.size()) {
                char next = m_src[m_pos + 1];
                if (next == '/') {
                    // Line comment
                    advance_char(); advance_char();
                    while (m_pos < m_src.size() && m_src[m_pos] != '\n') {
                        advance_char();
                    }
                } else if (next == '*') {
                    // Block comment
                    advance_char(); advance_char();
                    bool closed = false;
                    while (m_pos + 1 < m_src.size()) {
                        if (m_src[m_pos] == '*' && m_src[m_pos + 1] == '/') {
                            advance_char(); advance_char();
                            closed = true;
                            break;
                        }
                        if (m_src[m_pos] == '\n') {
                            m_pos++; m_line++; m_col = 1;
                        } else {
                            advance_char();
                        }
                    }
                    if (!closed) {
                        throw_parse_error("Unclosed block comment");
                    }
                } else {
                    break;
                }
            } else {
                break;
            }
        }
    }

    char peek() {
        skip_whitespace_and_comments();
        if (m_pos >= m_src.size()) return '\0';
        return m_src[m_pos];
    }

    char consume() {
        if (m_pos >= m_src.size()) return '\0';
        char c = m_src[m_pos];
        if (c == '\n') {
            m_line++;
            m_col = 1;
        } else {
            m_col++;
        }
        m_pos++;
        return c;
    }

    char get() {
        skip_whitespace_and_comments();
        return consume();
    }

    [[noreturn]] void throw_parse_error(std::string_view msg, size_t err_pos = static_cast<size_t>(-1)) const {
        size_t pos = (err_pos == static_cast<size_t>(-1)) ? m_pos : err_pos;
        
        // Calculate line and col for snippet
        size_t l = 1, c = 1, line_start = 0;
        for (size_t i = 0; i < pos && i < m_src.size(); ++i) {
            if (m_src[i] == '\n') {
                l++; c = 1; line_start = i + 1;
            } else {
                c++;
            }
        }
        size_t line_end = m_src.find('\n', line_start);
        if (line_end == std::string_view::npos) line_end = m_src.size();
        std::string_view snippet = m_src.substr(line_start, line_end - line_start);

        throw parse_error(msg, l, c, pos, snippet);
    }

    void expect_keyword(std::string_view kw) {
        if (m_pos + kw.size() > m_src.size() || m_src.substr(m_pos, kw.size()) != kw) {
            throw_parse_error("Expected keyword '" + std::string(kw) + "'");
        }
        m_pos += kw.size();
        m_col += kw.size();
    }

    std::string parse_string() {
        size_t start_pos = m_pos;
        if (m_pos >= m_src.size() || m_src[m_pos] != '"') {
            throw_parse_error("Expected '\"' at start of string");
        }
        m_col++;
        m_pos++; // skip opening "

        std::string result;
        size_t chunk_start = m_pos;

        while (m_pos < m_src.size()) {
            unsigned char c = static_cast<unsigned char>(m_src[m_pos]);
            if (c == '"') {
                if (m_pos > chunk_start) {
                    result.append(m_src.data() + chunk_start, m_pos - chunk_start);
                }
                m_col++;
                m_pos++; // skip closing "
                return result;
            }
            if (c == '\\') {
                if (m_pos > chunk_start) {
                    result.append(m_src.data() + chunk_start, m_pos - chunk_start);
                }
                m_col++;
                m_pos++; // skip '\'
                if (m_pos >= m_src.size()) {
                    throw_parse_error("Unexpected end of input inside escape sequence");
                }
                char esc = m_src[m_pos];
                m_col++;
                m_pos++;
                switch (esc) {
                    case '"':  result.push_back('"'); break;
                    case '\\': result.push_back('\\'); break;
                    case '/':  result.push_back('/'); break;
                    case 'b':  result.push_back('\b'); break;
                    case 'f':  result.push_back('\f'); break;
                    case 'n':  result.push_back('\n'); break;
                    case 'r':  result.push_back('\r'); break;
                    case 't':  result.push_back('\t'); break;
                    case 'u': {
                        uint32_t codepoint = parse_hex4();
                        // Check for UTF-16 surrogate pair
                        if (codepoint >= 0xD800 && codepoint <= 0xDBFF) {
                            // High surrogate, expect low surrogate \uDC00 - \uDFFF
                            if (m_pos + 6 <= m_src.size() && m_src[m_pos] == '\\' && m_src[m_pos + 1] == 'u') {
                                m_col += 2;
                                m_pos += 2;
                                uint32_t low = parse_hex4();
                                if (low >= 0xDC00 && low <= 0xDFFF) {
                                    codepoint = 0x10000 + ((codepoint - 0xD800) << 10) + (low - 0xDC00);
                                } else {
                                    throw_parse_error("Invalid low surrogate in Unicode escape sequence");
                                }
                            } else {
                                throw_parse_error("Missing low surrogate in Unicode escape sequence");
                            }
                        } else if (codepoint >= 0xDC00 && codepoint <= 0xDFFF) {
                            throw_parse_error("Unexpected lone low surrogate in Unicode escape sequence");
                        }
                        append_utf8(result, codepoint);
                        break;
                    }
                    default:
                        throw_parse_error(std::string("Invalid escape character '\\") + esc + "'");
                }
                chunk_start = m_pos;
                continue;
            }
            if (c < 0x20) {
                throw_parse_error("Unescaped control character in string");
            }
            if (c == '\n') {
                m_line++;
                m_col = 1;
            } else {
                m_col++;
            }
            m_pos++;
        }

        throw_parse_error("Unterminated string literal", start_pos);
    }

    value parse_number() {
        size_t start_pos = m_pos;
        bool is_negative = false;
        bool is_float = false;

        if (m_src[m_pos] == '-') {
            is_negative = true;
            m_col++;
            m_pos++;
            if (m_pos >= m_src.size() || !std::isdigit(static_cast<unsigned char>(m_src[m_pos]))) {
                throw_parse_error("Expected digit after minus sign in number");
            }
        }

        if (m_src[m_pos] == '0') {
            m_col++;
            m_pos++;
            if (m_pos < m_src.size() && std::isdigit(static_cast<unsigned char>(m_src[m_pos]))) {
                throw_parse_error("Leading zeros are not permitted in JSON numbers");
            }
        } else if (std::isdigit(static_cast<unsigned char>(m_src[m_pos]))) {
            while (m_pos < m_src.size() && std::isdigit(static_cast<unsigned char>(m_src[m_pos]))) {
                m_col++;
                m_pos++;
            }
        } else {
            throw_parse_error("Invalid character in number literal");
        }

        // Fractional part
        if (m_pos < m_src.size() && m_src[m_pos] == '.') {
            is_float = true;
            m_col++;
            m_pos++;
            if (m_pos >= m_src.size() || !std::isdigit(static_cast<unsigned char>(m_src[m_pos]))) {
                throw_parse_error("Expected digit after decimal point");
            }
            while (m_pos < m_src.size() && std::isdigit(static_cast<unsigned char>(m_src[m_pos]))) {
                m_col++;
                m_pos++;
            }
        }

        // Exponent part
        if (m_pos < m_src.size() && (m_src[m_pos] == 'e' || m_src[m_pos] == 'E')) {
            is_float = true;
            m_col++;
            m_pos++;
            if (m_pos < m_src.size() && (m_src[m_pos] == '+' || m_src[m_pos] == '-')) {
                m_col++;
                m_pos++;
            }
            if (m_pos >= m_src.size() || !std::isdigit(static_cast<unsigned char>(m_src[m_pos]))) {
                throw_parse_error("Expected digit in exponent");
            }
            while (m_pos < m_src.size() && std::isdigit(static_cast<unsigned char>(m_src[m_pos]))) {
                m_col++;
                m_pos++;
            }
        }

        std::string_view num_str = m_src.substr(start_pos, m_pos - start_pos);

        if (is_float) {
            double d = 0.0;
            auto [ptr, ec] = std::from_chars(num_str.data(), num_str.data() + num_str.size(), d);
            if (ec == std::errc()) {
                return value(d);
            }
            // Fast stack buffer fallback to avoid heap allocation
            char stack_buf[64];
            if (num_str.size() < sizeof(stack_buf)) {
                std::memcpy(stack_buf, num_str.data(), num_str.size());
                stack_buf[num_str.size()] = '\0';
                return value(std::strtod(stack_buf, nullptr));
            }
            std::string temp(num_str);
            return value(std::strtod(temp.c_str(), nullptr));
        }

        if (is_negative) {
            int64_t val = 0;
            auto [ptr, ec] = std::from_chars(num_str.data(), num_str.data() + num_str.size(), val);
            if (ec == std::errc()) {
                return value(val);
            }
            // Fallback to double on overflow without heap allocation
            char stack_buf[64];
            if (num_str.size() < sizeof(stack_buf)) {
                std::memcpy(stack_buf, num_str.data(), num_str.size());
                stack_buf[num_str.size()] = '\0';
                return value(std::strtod(stack_buf, nullptr));
            }
            std::string temp(num_str);
            return value(std::strtod(temp.c_str(), nullptr));
        } else {
            uint64_t uval = 0;
            auto [ptr, ec] = std::from_chars(num_str.data(), num_str.data() + num_str.size(), uval);
            if (ec == std::errc()) {
                if (uval <= static_cast<uint64_t>((std::numeric_limits<int64_t>::max)())) {
                    return value(static_cast<int64_t>(uval));
                }
                return value(uval);
            }
            // Fallback to double on overflow without heap allocation
            char stack_buf[64];
            if (num_str.size() < sizeof(stack_buf)) {
                std::memcpy(stack_buf, num_str.data(), num_str.size());
                stack_buf[num_str.size()] = '\0';
                return value(std::strtod(stack_buf, nullptr));
            }
            std::string temp(num_str);
            return value(std::strtod(temp.c_str(), nullptr));
        }
    }

private:
    std::string_view m_src;
    size_t m_pos;
    size_t m_line;
    size_t m_col;
    bool m_allow_comments;

    void advance_char() {
        if (m_pos < m_src.size()) {
            if (m_src[m_pos] == '\n') {
                m_line++;
                m_col = 1;
            } else {
                m_col++;
            }
            m_pos++;
        }
    }

    uint32_t parse_hex4() {
        if (m_pos + 4 > m_src.size()) {
            throw_parse_error("Unexpected end of input in \\u escape");
        }
        uint32_t val = 0;
        for (int i = 0; i < 4; ++i) {
            char c = m_src[m_pos];
            advance_char();
            val <<= 4;
            if (c >= '0' && c <= '9') val |= (c - '0');
            else if (c >= 'a' && c <= 'f') val |= (c - 'a' + 10);
            else if (c >= 'A' && c <= 'F') val |= (c - 'A' + 10);
            else throw_parse_error("Invalid hex digit in \\u escape sequence");
        }
        return val;
    }

    static void append_utf8(std::string& out, uint32_t cp) {
        if (cp <= 0x7F) {
            out.push_back(static_cast<char>(cp));
        } else if (cp <= 0x7FF) {
            out.push_back(static_cast<char>(0xC0 | (cp >> 6)));
            out.push_back(static_cast<char>(0x80 | (cp & 0x3F)));
        } else if (cp <= 0xFFFF) {
            out.push_back(static_cast<char>(0xE0 | (cp >> 12)));
            out.push_back(static_cast<char>(0x80 | ((cp >> 6) & 0x3F)));
            out.push_back(static_cast<char>(0x80 | (cp & 0x3F)));
        } else if (cp <= 0x10FFFF) {
            out.push_back(static_cast<char>(0xF0 | (cp >> 18)));
            out.push_back(static_cast<char>(0x80 | ((cp >> 12) & 0x3F)));
            out.push_back(static_cast<char>(0x80 | ((cp >> 6) & 0x3F)));
            out.push_back(static_cast<char>(0x80 | (cp & 0x3F)));
        }
    }
};

} // namespace senko
