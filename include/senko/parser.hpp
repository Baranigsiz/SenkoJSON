#pragma once

#include "fwd.hpp"
#include "error.hpp"
#include "value.hpp"
#include "lexer.hpp"

#include <string_view>
#include <istream>
#include <fstream>
#include <sstream>

namespace senko {

class parser {
public:
    static constexpr size_t max_depth = 512;

    explicit parser(std::string_view src, bool allow_comments = false, bool allow_trailing_comma = false)
        : m_lexer(src, allow_comments), m_allow_trailing_comma(allow_trailing_comma) {}

    value parse() {
        m_lexer.skip_whitespace_and_comments();
        if (!m_lexer.has_more() || m_lexer.peek() == '\0') {
            m_lexer.throw_parse_error("Empty input");
        }

        value result = parse_value(0);

        m_lexer.skip_whitespace_and_comments();
        if (m_lexer.has_more() && m_lexer.peek() != '\0') {
            m_lexer.throw_parse_error("Unexpected trailing characters after JSON root");
        }

        return result;
    }

private:
    lexer m_lexer;
    bool m_allow_trailing_comma;

    value parse_value(size_t depth) {
        if (depth > max_depth) {
            m_lexer.throw_parse_error("Maximum JSON nesting depth exceeded (potential stack overflow)");
        }

        char c = m_lexer.peek();
        switch (c) {
            case '{': return parse_object(depth + 1);
            case '[': return parse_array(depth + 1);
            case '"': return value(m_lexer.parse_string());
            case 't': return parse_true();
            case 'f': return parse_false();
            case 'n': return parse_null();
            case '-':
            case '0': case '1': case '2': case '3': case '4':
            case '5': case '6': case '7': case '8': case '9':
                return m_lexer.parse_number();
            default:
                m_lexer.throw_parse_error(std::string("Unexpected token '") + (c == '\0' ? "EOF" : std::string(1, c)) + "'");
        }
    }

    value parse_object(size_t depth) {
        m_lexer.consume(); // consume '{'
        value::object_t obj;

        char c = m_lexer.peek();
        if (c == '}') {
            m_lexer.consume(); // consume '}'
            return value(std::move(obj));
        }

        obj.reserve(4);

        while (true) {
            if (m_lexer.peek() != '"') {
                m_lexer.throw_parse_error("Expected string key in object");
            }
            std::string key = m_lexer.parse_string();

            if (m_lexer.peek() != ':') {
                m_lexer.throw_parse_error("Expected ':' after object key");
            }
            m_lexer.consume(); // consume ':'

            value val = parse_value(depth);
            obj.emplace_back(std::move(key), std::move(val));

            char next = m_lexer.peek();
            if (next == ',') {
                m_lexer.consume(); // consume ','
                if (m_lexer.peek() == '}') {
                    if (m_allow_trailing_comma) {
                        m_lexer.consume(); // consume '}'
                        break;
                    } else {
                        m_lexer.throw_parse_error("Trailing comma is not allowed in standard JSON");
                    }
                }
            } else if (next == '}') {
                m_lexer.consume(); // consume '}'
                break;
            } else {
                m_lexer.throw_parse_error("Expected ',' or '}' in object");
            }
        }

        return value(std::move(obj));
    }

    value parse_array(size_t depth) {
        m_lexer.consume(); // consume '['
        value::array_t arr;

        char c = m_lexer.peek();
        if (c == ']') {
            m_lexer.consume(); // consume ']'
            return value(std::move(arr));
        }

        arr.reserve(8);

        while (true) {
            arr.push_back(parse_value(depth));

            char next = m_lexer.peek();
            if (next == ',') {
                m_lexer.consume(); // consume ','
                if (m_lexer.peek() == ']') {
                    if (m_allow_trailing_comma) {
                        m_lexer.consume(); // consume ']'
                        break;
                    } else {
                        m_lexer.throw_parse_error("Trailing comma is not allowed in standard JSON");
                    }
                }
            } else if (next == ']') {
                m_lexer.consume(); // consume ']'
                break;
            } else {
                m_lexer.throw_parse_error("Expected ',' or ']' in array");
            }
        }

        return value(std::move(arr));
    }

    value parse_true() {
        if (m_lexer.get() == 't' && m_lexer.get() == 'r' && m_lexer.get() == 'u' && m_lexer.get() == 'e') {
            return value(true);
        }
        m_lexer.throw_parse_error("Invalid keyword (expected 'true')");
    }

    value parse_false() {
        if (m_lexer.get() == 'f' && m_lexer.get() == 'a' && m_lexer.get() == 'l' && m_lexer.get() == 's' && m_lexer.get() == 'e') {
            return value(false);
        }
        m_lexer.throw_parse_error("Invalid keyword (expected 'false')");
    }

    value parse_null() {
        if (m_lexer.get() == 'n' && m_lexer.get() == 'u' && m_lexer.get() == 'l' && m_lexer.get() == 'l') {
            return value(nullptr);
        }
        m_lexer.throw_parse_error("Invalid keyword (expected 'null')");
    }
};

// Inline implementations of value::parse
inline value value::parse(std::string_view input, bool allow_comments, bool allow_trailing_comma) {
    parser p(input, allow_comments, allow_trailing_comma);
    return p.parse();
}

inline value value::parse(std::istream& is, bool allow_comments, bool allow_trailing_comma) {
    std::string str((std::istreambuf_iterator<char>(is)), std::istreambuf_iterator<char>());
    return parse(str, allow_comments, allow_trailing_comma);
}

inline value value::parse_file(const std::string& filepath, bool allow_comments, bool allow_trailing_comma) {
    std::ifstream file(filepath, std::ios::in | std::ios::binary);
    if (!file.is_open()) {
        throw parse_error("Failed to open file: " + filepath);
    }
    return parse(file, allow_comments, allow_trailing_comma);
}

} // namespace senko
