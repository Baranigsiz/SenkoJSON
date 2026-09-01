/**
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


// ========================================================
// Header: fwd.hpp
// ========================================================



#include <cstdint>
#include <cstddef>
#include <string>
#include <vector>
#include <utility>
#include <type_traits>
#include <string_view>
#include <iosfwd>

namespace corejson {

// Forward declarations
class value;
using json = value;
class json_pointer;

/**
 * @brief Represents the JSON data type of a value.
 */
enum class value_t : uint8_t {
    null = 0,
    boolean,
    number_integer,
    number_unsigned,
    number_float,
    string,
    array,
    object
};

/**
 * @brief Returns the human-readable string name of a JSON type.
 */
inline constexpr std::string_view to_string(value_t t) noexcept {
    switch (t) {
        case value_t::null: return "null";
        case value_t::boolean: return "boolean";
        case value_t::number_integer: return "number (integer)";
        case value_t::number_unsigned: return "number (unsigned)";
        case value_t::number_float: return "number (float)";
        case value_t::string: return "string";
        case value_t::array: return "array";
        case value_t::object: return "object";
    }
    return "unknown";
}

// ADL helper tag
template <typename T, typename SFINAE = void>
struct adl_serializer;

} // namespace corejson


// ========================================================
// Header: error.hpp
// ========================================================



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


// ========================================================
// Header: value.hpp
// ========================================================






#include <variant>
#include <string>
#include <vector>
#include <utility>
#include <algorithm>
#include <type_traits>
#include <iostream>
#include <initializer_list>
#include <memory>

namespace corejson {

class value {
public:
    using array_t = std::vector<value>;
    using object_t = std::vector<std::pair<std::string, value>>;

private:
    using variant_t = std::variant<
        std::nullptr_t,
        bool,
        int64_t,
        uint64_t,
        double,
        std::string,
        array_t,
        object_t
    >;

    variant_t m_data;

public:
    // ==========================================
    // Constructors & Assignment
    // ==========================================

    value() noexcept : m_data(nullptr) {}
    value(std::nullptr_t) noexcept : m_data(nullptr) {}
    value(bool b) noexcept : m_data(b) {}

    // Signed integers
    template <typename T, typename std::enable_if_t<std::is_integral_v<T> && std::is_signed_v<T> && !std::is_same_v<T, bool>, int> = 0>
    value(T val) noexcept : m_data(static_cast<int64_t>(val)) {}

    // Unsigned integers
    template <typename T, typename std::enable_if_t<std::is_integral_v<T> && std::is_unsigned_v<T> && !std::is_same_v<T, bool>, int> = 0>
    value(T val) noexcept : m_data(static_cast<uint64_t>(val)) {}

    // Floating point
    template <typename T, typename std::enable_if_t<std::is_floating_point_v<T>, int> = 0>
    value(T val) noexcept : m_data(static_cast<double>(val)) {}

    // Strings
    value(const char* s) : m_data(s ? std::string(s) : std::string{}) {}
    value(std::string s) : m_data(std::move(s)) {}
    value(std::string_view sv) : m_data(std::string(sv)) {}

    // Structured types
    value(array_t arr) : m_data(std::move(arr)) {}
    value(object_t obj) : m_data(std::move(obj)) {}

    // Construct empty container of given type
    explicit value(value_t t) {
        switch (t) {
            case value_t::null: m_data = nullptr; break;
            case value_t::boolean: m_data = false; break;
            case value_t::number_integer: m_data = int64_t{0}; break;
            case value_t::number_unsigned: m_data = uint64_t{0}; break;
            case value_t::number_float: m_data = 0.0; break;
            case value_t::string: m_data = std::string{}; break;
            case value_t::array: m_data = array_t{}; break;
            case value_t::object: m_data = object_t{}; break;
        }
    }

    // Direct reference getters for high performance
    std::string& get_ref_string() {
        if (!is_string()) throw type_error("Expected string, got " + std::string(type_name()));
        return std::get<std::string>(m_data);
    }
    const std::string& get_ref_string() const {
        if (!is_string()) throw type_error("Expected string, got " + std::string(type_name()));
        return std::get<std::string>(m_data);
    }

    array_t& get_ref_array() {
        if (!is_array()) throw type_error("Expected array, got " + std::string(type_name()));
        return std::get<array_t>(m_data);
    }
    const array_t& get_ref_array() const {
        if (!is_array()) throw type_error("Expected array, got " + std::string(type_name()));
        return std::get<array_t>(m_data);
    }

    object_t& get_ref_object() {
        if (!is_object()) throw type_error("Expected object, got " + std::string(type_name()));
        return std::get<object_t>(m_data);
    }
    const object_t& get_ref_object() const {
        if (!is_object()) throw type_error("Expected object, got " + std::string(type_name()));
        return std::get<object_t>(m_data);
    }

    // Initializer list constructor (Smart Object / Array detection)
    value(std::initializer_list<value> init) {
        bool is_object_like = (init.size() > 0);
        for (const auto& elem : init) {
            if (!elem.is_array() || elem.get_ref_array().size() != 2 || !elem.get_ref_array()[0].is_string()) {
                is_object_like = false;
                break;
            }
        }

        if (is_object_like) {
            object_t obj;
            obj.reserve(init.size());
            for (const auto& elem : init) {
                const auto& sub_arr = elem.get_ref_array();
                obj.emplace_back(sub_arr[0].get_ref_string(), sub_arr[1]);
            }
            m_data = std::move(obj);
        } else {
            m_data = array_t(init.begin(), init.end());
        }
    }

    // Static factories
    static value array() { return value(value_t::array); }
    static value array(std::initializer_list<value> init) {
        array_t arr(init.begin(), init.end());
        return value(std::move(arr));
    }
    static value object() { return value(value_t::object); }
    static value object(std::initializer_list<std::pair<std::string, value>> init) {
        object_t obj;
        obj.reserve(init.size());
        for (const auto& pair : init) {
            obj.push_back(pair);
        }
        return value(std::move(obj));
    }

    // Generic ADL constructor (strictly enabled ONLY for custom user types)
    template <typename T, typename std::enable_if_t<
        !std::is_same_v<std::decay_t<T>, value> &&
        !std::is_arithmetic_v<std::decay_t<T>> &&
        !std::is_same_v<std::decay_t<T>, std::string> &&
        !std::is_same_v<std::decay_t<T>, std::string_view> &&
        !std::is_same_v<std::decay_t<T>, const char*> &&
        !std::is_same_v<std::decay_t<T>, char*> &&
        !std::is_same_v<std::decay_t<T>, std::nullptr_t> &&
        !std::is_same_v<std::decay_t<T>, array_t> &&
        !std::is_same_v<std::decay_t<T>, object_t>, int> = 0>
    value(const T& custom) {
        adl_serializer<T>::serialize(*this, custom);
    }

    template <typename T, typename std::enable_if_t<
        !std::is_same_v<std::decay_t<T>, value>, int> = 0>
    value& operator=(T&& val) {
        *this = value(std::forward<T>(val));
        return *this;
    }

    // ==========================================
    // Type Inspection
    // ==========================================

    value_t type() const noexcept {
        switch (m_data.index()) {
            case 0: return value_t::null;
            case 1: return value_t::boolean;
            case 2: return value_t::number_integer;
            case 3: return value_t::number_unsigned;
            case 4: return value_t::number_float;
            case 5: return value_t::string;
            case 6: return value_t::array;
            case 7: return value_t::object;
            default: return value_t::null;
        }
    }

    std::string_view type_name() const noexcept {
        return to_string(type());
    }

    bool is_null() const noexcept { return std::holds_alternative<std::nullptr_t>(m_data); }
    bool is_boolean() const noexcept { return std::holds_alternative<bool>(m_data); }
    bool is_number_integer() const noexcept { return std::holds_alternative<int64_t>(m_data); }
    bool is_number_unsigned() const noexcept { return std::holds_alternative<uint64_t>(m_data); }
    bool is_number_float() const noexcept { return std::holds_alternative<double>(m_data); }
    bool is_number() const noexcept {
        return is_number_integer() || is_number_unsigned() || is_number_float();
    }
    bool is_string() const noexcept { return std::holds_alternative<std::string>(m_data); }
    bool is_array() const noexcept { return std::holds_alternative<array_t>(m_data); }
    bool is_object() const noexcept { return std::holds_alternative<object_t>(m_data); }
    bool is_primitive() const noexcept { return is_null() || is_boolean() || is_number() || is_string(); }
    bool is_structured() const noexcept { return is_array() || is_object(); }

    // ==========================================
    // Value Accessors (get, get_to, value)
    // ==========================================

    template <typename T>
    T get() const {
        if constexpr (std::is_same_v<T, bool>) {
            if (!is_boolean()) throw type_error("Expected boolean, got " + std::string(type_name()));
            return std::get<bool>(m_data);
        } else if constexpr (std::is_integral_v<T> && std::is_signed_v<T>) {
            if (is_number_integer()) return static_cast<T>(std::get<int64_t>(m_data));
            if (is_number_unsigned()) return static_cast<T>(std::get<uint64_t>(m_data));
            if (is_number_float()) return static_cast<T>(std::get<double>(m_data));
            throw type_error("Expected integer number, got " + std::string(type_name()));
        } else if constexpr (std::is_integral_v<T> && std::is_unsigned_v<T>) {
            if (is_number_unsigned()) return static_cast<T>(std::get<uint64_t>(m_data));
            if (is_number_integer()) {
                int64_t v = std::get<int64_t>(m_data);
                if (v < 0) throw type_error("Cannot convert negative integer to unsigned");
                return static_cast<T>(v);
            }
            if (is_number_float()) return static_cast<T>(std::get<double>(m_data));
            throw type_error("Expected unsigned number, got " + std::string(type_name()));
        } else if constexpr (std::is_floating_point_v<T>) {
            if (is_number_float()) return static_cast<T>(std::get<double>(m_data));
            if (is_number_integer()) return static_cast<T>(std::get<int64_t>(m_data));
            if (is_number_unsigned()) return static_cast<T>(std::get<uint64_t>(m_data));
            throw type_error("Expected floating-point number, got " + std::string(type_name()));
        } else if constexpr (std::is_same_v<T, std::string>) {
            if (!is_string()) throw type_error("Expected string, got " + std::string(type_name()));
            return std::get<std::string>(m_data);
        } else if constexpr (std::is_same_v<T, std::string_view>) {
            if (!is_string()) throw type_error("Expected string, got " + std::string(type_name()));
            return std::string_view(std::get<std::string>(m_data));
        } else if constexpr (std::is_same_v<T, array_t>) {
            if (!is_array()) throw type_error("Expected array, got " + std::string(type_name()));
            return std::get<array_t>(m_data);
        } else if constexpr (std::is_same_v<T, object_t>) {
            if (!is_object()) throw type_error("Expected object, got " + std::string(type_name()));
            return std::get<object_t>(m_data);
        } else if constexpr (std::is_same_v<T, value>) {
            return *this;
        } else {
            T target{};
            adl_serializer<T>::deserialize(*this, target);
            return target;
        }
    }

    template <typename T>
    void get_to(T& val) const {
        val = get<T>();
    }

    template <typename T>
    T value_or(std::string_view key, T default_value) const {
        if (!is_object()) return default_value;
        const auto& obj = std::get<object_t>(m_data);
        for (const auto& pair : obj) {
            if (pair.first == key) {
                try {
                    return pair.second.get<T>();
                } catch (...) {
                    return default_value;
                }
            }
        }
        return default_value;
    }

    template <typename T>
    T value_or(size_t index, T default_value) const {
        if (!is_array()) return default_value;
        const auto& arr = std::get<array_t>(m_data);
        if (index >= arr.size()) return default_value;
        try {
            return arr[index].get<T>();
        } catch (...) {
            return default_value;
        }
    }

    // ==========================================
    // Container Operations & Indexing
    // ==========================================

    size_t size() const noexcept {
        if (is_array()) return std::get<array_t>(m_data).size();
        if (is_object()) return std::get<object_t>(m_data).size();
        if (is_null()) return 0;
        return 1;
    }

    bool empty() const noexcept {
        if (is_array()) return std::get<array_t>(m_data).empty();
        if (is_object()) return std::get<object_t>(m_data).empty();
        if (is_null()) return true;
        return false;
    }

    void clear() noexcept {
        if (is_array()) std::get<array_t>(m_data).clear();
        else if (is_object()) std::get<object_t>(m_data).clear();
        else if (is_string()) std::get<std::string>(m_data).clear();
        else m_data = nullptr;
    }

    bool contains(std::string_view key) const noexcept {
        if (!is_object()) return false;
        const auto& obj = std::get<object_t>(m_data);
        for (const auto& pair : obj) {
            if (pair.first == key) return true;
        }
        return false;
    }

    size_t count(std::string_view key) const noexcept {
        return contains(key) ? 1 : 0;
    }

    // Object Indexing (mutable) -> automatically becomes object if null
    value& operator[](std::string_view key) {
        if (is_null()) {
            m_data = object_t{};
        }
        if (!is_object()) {
            throw type_error("Cannot use operator[](string) on non-object type " + std::string(type_name()));
        }
        auto& obj = std::get<object_t>(m_data);
        for (auto& pair : obj) {
            if (pair.first == key) return pair.second;
        }
        obj.emplace_back(std::string(key), value{});
        return obj.back().second;
    }

    // Strict element access (throws out_of_range)
    value& at(std::string_view key) {
        if (!is_object()) {
            throw type_error("Cannot use at(key) on non-object type " + std::string(type_name()));
        }
        auto& obj = std::get<object_t>(m_data);
        for (auto& pair : obj) {
            if (pair.first == key) return pair.second;
        }
        throw out_of_range("Key not found in object: '" + std::string(key) + "'");
    }

    const value& at(std::string_view key) const {
        if (!is_object()) {
            throw type_error("Cannot use at(key) on non-object type " + std::string(type_name()));
        }
        const auto& obj = std::get<object_t>(m_data);
        for (const auto& pair : obj) {
            if (pair.first == key) return pair.second;
        }
        throw out_of_range("Key not found in object: '" + std::string(key) + "'");
    }

    value& at(size_t index) {
        if (!is_array()) {
            throw type_error("Cannot use at(index) on non-array type " + std::string(type_name()));
        }
        auto& arr = std::get<array_t>(m_data);
        if (index >= arr.size()) {
            throw out_of_range("Array index " + std::to_string(index) + " out of range (size: " + std::to_string(arr.size()) + ")");
        }
        return arr[index];
    }

    const value& at(size_t index) const {
        if (!is_array()) {
            throw type_error("Cannot use at(index) on non-array type " + std::string(type_name()));
        }
        const auto& arr = std::get<array_t>(m_data);
        if (index >= arr.size()) {
            throw out_of_range("Array index " + std::to_string(index) + " out of range (size: " + std::to_string(arr.size()) + ")");
        }
        return arr[index];
    }

    // Object Indexing (const)
    const value& operator[](std::string_view key) const {
        return at(key);
    }

    // Array Indexing (mutable) -> automatically becomes array if null
    template <typename Int, typename std::enable_if_t<std::is_integral_v<Int>, int> = 0>
    value& operator[](Int index) {
        if (is_null()) {
            m_data = array_t{};
        }
        if (!is_array()) {
            throw type_error("Cannot use operator[](index) on non-array type " + std::string(type_name()));
        }
        auto& arr = std::get<array_t>(m_data);
        size_t uidx = static_cast<size_t>(index);
        if (uidx >= arr.size()) {
            arr.resize(uidx + 1);
        }
        return arr[uidx];
    }

    template <typename Int, typename std::enable_if_t<std::is_integral_v<Int>, int> = 0>
    const value& operator[](Int index) const {
        return at(static_cast<size_t>(index));
    }

    // Array modifiers
    void push_back(value val) {
        if (is_null()) {
            m_data = array_t{};
        }
        if (!is_array()) {
            throw type_error("Cannot push_back on non-array type " + std::string(type_name()));
        }
        std::get<array_t>(m_data).push_back(std::move(val));
    }

    void emplace_back(value val) {
        push_back(std::move(val));
    }

    // Object modifiers
    void emplace(std::string key, value val) {
        if (is_null()) {
            m_data = object_t{};
        }
        if (!is_object()) {
            throw type_error("Cannot emplace key-value on non-object type " + std::string(type_name()));
        }
        auto& obj = std::get<object_t>(m_data);
        for (auto& pair : obj) {
            if (pair.first == key) {
                pair.second = std::move(val);
                return;
            }
        }
        obj.emplace_back(std::move(key), std::move(val));
    }

    bool erase(std::string_view key) {
        if (!is_object()) return false;
        auto& obj = std::get<object_t>(m_data);
        auto it = std::find_if(obj.begin(), obj.end(), [key](const auto& pair) {
            return pair.first == key;
        });
        if (it != obj.end()) {
            obj.erase(it);
            return true;
        }
        return false;
    }

    bool erase(size_t index) {
        if (!is_array()) return false;
        auto& arr = std::get<array_t>(m_data);
        if (index < arr.size()) {
            arr.erase(arr.begin() + index);
            return true;
        }
        return false;
    }

    // ==========================================
    // Comparisons
    // ==========================================

    bool operator==(const value& other) const {
        if (type() != other.type()) {
            // Compare integer vs unsigned vs float numerically if both are numbers
            if (is_number() && other.is_number()) {
                return get<double>() == other.get<double>();
            }
            return false;
        }
        return m_data == other.m_data;
    }

    bool operator!=(const value& other) const {
        return !(*this == other);
    }

    // ==========================================
    // Serialization & Parsing declarations
    // ==========================================

    std::string dump(int indent = -1) const;
    void dump(std::ostream& os, int indent = -1) const;

    static value parse(std::string_view input, bool allow_comments = false, bool allow_trailing_comma = false);
    static value parse(std::istream& is, bool allow_comments = false, bool allow_trailing_comma = false);

    // JSON Pointer support declarations
    value& at_ptr(const json_pointer& ptr);
    const value& at_ptr(const json_pointer& ptr) const;
    value& operator[](const json_pointer& ptr);
    const value& operator[](const json_pointer& ptr) const;
};

// Stream operator for output
std::ostream& operator<<(std::ostream& os, const value& j);

// Default ADL serializer fallback
template <typename T, typename SFINAE>
struct adl_serializer {
    static void serialize(value& j, const T& val) {
        to_json(j, val);
    }
    static void deserialize(const value& j, T& val) {
        from_json(j, val);
    }
};

} // namespace corejson


// ========================================================
// Header: lexer.hpp
// ========================================================







#include <string_view>
#include <string>
#include <cstdint>
#include <cctype>
#include <charconv>
#include <sstream>

namespace corejson {

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
        while (m_pos < m_src.size()) {
            char c = m_src[m_pos];
            if (c == ' ' || c == '\t' || c == '\r') {
                advance_char();
            } else if (c == '\n') {
                m_pos++;
                m_line++;
                m_col = 1;
            } else if (m_allow_comments && c == '/' && m_pos + 1 < m_src.size()) {
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

    char get() {
        skip_whitespace_and_comments();
        if (m_pos >= m_src.size()) return '\0';
        char c = m_src[m_pos];
        advance_char();
        return c;
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

    std::string parse_string() {
        size_t start_pos = m_pos;
        if (m_pos >= m_src.size() || m_src[m_pos] != '"') {
            throw_parse_error("Expected '\"' at start of string");
        }
        advance_char(); // skip opening "

        std::string result;
        while (m_pos < m_src.size()) {
            char c = m_src[m_pos];
            if (c == '"') {
                advance_char(); // skip closing "
                return result;
            }
            if (static_cast<unsigned char>(c) < 0x20) {
                throw_parse_error("Unescaped control character in string");
            }
            if (c == '\\') {
                advance_char();
                if (m_pos >= m_src.size()) {
                    throw_parse_error("Unexpected end of input inside escape sequence");
                }
                char esc = m_src[m_pos];
                advance_char();
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
                                advance_char(); advance_char();
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
            } else {
                result.push_back(c);
                advance_char();
            }
        }

        throw_parse_error("Unterminated string literal", start_pos);
    }

    value parse_number() {
        size_t start_pos = m_pos;
        bool is_negative = false;
        bool is_float = false;

        if (m_src[m_pos] == '-') {
            is_negative = true;
            advance_char();
            if (m_pos >= m_src.size() || !std::isdigit(static_cast<unsigned char>(m_src[m_pos]))) {
                throw_parse_error("Expected digit after minus sign in number");
            }
        }

        if (m_src[m_pos] == '0') {
            advance_char();
            if (m_pos < m_src.size() && std::isdigit(static_cast<unsigned char>(m_src[m_pos]))) {
                throw_parse_error("Leading zeros are not permitted in JSON numbers");
            }
        } else if (std::isdigit(static_cast<unsigned char>(m_src[m_pos]))) {
            while (m_pos < m_src.size() && std::isdigit(static_cast<unsigned char>(m_src[m_pos]))) {
                advance_char();
            }
        } else {
            throw_parse_error("Invalid character in number literal");
        }

        // Fractional part
        if (m_pos < m_src.size() && m_src[m_pos] == '.') {
            is_float = true;
            advance_char();
            if (m_pos >= m_src.size() || !std::isdigit(static_cast<unsigned char>(m_src[m_pos]))) {
                throw_parse_error("Expected digit after decimal point");
            }
            while (m_pos < m_src.size() && std::isdigit(static_cast<unsigned char>(m_src[m_pos]))) {
                advance_char();
            }
        }

        // Exponent part
        if (m_pos < m_src.size() && (m_src[m_pos] == 'e' || m_src[m_pos] == 'E')) {
            is_float = true;
            advance_char();
            if (m_pos < m_src.size() && (m_src[m_pos] == '+' || m_src[m_pos] == '-')) {
                advance_char();
            }
            if (m_pos >= m_src.size() || !std::isdigit(static_cast<unsigned char>(m_src[m_pos]))) {
                throw_parse_error("Expected digit in exponent");
            }
            while (m_pos < m_src.size() && std::isdigit(static_cast<unsigned char>(m_src[m_pos]))) {
                advance_char();
            }
        }

        std::string_view num_str = m_src.substr(start_pos, m_pos - start_pos);

        if (is_float) {
            std::string temp(num_str);
            char* end_ptr = nullptr;
            double d = std::strtod(temp.c_str(), &end_ptr);
            return value(d);
        }

        if (is_negative) {
            int64_t val = 0;
            auto [ptr, ec] = std::from_chars(num_str.data(), num_str.data() + num_str.size(), val);
            if (ec == std::errc()) {
                return value(val);
            }
            // Fallback to double on overflow
            std::string temp(num_str);
            return value(std::strtod(temp.c_str(), nullptr));
        } else {
            uint64_t uval = 0;
            auto [ptr, ec] = std::from_chars(num_str.data(), num_str.data() + num_str.size(), uval);
            if (ec == std::errc()) {
                if (uval <= static_cast<uint64_t>(INT64_MAX)) {
                    return value(static_cast<int64_t>(uval));
                }
                return value(uval);
            }
            // Fallback to double on overflow
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

} // namespace corejson


// ========================================================
// Header: parser.hpp
// ========================================================








#include <string_view>
#include <istream>
#include <sstream>

namespace corejson {

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
        m_lexer.get(); // consume '{'
        value::object_t obj;

        m_lexer.skip_whitespace_and_comments();
        if (m_lexer.peek() == '}') {
            m_lexer.get(); // consume '}'
            return value(std::move(obj));
        }

        while (true) {
            m_lexer.skip_whitespace_and_comments();
            if (m_lexer.peek() != '"') {
                m_lexer.throw_parse_error("Expected string key in object");
            }
            std::string key = m_lexer.parse_string();

            m_lexer.skip_whitespace_and_comments();
            if (m_lexer.peek() != ':') {
                m_lexer.throw_parse_error("Expected ':' after object key");
            }
            m_lexer.get(); // consume ':'

            value val = parse_value(depth);
            obj.emplace_back(std::move(key), std::move(val));

            m_lexer.skip_whitespace_and_comments();
            char next = m_lexer.peek();
            if (next == ',') {
                m_lexer.get(); // consume ','
                m_lexer.skip_whitespace_and_comments();
                if (m_lexer.peek() == '}') {
                    if (m_allow_trailing_comma) {
                        m_lexer.get(); // consume '}'
                        break;
                    } else {
                        m_lexer.throw_parse_error("Trailing comma is not allowed in standard JSON");
                    }
                }
            } else if (next == '}') {
                m_lexer.get(); // consume '}'
                break;
            } else {
                m_lexer.throw_parse_error("Expected ',' or '}' in object");
            }
        }

        return value(std::move(obj));
    }

    value parse_array(size_t depth) {
        m_lexer.get(); // consume '['
        value::array_t arr;

        m_lexer.skip_whitespace_and_comments();
        if (m_lexer.peek() == ']') {
            m_lexer.get(); // consume ']'
            return value(std::move(arr));
        }

        while (true) {
            arr.push_back(parse_value(depth));

            m_lexer.skip_whitespace_and_comments();
            char next = m_lexer.peek();
            if (next == ',') {
                m_lexer.get(); // consume ','
                m_lexer.skip_whitespace_and_comments();
                if (m_lexer.peek() == ']') {
                    if (m_allow_trailing_comma) {
                        m_lexer.get(); // consume ']'
                        break;
                    } else {
                        m_lexer.throw_parse_error("Trailing comma is not allowed in standard JSON");
                    }
                }
            } else if (next == ']') {
                m_lexer.get(); // consume ']'
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

} // namespace corejson


// ========================================================
// Header: serializer.hpp
// ========================================================






#include <string>
#include <sstream>
#include <iomanip>
#include <cmath>
#include <limits>

namespace corejson {

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

} // namespace corejson


// ========================================================
// Header: json_pointer.hpp
// ========================================================







#include <string>
#include <string_view>
#include <vector>
#include <sstream>

namespace corejson {

/**
 * @brief RFC 6901 JSON Pointer implementation.
 */
class json_pointer {
public:
    json_pointer() = default;

    explicit json_pointer(std::string_view s) {
        if (s.empty()) return;
        if (s[0] != '/') {
            throw pointer_error("JSON Pointer must start with '/' or be empty");
        }

        size_t start = 1;
        while (start <= s.size()) {
            size_t next_slash = s.find('/', start);
            if (next_slash == std::string_view::npos) {
                next_slash = s.size();
            }
            m_tokens.push_back(unescape(s.substr(start, next_slash - start)));
            start = next_slash + 1;
        }
    }

    const std::vector<std::string>& tokens() const noexcept { return m_tokens; }
    bool empty() const noexcept { return m_tokens.empty(); }

    std::string to_string() const {
        if (m_tokens.empty()) return "";
        std::string result;
        for (const auto& tok : m_tokens) {
            result += '/' + escape(tok);
        }
        return result;
    }

    void push_back(std::string token) {
        m_tokens.push_back(std::move(token));
    }

    void pop_back() {
        if (!m_tokens.empty()) m_tokens.pop_back();
    }

    // Resolves pointer against a root JSON value (throws out_of_range / type_error / pointer_error)
    value& resolve(value& root) const {
        value* cur = &root;
        for (const auto& token : m_tokens) {
            if (cur->is_object()) {
                if (!cur->contains(token)) {
                    throw out_of_range("JSON Pointer token not found in object: '" + token + "'");
                }
                cur = &(*cur)[token];
            } else if (cur->is_array()) {
                if (token == "-") {
                    throw pointer_error("Cannot resolve '-' array index token for reading");
                }
                size_t idx = 0;
                try {
                    idx = std::stoull(token);
                } catch (...) {
                    throw pointer_error("Invalid array index in JSON Pointer: '" + token + "'");
                }
                cur = &cur->at(idx);
            } else {
                throw type_error("Cannot navigate through primitive JSON value with token: '" + token + "'");
            }
        }
        return *cur;
    }

    const value& resolve(const value& root) const {
        const value* cur = &root;
        for (const auto& token : m_tokens) {
            if (cur->is_object()) {
                if (!cur->contains(token)) {
                    throw out_of_range("JSON Pointer token not found in object: '" + token + "'");
                }
                cur = &cur->at(token);
            } else if (cur->is_array()) {
                if (token == "-") {
                    throw pointer_error("Cannot resolve '-' array index token for reading");
                }
                size_t idx = 0;
                try {
                    idx = std::stoull(token);
                } catch (...) {
                    throw pointer_error("Invalid array index in JSON Pointer: '" + token + "'");
                }
                cur = &cur->at(idx);
            } else {
                throw type_error("Cannot navigate through primitive JSON value with token: '" + token + "'");
            }
        }
        return *cur;
    }

    static std::string unescape(std::string_view s) {
        std::string res;
        res.reserve(s.size());
        for (size_t i = 0; i < s.size(); ++i) {
            if (s[i] == '~' && i + 1 < s.size()) {
                if (s[i + 1] == '0') {
                    res.push_back('~');
                    i++;
                } else if (s[i + 1] == '1') {
                    res.push_back('/');
                    i++;
                } else {
                    res.push_back(s[i]);
                }
            } else {
                res.push_back(s[i]);
            }
        }
        return res;
    }

    static std::string escape(std::string_view s) {
        std::string res;
        for (char c : s) {
            if (c == '~') res += "~0";
            else if (c == '/') res += "~1";
            else res.push_back(c);
        }
        return res;
    }

private:
    std::vector<std::string> m_tokens;
};

// Implement value pointer helper methods
inline value& value::at_ptr(const json_pointer& ptr) {
    return ptr.resolve(*this);
}

inline const value& value::at_ptr(const json_pointer& ptr) const {
    return ptr.resolve(*this);
}

inline value& value::operator[](const json_pointer& ptr) {
    return ptr.resolve(*this);
}

inline const value& value::operator[](const json_pointer& ptr) const {
    return ptr.resolve(*this);
}

} // namespace corejson


// ========================================================
// Header: macro.hpp
// ========================================================






namespace corejson {

// Helper macros for automatic to_json / from_json struct binding
#define COREJSON_TO_JSON(v, key) j[#key] = v.key;
#define COREJSON_FROM_JSON(v, key) if (j.contains(#key)) { j.at(#key).get_to(v.key); }

// Preprocessor counting and dispatch
#define COREJSON_ARG_N( \
    _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, \
    _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, \
    _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, \
    _31, _32, N, ...) N

#define COREJSON_RSEQ_N() \
    32, 31, 30, 29, 28, 27, 26, 25, 24, 23, \
    22, 21, 20, 19, 18, 17, 16, 15, 14, 13, \
    12, 11, 10, 9, 8, 7, 6, 5, 4, 3, \
    2, 1, 0

#define COREJSON_NARGS_(...) COREJSON_EXPAND(COREJSON_ARG_N(__VA_ARGS__))
#define COREJSON_NARGS(...) COREJSON_NARGS_(__VA_ARGS__, COREJSON_RSEQ_N())
#define COREJSON_EXPAND(x) x
#define COREJSON_CONCAT(x, y) COREJSON_CONCAT_(x, y)
#define COREJSON_CONCAT_(x, y) x##y

// Per-count macro expansions
#define COREJSON_TO_1(v, a) COREJSON_TO_JSON(v, a)
#define COREJSON_TO_2(v, a, b) COREJSON_TO_1(v, a) COREJSON_TO_JSON(v, b)
#define COREJSON_TO_3(v, a, b, c) COREJSON_TO_2(v, a, b) COREJSON_TO_JSON(v, c)
#define COREJSON_TO_4(v, a, b, c, d) COREJSON_TO_3(v, a, b, c) COREJSON_TO_JSON(v, d)
#define COREJSON_TO_5(v, a, b, c, d, e) COREJSON_TO_4(v, a, b, c, d) COREJSON_TO_JSON(v, e)
#define COREJSON_TO_6(v, a, b, c, d, e, f) COREJSON_TO_5(v, a, b, c, d, e) COREJSON_TO_JSON(v, f)
#define COREJSON_TO_7(v, a, b, c, d, e, f, g) COREJSON_TO_6(v, a, b, c, d, e, f) COREJSON_TO_JSON(v, g)
#define COREJSON_TO_8(v, a, b, c, d, e, f, g, h) COREJSON_TO_7(v, a, b, c, d, e, f, g) COREJSON_TO_JSON(v, h)
#define COREJSON_TO_9(v, a, b, c, d, e, f, g, h, i) COREJSON_TO_8(v, a, b, c, d, e, f, g, h) COREJSON_TO_JSON(v, i)
#define COREJSON_TO_10(v, a, b, c, d, e, f, g, h, i, j) COREJSON_TO_9(v, a, b, c, d, e, f, g, h, i) COREJSON_TO_JSON(v, j)

#define COREJSON_FROM_1(v, a) COREJSON_FROM_JSON(v, a)
#define COREJSON_FROM_2(v, a, b) COREJSON_FROM_1(v, a) COREJSON_FROM_JSON(v, b)
#define COREJSON_FROM_3(v, a, b, c) COREJSON_FROM_2(v, a, b) COREJSON_FROM_JSON(v, c)
#define COREJSON_FROM_4(v, a, b, c, d) COREJSON_FROM_3(v, a, b, c) COREJSON_FROM_JSON(v, d)
#define COREJSON_FROM_5(v, a, b, c, d, e) COREJSON_FROM_4(v, a, b, c, d, e) COREJSON_FROM_JSON(v, e)
#define COREJSON_FROM_6(v, a, b, c, d, e, f) COREJSON_FROM_5(v, a, b, c, d, e, f) COREJSON_FROM_JSON(v, f)
#define COREJSON_FROM_7(v, a, b, c, d, e, f, g) COREJSON_FROM_6(v, a, b, c, d, e, f, g) COREJSON_FROM_JSON(v, g)
#define COREJSON_FROM_8(v, a, b, c, d, e, f, g, h) COREJSON_FROM_7(v, a, b, c, d, e, f, g) COREJSON_FROM_JSON(v, h)
#define COREJSON_FROM_9(v, a, b, c, d, e, f, g, h, i) COREJSON_FROM_8(v, a, b, c, d, e, f, g, h) COREJSON_FROM_JSON(v, i)
#define COREJSON_FROM_10(v, a, b, c, d, e, f, g, h, i, j) COREJSON_FROM_9(v, a, b, c, d, e, f, g, h, i) COREJSON_FROM_JSON(v, j)

/**
 * @brief Macro to define struct/class serialization & deserialization functions.
 * Usage:
 * struct User {
 *     std::string name;
 *     int age;
 * };
 * COREJSON_BIND(User, name, age)
 */
#define COREJSON_BIND(Type, ...) \
    inline void to_json(::corejson::value& j, const Type& v) { \
        j = ::corejson::value::object(); \
        COREJSON_EXPAND(COREJSON_CONCAT(COREJSON_TO_, COREJSON_NARGS(__VA_ARGS__))(v, __VA_ARGS__)) \
    } \
    inline void from_json(const ::corejson::value& j, Type& v) { \
        COREJSON_EXPAND(COREJSON_CONCAT(COREJSON_FROM_, COREJSON_NARGS(__VA_ARGS__))(v, __VA_ARGS__)) \
    }

// Alias for flexibility
#define COREJSON_DEFINE_TYPE(Type, ...) COREJSON_BIND(Type, __VA_ARGS__)

} // namespace corejson


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
