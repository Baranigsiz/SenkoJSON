#pragma once

#include "fwd.hpp"
#include "value.hpp"

#include <string>
#include <sstream>
#include <iomanip>
#include <cmath>
#include <limits>

namespace senko {

class serializer {
public:
    explicit serializer(std::ostream& os, int indent = -1)
        : m_os(os), m_indent(indent), m_depth(0) {}

    void dump(const value& v) {
        switch (v.type()) {
            case value_t::null:
                m_os << "null";
                break;
            case value_t::boolean:
                m_os << (v.get<bool>() ? "true" : "false");
                break;
            case value_t::number_integer:
                m_os << v.get<int64_t>();
                break;
            case value_t::number_unsigned:
                m_os << v.get<uint64_t>();
                break;
            case value_t::number_float: {
                double d = v.get<double>();
                if (std::isnan(d) || std::isinf(d)) {
                    m_os << "null"; // RFC 8259 requires NaN/Infinity to be serialized as null
                } else {
                    std::ostringstream ss;
                    ss << std::setprecision(std::numeric_limits<double>::max_digits10) << d;
                    std::string s = ss.str();
                    // Ensure float contains '.' or 'e' to distinguish from integer
                    if (s.find('.') == std::string::npos && s.find('e') == std::string::npos && s.find('E') == std::string::npos) {
                        s += ".0";
                    }
                    m_os << s;
                }
                break;
            }
            case value_t::string:
                dump_string(v.get_ref_string());
                break;
            case value_t::array:
                dump_array(v.get_ref_array());
                break;
            case value_t::object:
                dump_object(v.get_ref_object());
                break;
        }
    }

    static std::string dump_to_string(const value& v, int indent = -1) {
        std::ostringstream ss;
        serializer s(ss, indent);
        s.dump(v);
        return ss.str();
    }

private:
    std::ostream& m_os;
    int m_indent;
    int m_depth;

    void indent_newline() {
        if (m_indent >= 0) {
            m_os << "\n" << std::string(static_cast<size_t>(m_depth * m_indent), ' ');
        }
    }

    void dump_string(std::string_view sv) {
        m_os << '"';
        for (size_t i = 0; i < sv.size(); ++i) {
            unsigned char c = static_cast<unsigned char>(sv[i]);
            switch (c) {
                case '"':  m_os << "\\\""; break;
                case '\\': m_os << "\\\\"; break;
                case '\b': m_os << "\\b"; break;
                case '\f': m_os << "\\f"; break;
                case '\n': m_os << "\\n"; break;
                case '\r': m_os << "\\r"; break;
                case '\t': m_os << "\\t"; break;
                default:
                    if (c < 0x20) {
                        // Escape control characters as \u00XX
                        m_os << "\\u"
                             << std::hex << std::setw(4) << std::setfill('0')
                             << static_cast<int>(c)
                             << std::dec;
                    } else {
                        m_os << static_cast<char>(c);
                    }
                    break;
            }
        }
        m_os << '"';
    }

    void dump_array(const value::array_t& arr) {
        if (arr.empty()) {
            m_os << "[]";
            return;
        }

        m_os << '[';
        m_depth++;

        for (size_t i = 0; i < arr.size(); ++i) {
            indent_newline();
            dump(arr[i]);
            if (i + 1 < arr.size()) {
                m_os << ',';
            }
        }

        m_depth--;
        indent_newline();
        m_os << ']';
    }

    void dump_object(const value::object_t& obj) {
        if (obj.empty()) {
            m_os << "{}";
            return;
        }

        m_os << '{';
        m_depth++;

        for (size_t i = 0; i < obj.size(); ++i) {
            indent_newline();
            dump_string(obj[i].first);
            m_os << (m_indent >= 0 ? ": " : ":");
            dump(obj[i].second);
            if (i + 1 < obj.size()) {
                m_os << ',';
            }
        }

        m_depth--;
        indent_newline();
        m_os << '}';
    }
};

// Inline implementations of value::dump and operator<<
inline std::string value::dump(int indent) const {
    return serializer::dump_to_string(*this, indent);
}

inline void value::dump(std::ostream& os, int indent) const {
    serializer s(os, indent);
    s.dump(*this);
}

inline std::ostream& operator<<(std::ostream& os, const value& j) {
    j.dump(os, -1);
    return os;
}

} // namespace senko
