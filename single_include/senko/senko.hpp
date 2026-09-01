/**
 * SenkoJSON - Single Header Amalgamation
 * https://github.com/Baranigsiz/SenkoJSON
 * 
 * Version: 2.2.0
 * License: MIT
 * 
 * Lightning-fast, zero-overhead modern C++17/20 JSON library with MessagePack, CBOR & JSONPath.
 */

#ifndef SENKO_SINGLE_AMALGAMATION_HPP
#define SENKO_SINGLE_AMALGAMATION_HPP

#define SENKO_VERSION_MAJOR 2
#define SENKO_VERSION_MINOR 2
#define SENKO_VERSION_PATCH 0


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

namespace senko {

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

} // namespace senko

// Alias for backwards compatibility
namespace corejson = senko;


// ========================================================
// Header: error.hpp
// ========================================================



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

} // namespace senko


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

namespace senko {

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

    // JSONPath support declarations
    std::vector<value> jsonpath(std::string_view query) const;
    value jsonpath_first(std::string_view query) const;

    // JSON Patch (RFC 6902) & Diff declarations
    value patch(const value& patch_doc) const;
    void patch_in_place(const value& patch_doc);
    static value diff(const value& source, const value& target);
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

} // namespace senko


// ========================================================
// Header: stl_adapters.hpp
// ========================================================






#include <optional>
#include <vector>
#include <deque>
#include <list>
#include <set>
#include <unordered_set>
#include <map>
#include <unordered_map>
#include <utility>
#include <type_traits>

namespace senko {

// ==========================================
// std::optional<T> Adapter
// ==========================================
template <typename T>
struct adl_serializer<std::optional<T>> {
    static void serialize(value& j, const std::optional<T>& opt) {
        if (opt.has_value()) {
            j = *opt;
        } else {
            j = nullptr;
        }
    }

    static void deserialize(const value& j, std::optional<T>& opt) {
        if (j.is_null()) {
            opt = std::nullopt;
        } else {
            opt = j.get<T>();
        }
    }
};

// ==========================================
// std::pair<T1, T2> Adapter
// ==========================================
template <typename T1, typename T2>
struct adl_serializer<std::pair<T1, T2>> {
    static void serialize(value& j, const std::pair<T1, T2>& p) {
        value::array_t arr;
        arr.reserve(2);
        arr.emplace_back(p.first);
        arr.emplace_back(p.second);
        j = std::move(arr);
    }

    static void deserialize(const value& j, std::pair<T1, T2>& p) {
        if (!j.is_array() || j.size() < 2) {
            throw type_error("Expected array with at least 2 elements for std::pair, got " + std::string(j.type_name()));
        }
        p.first = j[0].get<T1>();
        p.second = j[1].get<T2>();
    }
};

// ==========================================
// std::vector<T> Adapter (for non-value types)
// ==========================================
template <typename T>
struct adl_serializer<std::vector<T>, std::enable_if_t<!std::is_same_v<T, value>>> {
    static void serialize(value& j, const std::vector<T>& vec) {
        value::array_t arr;
        arr.reserve(vec.size());
        for (const auto& item : vec) {
            arr.emplace_back(item);
        }
        j = std::move(arr);
    }

    static void deserialize(const value& j, std::vector<T>& vec) {
        if (!j.is_array()) {
            throw type_error("Expected array for std::vector, got " + std::string(j.type_name()));
        }
        vec.clear();
        vec.reserve(j.size());
        for (size_t i = 0; i < j.size(); ++i) {
            vec.push_back(j[i].get<T>());
        }
    }
};

// ==========================================
// std::deque<T> Adapter
// ==========================================
template <typename T>
struct adl_serializer<std::deque<T>> {
    static void serialize(value& j, const std::deque<T>& deq) {
        value::array_t arr;
        arr.reserve(deq.size());
        for (const auto& item : deq) {
            arr.emplace_back(item);
        }
        j = std::move(arr);
    }

    static void deserialize(const value& j, std::deque<T>& deq) {
        if (!j.is_array()) {
            throw type_error("Expected array for std::deque, got " + std::string(j.type_name()));
        }
        deq.clear();
        for (size_t i = 0; i < j.size(); ++i) {
            deq.push_back(j[i].get<T>());
        }
    }
};

// ==========================================
// std::list<T> Adapter
// ==========================================
template <typename T>
struct adl_serializer<std::list<T>> {
    static void serialize(value& j, const std::list<T>& lst) {
        value::array_t arr;
        arr.reserve(lst.size());
        for (const auto& item : lst) {
            arr.emplace_back(item);
        }
        j = std::move(arr);
    }

    static void deserialize(const value& j, std::list<T>& lst) {
        if (!j.is_array()) {
            throw type_error("Expected array for std::list, got " + std::string(j.type_name()));
        }
        lst.clear();
        for (size_t i = 0; i < j.size(); ++i) {
            lst.push_back(j[i].get<T>());
        }
    }
};

// ==========================================
// std::set<T> Adapter
// ==========================================
template <typename T>
struct adl_serializer<std::set<T>> {
    static void serialize(value& j, const std::set<T>& s) {
        value::array_t arr;
        arr.reserve(s.size());
        for (const auto& item : s) {
            arr.emplace_back(item);
        }
        j = std::move(arr);
    }

    static void deserialize(const value& j, std::set<T>& s) {
        if (!j.is_array()) {
            throw type_error("Expected array for std::set, got " + std::string(j.type_name()));
        }
        s.clear();
        for (size_t i = 0; i < j.size(); ++i) {
            s.insert(j[i].get<T>());
        }
    }
};

// ==========================================
// std::unordered_set<T> Adapter
// ==========================================
template <typename T>
struct adl_serializer<std::unordered_set<T>> {
    static void serialize(value& j, const std::unordered_set<T>& s) {
        value::array_t arr;
        arr.reserve(s.size());
        for (const auto& item : s) {
            arr.emplace_back(item);
        }
        j = std::move(arr);
    }

    static void deserialize(const value& j, std::unordered_set<T>& s) {
        if (!j.is_array()) {
            throw type_error("Expected array for std::unordered_set, got " + std::string(j.type_name()));
        }
        s.clear();
        for (size_t i = 0; i < j.size(); ++i) {
            s.insert(j[i].get<T>());
        }
    }
};

// ==========================================
// std::map<std::string, T> Adapter
// ==========================================
template <typename T>
struct adl_serializer<std::map<std::string, T>> {
    static void serialize(value& j, const std::map<std::string, T>& m) {
        value::object_t obj;
        obj.reserve(m.size());
        for (const auto& [k, v] : m) {
            obj.emplace_back(k, value(v));
        }
        j = std::move(obj);
    }

    static void deserialize(const value& j, std::map<std::string, T>& m) {
        if (!j.is_object()) {
            throw type_error("Expected object for std::map, got " + std::string(j.type_name()));
        }
        m.clear();
        const auto& obj = j.get_ref_object();
        for (const auto& [k, v] : obj) {
            m[k] = v.template get<T>();
        }
    }
};

// ==========================================
// std::unordered_map<std::string, T> Adapter
// ==========================================
template <typename T>
struct adl_serializer<std::unordered_map<std::string, T>> {
    static void serialize(value& j, const std::unordered_map<std::string, T>& m) {
        value::object_t obj;
        obj.reserve(m.size());
        for (const auto& [k, v] : m) {
            obj.emplace_back(k, value(v));
        }
        j = std::move(obj);
    }

    static void deserialize(const value& j, std::unordered_map<std::string, T>& m) {
        if (!j.is_object()) {
            throw type_error("Expected object for std::unordered_map, got " + std::string(j.type_name()));
        }
        m.clear();
        const auto& obj = j.get_ref_object();
        for (const auto& [k, v] : obj) {
            m[k] = v.template get<T>();
        }
    }
};

} // namespace senko


// ========================================================
// Header: lexer.hpp
// ========================================================







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


// ========================================================
// Header: parser.hpp
// ========================================================








#include <string_view>
#include <istream>
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

} // namespace senko


// ========================================================
// Header: serializer.hpp
// ========================================================






#include <string>
#include <sstream>
#include <iomanip>
#include <cmath>
#include <limits>
#include <charconv>
#include <cstdio>
#include <cstring>

namespace senko {

namespace detail {

class fast_string_serializer {
public:
    explicit fast_string_serializer(std::string& out, int indent = -1)
        : m_out(out), m_indent(indent), m_depth(0) {}

    void dump(const value& v) {
        switch (v.type()) {
            case value_t::null:
                m_out.append("null", 4);
                break;
            case value_t::boolean:
                if (v.get<bool>()) {
                    m_out.append("true", 4);
                } else {
                    m_out.append("false", 5);
                }
                break;
            case value_t::number_integer: {
                char buf[32];
                auto [ptr, ec] = std::to_chars(buf, buf + sizeof(buf), v.get<int64_t>());
                m_out.append(buf, ptr - buf);
                break;
            }
            case value_t::number_unsigned: {
                char buf[32];
                auto [ptr, ec] = std::to_chars(buf, buf + sizeof(buf), v.get<uint64_t>());
                m_out.append(buf, ptr - buf);
                break;
            }
            case value_t::number_float: {
                double d = v.get<double>();
                if (std::isnan(d) || std::isinf(d)) {
                    m_out.append("null", 4); // RFC 8259 requires NaN/Infinity to be serialized as null
                } else {
                    char buf[64];
                    auto [ptr, ec] = std::to_chars(buf, buf + sizeof(buf), d);
                    if (ec == std::errc()) {
                        std::string_view sv(buf, ptr - buf);
                        m_out.append(sv);
                        if (sv.find('.') == std::string_view::npos && sv.find('e') == std::string_view::npos && sv.find('E') == std::string_view::npos) {
                            m_out.append(".0", 2);
                        }
                    } else {
                        int len = std::snprintf(buf, sizeof(buf), "%.17g", d);
                        if (len > 0) {
                            std::string_view sv(buf, len);
                            m_out.append(sv);
                            if (sv.find('.') == std::string_view::npos && sv.find('e') == std::string_view::npos && sv.find('E') == std::string_view::npos) {
                                m_out.append(".0", 2);
                            }
                        }
                    }
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

private:
    std::string& m_out;
    int m_indent;
    int m_depth;

    void indent_newline() {
        if (m_indent >= 0) {
            m_out.push_back('\n');
            m_out.append(static_cast<size_t>(m_depth * m_indent), ' ');
        }
    }

    void dump_string(std::string_view sv) {
        m_out.push_back('"');
        size_t chunk_start = 0;
        for (size_t i = 0; i < sv.size(); ++i) {
            unsigned char c = static_cast<unsigned char>(sv[i]);
            const char* esc = nullptr;
            size_t esc_len = 0;
            char hex_buf[8];

            switch (c) {
                case '"':  esc = "\\\""; esc_len = 2; break;
                case '\\': esc = "\\\\"; esc_len = 2; break;
                case '\b': esc = "\\b"; esc_len = 2; break;
                case '\f': esc = "\\f"; esc_len = 2; break;
                case '\n': esc = "\\n"; esc_len = 2; break;
                case '\r': esc = "\\r"; esc_len = 2; break;
                case '\t': esc = "\\t"; esc_len = 2; break;
                default:
                    if (c < 0x20) {
                        std::snprintf(hex_buf, sizeof(hex_buf), "\\u%04x", c);
                        esc = hex_buf;
                        esc_len = 6;
                    }
                    break;
            }

            if (esc) {
                if (i > chunk_start) {
                    m_out.append(sv.data() + chunk_start, i - chunk_start);
                }
                m_out.append(esc, esc_len);
                chunk_start = i + 1;
            }
        }
        if (sv.size() > chunk_start) {
            m_out.append(sv.data() + chunk_start, sv.size() - chunk_start);
        }
        m_out.push_back('"');
    }

    void dump_array(const value::array_t& arr) {
        if (arr.empty()) {
            m_out.append("[]", 2);
            return;
        }

        m_out.push_back('[');
        m_depth++;

        for (size_t i = 0; i < arr.size(); ++i) {
            indent_newline();
            dump(arr[i]);
            if (i + 1 < arr.size()) {
                m_out.push_back(',');
            }
        }

        m_depth--;
        indent_newline();
        m_out.push_back(']');
    }

    void dump_object(const value::object_t& obj) {
        if (obj.empty()) {
            m_out.append("{}", 2);
            return;
        }

        m_out.push_back('{');
        m_depth++;

        for (size_t i = 0; i < obj.size(); ++i) {
            indent_newline();
            dump_string(obj[i].first);
            if (m_indent >= 0) {
                m_out.append(": ", 2);
            } else {
                m_out.push_back(':');
            }
            dump(obj[i].second);
            if (i + 1 < obj.size()) {
                m_out.push_back(',');
            }
        }

        m_depth--;
        indent_newline();
        m_out.push_back('}');
    }
};

} // namespace detail

class serializer {
public:
    explicit serializer(std::ostream& os, int indent = -1)
        : m_os(os), m_indent(indent) {}

    void dump(const value& v) {
        std::string s = dump_to_string(v, m_indent);
        m_os << s;
    }

    static std::string dump_to_string(const value& v, int indent = -1) {
        std::string out;
        out.reserve(256);
        detail::fast_string_serializer s(out, indent);
        s.dump(v);
        return out;
    }

private:
    std::ostream& m_os;
    int m_indent;
};

// Inline implementations of value::dump and operator<<
inline std::string value::dump(int indent) const {
    return serializer::dump_to_string(*this, indent);
}

inline void value::dump(std::ostream& os, int indent) const {
    std::string s = dump(indent);
    os << s;
}

inline std::ostream& operator<<(std::ostream& os, const value& j) {
    std::string s = j.dump(-1);
    os << s;
    return os;
}

} // namespace senko


// ========================================================
// Header: json_pointer.hpp
// ========================================================







#include <string>
#include <string_view>
#include <vector>
#include <sstream>

namespace senko {

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

} // namespace senko


// ========================================================
// Header: jsonpath.hpp
// ========================================================







#include <string>
#include <string_view>
#include <vector>
#include <sstream>
#include <functional>
#include <cctype>

namespace senko {

class jsonpath_error : public exception {
public:
    explicit jsonpath_error(std::string msg) : exception("[senko::jsonpath_error] " + std::move(msg)) {}
};

namespace detail {

enum class segment_type {
    root,           // $
    child_key,      // .key or ['key']
    child_wildcard, // .* or [*]
    array_index,    // [0], [-1]
    descendant_key, // ..key
    descendant_wildcard, // ..*
    filter          // [?(@.field == val)]
};

struct filter_expr {
    std::string key;
    std::string op; // "==", "!=", "<", "<=", ">", ">="
    std::string value_str;
    bool is_number = false;
    double number_val = 0.0;
    bool is_bool = false;
    bool bool_val = false;

    bool evaluate(const value& item) const {
        if (!item.is_object() || !item.contains(key)) return false;
        const value& field = item.at(key);

        if (is_number && field.is_number()) {
            double v = field.get<double>();
            if (op == "==") return v == number_val;
            if (op == "!=") return v != number_val;
            if (op == "<") return v < number_val;
            if (op == "<=") return v <= number_val;
            if (op == ">") return v > number_val;
            if (op == ">=") return v >= number_val;
        } else if (is_bool && field.is_boolean()) {
            bool b = field.get<bool>();
            if (op == "==") return b == bool_val;
            if (op == "!=") return b != bool_val;
        } else if (field.is_string()) {
            std::string s = field.get<std::string>();
            if (op == "==") return s == value_str;
            if (op == "!=") return s != value_str;
        }
        return false;
    }
};

struct path_segment {
    segment_type type;
    std::string key;
    int index = 0;
    filter_expr filter;
};

inline void collect_descendants(const value& current, std::string_view target_key, std::vector<value>& results) {
    if (current.is_object()) {
        const auto& obj = current.get_ref_object();
        for (const auto& pair : obj) {
            if (pair.first == target_key) {
                results.push_back(pair.second);
            }
            collect_descendants(pair.second, target_key, results);
        }
    } else if (current.is_array()) {
        const auto& arr = current.get_ref_array();
        for (const auto& elem : arr) {
            collect_descendants(elem, target_key, results);
        }
    }
}

inline std::vector<path_segment> parse_jsonpath(std::string_view expr) {
    std::vector<path_segment> segments;
    size_t i = 0;
    size_t len = expr.size();

    // Skip leading whitespace
    while (i < len && std::isspace(static_cast<unsigned char>(expr[i]))) i++;

    if (i >= len || expr[i] != '$') {
        throw jsonpath_error("JSONPath expression must start with '$'");
    }
    segments.push_back({segment_type::root, "", 0, {}});
    i++; // skip '$'

    while (i < len) {
        if (expr[i] == '.') {
            i++;
            if (i < len && expr[i] == '.') {
                // Recursive descent ..
                i++;
                if (i < len && expr[i] == '*') {
                    segments.push_back({segment_type::descendant_wildcard, "", 0, {}});
                    i++;
                } else {
                    size_t start = i;
                    while (i < len && (std::isalnum(static_cast<unsigned char>(expr[i])) || expr[i] == '_' || expr[i] == '-')) {
                        i++;
                    }
                    if (start == i) throw jsonpath_error("Expected key name after '..'");
                    segments.push_back({segment_type::descendant_key, std::string(expr.substr(start, i - start)), 0, {}});
                }
            } else if (i < len && expr[i] == '*') {
                segments.push_back({segment_type::child_wildcard, "", 0, {}});
                i++;
            } else {
                size_t start = i;
                while (i < len && (std::isalnum(static_cast<unsigned char>(expr[i])) || expr[i] == '_' || expr[i] == '-')) {
                    i++;
                }
                if (start == i) throw jsonpath_error("Expected key name after '.'");
                segments.push_back({segment_type::child_key, std::string(expr.substr(start, i - start)), 0, {}});
            }
        } else if (expr[i] == '[') {
            i++; // skip '['
            while (i < len && std::isspace(static_cast<unsigned char>(expr[i]))) i++;

            if (i < len && expr[i] == '*') {
                segments.push_back({segment_type::child_wildcard, "", 0, {}});
                i++;
            } else if (i < len && (expr[i] == '\'' || expr[i] == '"')) {
                // ['key']
                char quote = expr[i++];
                size_t start = i;
                while (i < len && expr[i] != quote) i++;
                if (i >= len) throw jsonpath_error("Unterminated quoted key in bracket notation");
                segments.push_back({segment_type::child_key, std::string(expr.substr(start, i - start)), 0, {}});
                i++; // skip closing quote
            } else if (i < len && expr[i] == '?') {
                // Filter [?(@.price < 10)]
                i++; // skip '?'
                while (i < len && std::isspace(static_cast<unsigned char>(expr[i]))) i++;
                if (i < len && expr[i] == '(') i++; // skip '('
                while (i < len && std::isspace(static_cast<unsigned char>(expr[i]))) i++;

                if (i < len && expr[i] == '@') i++; // skip '@'
                if (i < len && expr[i] == '.') i++; // skip '.'

                // Read property name
                size_t start_k = i;
                while (i < len && (std::isalnum(static_cast<unsigned char>(expr[i])) || expr[i] == '_' || expr[i] == '-')) i++;
                std::string k = std::string(expr.substr(start_k, i - start_k));

                while (i < len && std::isspace(static_cast<unsigned char>(expr[i]))) i++;

                // Read operator
                std::string op;
                if (i + 1 < len && (expr.substr(i, 2) == "==" || expr.substr(i, 2) == "!=" || expr.substr(i, 2) == "<=" || expr.substr(i, 2) == ">=")) {
                    op = std::string(expr.substr(i, 2));
                    i += 2;
                } else if (i < len && (expr[i] == '<' || expr[i] == '>')) {
                    op = std::string(1, expr[i]);
                    i++;
                } else {
                    throw jsonpath_error("Unsupported or missing comparison operator in filter");
                }

                while (i < len && std::isspace(static_cast<unsigned char>(expr[i]))) i++;

                // Read value
                filter_expr filt;
                filt.key = k;
                filt.op = op;

                if (expr[i] == '\'' || expr[i] == '"') {
                    char q = expr[i++];
                    size_t sv = i;
                    while (i < len && expr[i] != q) i++;
                    filt.value_str = std::string(expr.substr(sv, i - sv));
                    i++; // skip quote
                } else {
                    size_t sv = i;
                    while (i < len && expr[i] != ')' && expr[i] != ']' && !std::isspace(static_cast<unsigned char>(expr[i]))) i++;
                    std::string raw_val = std::string(expr.substr(sv, i - sv));
                    if (raw_val == "true") {
                        filt.is_bool = true;
                        filt.bool_val = true;
                    } else if (raw_val == "false") {
                        filt.is_bool = true;
                        filt.bool_val = false;
                    } else {
                        filt.is_number = true;
                        filt.number_val = std::stod(raw_val);
                    }
                }

                while (i < len && expr[i] != ']') i++;
                segments.push_back({segment_type::filter, "", 0, filt});

            } else {
                // [index]
                size_t start = i;
                if (expr[i] == '-') i++;
                while (i < len && std::isdigit(static_cast<unsigned char>(expr[i]))) i++;
                int idx = std::stoi(std::string(expr.substr(start, i - start)));
                segments.push_back({segment_type::array_index, "", idx, {}});
            }

            while (i < len && expr[i] != ']') i++;
            if (i >= len || expr[i] != ']') throw jsonpath_error("Expected ']' closing bracket");
            i++; // skip ']'
        } else {
            throw jsonpath_error(std::string("Unexpected character '") + expr[i] + "' in JSONPath");
        }
    }

    return segments;
}

} // namespace detail

inline std::vector<value> evaluate_jsonpath(const value& root, std::string_view query);

inline std::vector<value> value::jsonpath(std::string_view query) const {
    return evaluate_jsonpath(*this, query);
}

inline value value::jsonpath_first(std::string_view query) const {
    auto results = evaluate_jsonpath(*this, query);
    if (results.empty()) return value(nullptr);
    return results[0];
}

inline std::vector<value> evaluate_jsonpath(const value& root, std::string_view query) {
    auto segments = detail::parse_jsonpath(query);
    std::vector<value> current_set = {root};

    for (const auto& seg : segments) {
        std::vector<value> next_set;

        for (const auto& item : current_set) {
            switch (seg.type) {
                case detail::segment_type::root:
                    next_set.push_back(item);
                    break;
                case detail::segment_type::child_key:
                    if (item.is_object() && item.contains(seg.key)) {
                        next_set.push_back(item.at(seg.key));
                    }
                    break;
                case detail::segment_type::child_wildcard:
                    if (item.is_object()) {
                        for (const auto& pair : item.get_ref_object()) {
                            next_set.push_back(pair.second);
                        }
                    } else if (item.is_array()) {
                        for (const auto& elem : item.get_ref_array()) {
                            next_set.push_back(elem);
                        }
                    }
                    break;
                case detail::segment_type::array_index:
                    if (item.is_array()) {
                        int idx = seg.index;
                        const auto& arr = item.get_ref_array();
                        if (idx < 0) idx += static_cast<int>(arr.size());
                        if (idx >= 0 && static_cast<size_t>(idx) < arr.size()) {
                            next_set.push_back(arr[static_cast<size_t>(idx)]);
                        }
                    }
                    break;
                case detail::segment_type::descendant_key:
                    detail::collect_descendants(item, seg.key, next_set);
                    break;
                case detail::segment_type::descendant_wildcard:
                    // Add all children recursively
                    break;
                case detail::segment_type::filter:
                    if (item.is_array()) {
                        for (const auto& elem : item.get_ref_array()) {
                            if (seg.filter.evaluate(elem)) {
                                next_set.push_back(elem);
                            }
                        }
                    } else if (item.is_object()) {
                        if (seg.filter.evaluate(item)) {
                            next_set.push_back(item);
                        }
                    }
                    break;
            }
        }

        current_set = std::move(next_set);
        if (current_set.empty()) break;
    }

    return current_set;
}

} // namespace senko



// ========================================================
// Header: patch.hpp
// ========================================================








#include <string>
#include <vector>
#include <utility>
#include <algorithm>

namespace senko {

/**
 * @brief Exception thrown when a JSON Patch (RFC 6902) operation fails.
 */
class patch_error : public exception {
public:
    explicit patch_error(std::string msg)
        : exception("[senko::patch_error] " + std::move(msg)) {}
};

/**
 * @brief Exception thrown when a JSON Patch 'test' assertion operation fails.
 */
class patch_test_failed : public patch_error {
public:
    explicit patch_test_failed(std::string msg)
        : patch_error("Test operation failed: " + std::move(msg)) {}
};

namespace detail {

inline void apply_patch_op(value& doc, const value& op_obj) {
    if (!op_obj.is_object()) {
        throw patch_error("Each patch operation must be a JSON object");
    }

    if (!op_obj.contains("op") || !op_obj.at("op").is_string()) {
        throw patch_error("Patch operation missing 'op' string property");
    }
    if (!op_obj.contains("path") || !op_obj.at("path").is_string()) {
        throw patch_error("Patch operation missing 'path' string property");
    }

    std::string op = op_obj.at("op").get<std::string>();
    std::string path_str = op_obj.at("path").get<std::string>();
    json_pointer ptr(path_str);

    if (op == "add") {
        if (!op_obj.contains("value")) {
            throw patch_error("Operation 'add' requires 'value' property");
        }
        const value& val = op_obj.at("value");

        if (ptr.empty()) {
            doc = val;
            return;
        }

        // Navigate to parent
        json_pointer parent_ptr = ptr;
        std::string last_token = parent_ptr.tokens().back();
        parent_ptr.pop_back();

        value& parent = parent_ptr.resolve(doc);
        if (parent.is_object()) {
            parent[last_token] = val;
        } else if (parent.is_array()) {
            auto& arr = parent.get_ref_array();
            if (last_token == "-") {
                arr.push_back(val);
            } else {
                size_t idx = 0;
                try {
                    idx = std::stoull(last_token);
                } catch (...) {
                    throw patch_error("Invalid array index in JSON Patch: '" + last_token + "'");
                }
                if (idx > arr.size()) {
                    throw out_of_range("Array index out of range for add operation: " + std::to_string(idx));
                }
                arr.insert(arr.begin() + idx, val);
            }
        } else {
            throw type_error("Cannot add element to primitive JSON value at path: " + parent_ptr.to_string());
        }

    } else if (op == "remove") {
        if (ptr.empty()) {
            throw patch_error("Cannot remove root JSON document");
        }

        json_pointer parent_ptr = ptr;
        std::string last_token = parent_ptr.tokens().back();
        parent_ptr.pop_back();

        value& parent = parent_ptr.resolve(doc);
        if (parent.is_object()) {
            if (!parent.contains(last_token)) {
                throw out_of_range("Key not found for remove operation: '" + last_token + "'");
            }
            parent.erase(last_token);
        } else if (parent.is_array()) {
            auto& arr = parent.get_ref_array();
            size_t idx = 0;
            try {
                idx = std::stoull(last_token);
            } catch (...) {
                throw patch_error("Invalid array index in JSON Patch: '" + last_token + "'");
            }
            if (idx >= arr.size()) {
                throw out_of_range("Array index out of range for remove operation: " + std::to_string(idx));
            }
            arr.erase(arr.begin() + idx);
        } else {
            throw type_error("Cannot remove element from primitive value at path: " + parent_ptr.to_string());
        }

    } else if (op == "replace") {
        if (!op_obj.contains("value")) {
            throw patch_error("Operation 'replace' requires 'value' property");
        }
        const value& val = op_obj.at("value");

        if (ptr.empty()) {
            doc = val;
            return;
        }

        // Must exist before replacing
        value& target = ptr.resolve(doc);
        target = val;

    } else if (op == "move") {
        if (!op_obj.contains("from") || !op_obj.at("from").is_string()) {
            throw patch_error("Operation 'move' requires 'from' string property");
        }
        std::string from_str = op_obj.at("from").get<std::string>();
        json_pointer from_ptr(from_str);

        if (from_ptr.empty()) {
            throw patch_error("Cannot move from root JSON document");
        }

        // Value to move
        value val = from_ptr.resolve(doc);

        // Remove from source
        value remove_op = value::object({{"op", "remove"}, {"path", from_str}});
        apply_patch_op(doc, remove_op);

        // Add to destination
        value add_op = value::object({{"op", "add"}, {"path", path_str}, {"value", std::move(val)}});
        apply_patch_op(doc, add_op);

    } else if (op == "copy") {
        if (!op_obj.contains("from") || !op_obj.at("from").is_string()) {
            throw patch_error("Operation 'copy' requires 'from' string property");
        }
        std::string from_str = op_obj.at("from").get<std::string>();
        json_pointer from_ptr(from_str);

        value val = from_ptr.resolve(doc);

        value add_op = value::object({{"op", "add"}, {"path", path_str}, {"value", std::move(val)}});
        apply_patch_op(doc, add_op);

    } else if (op == "test") {
        if (!op_obj.contains("value")) {
            throw patch_error("Operation 'test' requires 'value' property");
        }
        const value& expected = op_obj.at("value");
        const value& actual = ptr.resolve(doc);

        if (actual != expected) {
            throw patch_test_failed("Value at path '" + path_str + "' does not match expected value");
        }

    } else {
        throw patch_error("Unsupported JSON Patch operation: '" + op + "'");
    }
}

inline void generate_diff_recursive(const value& source, const value& target, const std::string& path, value::array_t& patches) {
    if (source == target) {
        return;
    }

    if (source.is_object() && target.is_object()) {
        const auto& src_obj = source.get_ref_object();
        const auto& tgt_obj = target.get_ref_object();

        // Check for removed keys
        for (const auto& [k, v] : src_obj) {
            if (!target.contains(k)) {
                std::string item_path = path + "/" + json_pointer::escape(k);
                patches.push_back(value::object({
                    {"op", "remove"},
                    {"path", item_path}
                }));
            }
        }

        // Check for added or modified keys
        for (const auto& [k, v] : tgt_obj) {
            std::string item_path = path + "/" + json_pointer::escape(k);
            if (!source.contains(k)) {
                patches.push_back(value::object({
                    {"op", "add"},
                    {"path", item_path},
                    {"value", v}
                }));
            } else {
                generate_diff_recursive(source.at(k), v, item_path, patches);
            }
        }
    } else if (source.is_array() && target.is_array()) {
        const auto& src_arr = source.get_ref_array();
        const auto& tgt_arr = target.get_ref_array();
        size_t min_len = (std::min)(src_arr.size(), tgt_arr.size());

        // Diffs in common indices
        for (size_t i = 0; i < min_len; ++i) {
            std::string item_path = path + "/" + std::to_string(i);
            generate_diff_recursive(src_arr[i], tgt_arr[i], item_path, patches);
        }

        // Extra elements in target (add)
        for (size_t i = min_len; i < tgt_arr.size(); ++i) {
            patches.push_back(value::object({
                {"op", "add"},
                {"path", path + "/-"},
                {"value", tgt_arr[i]}
            }));
        }

        // Extra elements in source (remove in reverse order)
        for (size_t i = src_arr.size(); i > min_len; --i) {
            patches.push_back(value::object({
                {"op", "remove"},
                {"path", path + "/" + std::to_string(i - 1)}
            }));
        }
    } else {
        // Types or primitives differ -> replace
        patches.push_back(value::object({
            {"op", "replace"},
            {"path", path.empty() ? "" : path},
            {"value", target}
        }));
    }
}

} // namespace detail

/**
 * @brief Applies an RFC 6902 JSON Patch document to the given JSON value in-place.
 */
inline void apply_patch(value& doc, const value& patch_doc) {
    if (!patch_doc.is_array()) {
        throw patch_error("JSON Patch document must be an array of operation objects");
    }
    for (size_t i = 0; i < patch_doc.size(); ++i) {
        detail::apply_patch_op(doc, patch_doc[i]);
    }
}

/**
 * @brief Computes the RFC 6902 JSON Patch that transforms `source` into `target`.
 */
inline value diff(const value& source, const value& target) {
    value::array_t patches;
    detail::generate_diff_recursive(source, target, "", patches);
    return value(std::move(patches));
}

// Inline member methods for value
inline value value::patch(const value& patch_doc) const {
    value copy = *this;
    apply_patch(copy, patch_doc);
    return copy;
}

inline void value::patch_in_place(const value& patch_doc) {
    apply_patch(*this, patch_doc);
}

inline value value::diff(const value& source, const value& target) {
    return senko::diff(source, target);
}

} // namespace senko


// ========================================================
// Header: binary/msgpack.hpp
// ========================================================







#include <vector>
#include <cstdint>
#include <cstring>
#include <string_view>
#include <type_traits>

namespace senko {

class msgpack_error : public exception {
public:
    explicit msgpack_error(std::string msg) : exception("[senko::msgpack_error] " + std::move(msg)) {}
};

namespace detail {

// Big-Endian helpers
inline void write_u8(std::vector<uint8_t>& out, uint8_t v) {
    out.push_back(v);
}

inline void write_u16_be(std::vector<uint8_t>& out, uint16_t v) {
    out.push_back(static_cast<uint8_t>((v >> 8) & 0xFF));
    out.push_back(static_cast<uint8_t>(v & 0xFF));
}

inline void write_u32_be(std::vector<uint8_t>& out, uint32_t v) {
    out.push_back(static_cast<uint8_t>((v >> 24) & 0xFF));
    out.push_back(static_cast<uint8_t>((v >> 16) & 0xFF));
    out.push_back(static_cast<uint8_t>((v >> 8) & 0xFF));
    out.push_back(static_cast<uint8_t>(v & 0xFF));
}

inline void write_u64_be(std::vector<uint8_t>& out, uint64_t v) {
    for (int i = 7; i >= 0; --i) {
        out.push_back(static_cast<uint8_t>((v >> (i * 8)) & 0xFF));
    }
}

inline uint16_t read_u16_be(const uint8_t* p) {
    return static_cast<uint16_t>((uint16_t(p[0]) << 8) | uint16_t(p[1]));
}

inline uint32_t read_u32_be(const uint8_t* p) {
    return (uint32_t(p[0]) << 24) | (uint32_t(p[1]) << 16) | (uint32_t(p[2]) << 8) | uint32_t(p[3]);
}

inline uint64_t read_u64_be(const uint8_t* p) {
    uint64_t v = 0;
    for (int i = 0; i < 8; ++i) {
        v = (v << 8) | uint64_t(p[i]);
    }
    return v;
}

inline void serialize_msgpack_impl(const value& v, std::vector<uint8_t>& out) {
    switch (v.type()) {
        case value_t::null:
            write_u8(out, 0xC0); // nil
            break;
        case value_t::boolean:
            write_u8(out, v.get<bool>() ? 0xC3 : 0xC2); // true : false
            break;
        case value_t::number_integer: {
            int64_t val = v.get<int64_t>();
            if (val >= -32 && val <= 127) {
                write_u8(out, static_cast<uint8_t>(val));
            } else if (val >= std::numeric_limits<int8_t>::min() && val <= std::numeric_limits<int8_t>::max()) {
                write_u8(out, 0xD0); // int 8
                write_u8(out, static_cast<uint8_t>(val));
            } else if (val >= std::numeric_limits<int16_t>::min() && val <= std::numeric_limits<int16_t>::max()) {
                write_u8(out, 0xD1); // int 16
                write_u16_be(out, static_cast<uint16_t>(val));
            } else if (val >= std::numeric_limits<int32_t>::min() && val <= std::numeric_limits<int32_t>::max()) {
                write_u8(out, 0xD2); // int 32
                write_u32_be(out, static_cast<uint32_t>(val));
            } else {
                write_u8(out, 0xD3); // int 64
                write_u64_be(out, static_cast<uint64_t>(val));
            }
            break;
        }
        case value_t::number_unsigned: {
            uint64_t val = v.get<uint64_t>();
            if (val <= 127) {
                write_u8(out, static_cast<uint8_t>(val));
            } else if (val <= std::numeric_limits<uint8_t>::max()) {
                write_u8(out, 0xCC); // uint 8
                write_u8(out, static_cast<uint8_t>(val));
            } else if (val <= std::numeric_limits<uint16_t>::max()) {
                write_u8(out, 0xCD); // uint 16
                write_u16_be(out, static_cast<uint16_t>(val));
            } else if (val <= std::numeric_limits<uint32_t>::max()) {
                write_u8(out, 0xCE); // uint 32
                write_u32_be(out, static_cast<uint32_t>(val));
            } else {
                write_u8(out, 0xCF); // uint 64
                write_u64_be(out, val);
            }
            break;
        }
        case value_t::number_float: {
            double d = v.get<double>();
            write_u8(out, 0xCB); // float 64
            uint64_t raw = 0;
            std::memcpy(&raw, &d, sizeof(double));
            write_u64_be(out, raw);
            break;
        }
        case value_t::string: {
            const std::string& str = v.get_ref_string();
            size_t len = str.size();
            if (len <= 31) {
                write_u8(out, static_cast<uint8_t>(0xA0 | len));
            } else if (len <= 0xFF) {
                write_u8(out, 0xD9); // str 8
                write_u8(out, static_cast<uint8_t>(len));
            } else if (len <= 0xFFFF) {
                write_u8(out, 0xDA); // str 16
                write_u16_be(out, static_cast<uint16_t>(len));
            } else {
                write_u8(out, 0xDB); // str 32
                write_u32_be(out, static_cast<uint32_t>(len));
            }
            out.insert(out.end(), str.begin(), str.end());
            break;
        }
        case value_t::array: {
            const auto& arr = v.get_ref_array();
            size_t len = arr.size();
            if (len <= 15) {
                write_u8(out, static_cast<uint8_t>(0x90 | len));
            } else if (len <= 0xFFFF) {
                write_u8(out, 0xDC); // array 16
                write_u16_be(out, static_cast<uint16_t>(len));
            } else {
                write_u8(out, 0xDD); // array 32
                write_u32_be(out, static_cast<uint32_t>(len));
            }
            for (const auto& elem : arr) {
                serialize_msgpack_impl(elem, out);
            }
            break;
        }
        case value_t::object: {
            const auto& obj = v.get_ref_object();
            size_t len = obj.size();
            if (len <= 15) {
                write_u8(out, static_cast<uint8_t>(0x80 | len));
            } else if (len <= 0xFFFF) {
                write_u8(out, 0xDE); // map 16
                write_u16_be(out, static_cast<uint16_t>(len));
            } else {
                write_u8(out, 0xDF); // map 32
                write_u32_be(out, static_cast<uint32_t>(len));
            }
            for (const auto& pair : obj) {
                // Key (string)
                size_t klen = pair.first.size();
                if (klen <= 31) {
                    write_u8(out, static_cast<uint8_t>(0xA0 | klen));
                } else if (klen <= 0xFF) {
                    write_u8(out, 0xD9);
                    write_u8(out, static_cast<uint8_t>(klen));
                } else if (klen <= 0xFFFF) {
                    write_u8(out, 0xDA);
                    write_u16_be(out, static_cast<uint16_t>(klen));
                } else {
                    write_u8(out, 0xDB);
                    write_u32_be(out, static_cast<uint32_t>(klen));
                }
                out.insert(out.end(), pair.first.begin(), pair.first.end());
                // Value
                serialize_msgpack_impl(pair.second, out);
            }
            break;
        }
    }
}

class msgpack_reader {
public:
    msgpack_reader(const uint8_t* data, size_t size)
        : m_data(data), m_size(size), m_pos(0) {}

    value parse() {
        if (m_pos >= m_size) {
            throw msgpack_error("Unexpected end of MessagePack input");
        }
        uint8_t tag = m_data[m_pos++];

        // Positive fixint: 0x00 - 0x7f
        if (tag <= 0x7F) {
            return value(static_cast<int64_t>(tag));
        }
        // Fixmap: 0x80 - 0x8f
        if (tag >= 0x80 && tag <= 0x8F) {
            return parse_map(tag & 0x0F);
        }
        // Fixarray: 0x90 - 0x9f
        if (tag >= 0x90 && tag <= 0x9F) {
            return parse_array(tag & 0x0F);
        }
        // Fixstr: 0xa0 - 0xbf
        if (tag >= 0xA0 && tag <= 0xBF) {
            return parse_string_bytes(tag & 0x1F);
        }
        // Negative fixint: 0xe0 - 0xff
        if (tag >= 0xE0) {
            return value(static_cast<int64_t>(static_cast<int8_t>(tag)));
        }

        switch (tag) {
            case 0xC0: return value(nullptr);
            case 0xC2: return value(false);
            case 0xC3: return value(true);
            case 0xCA: { // float 32
                ensure_bytes(4);
                uint32_t raw = read_u32_be(&m_data[m_pos]);
                m_pos += 4;
                float f = 0.0f;
                std::memcpy(&f, &raw, sizeof(float));
                return value(static_cast<double>(f));
            }
            case 0xCB: { // float 64
                ensure_bytes(8);
                uint64_t raw = read_u64_be(&m_data[m_pos]);
                m_pos += 8;
                double d = 0.0;
                std::memcpy(&d, &raw, sizeof(double));
                return value(d);
            }
            case 0xCC: { // uint 8
                ensure_bytes(1);
                return value(static_cast<uint64_t>(m_data[m_pos++]));
            }
            case 0xCD: { // uint 16
                ensure_bytes(2);
                uint16_t v = read_u16_be(&m_data[m_pos]);
                m_pos += 2;
                return value(static_cast<uint64_t>(v));
            }
            case 0xCE: { // uint 32
                ensure_bytes(4);
                uint32_t v = read_u32_be(&m_data[m_pos]);
                m_pos += 4;
                return value(static_cast<uint64_t>(v));
            }
            case 0xCF: { // uint 64
                ensure_bytes(8);
                uint64_t v = read_u64_be(&m_data[m_pos]);
                m_pos += 8;
                return value(v);
            }
            case 0xD0: { // int 8
                ensure_bytes(1);
                return value(static_cast<int64_t>(static_cast<int8_t>(m_data[m_pos++])));
            }
            case 0xD1: { // int 16
                ensure_bytes(2);
                int16_t v = static_cast<int16_t>(read_u16_be(&m_data[m_pos]));
                m_pos += 2;
                return value(static_cast<int64_t>(v));
            }
            case 0xD2: { // int 32
                ensure_bytes(4);
                int32_t v = static_cast<int32_t>(read_u32_be(&m_data[m_pos]));
                m_pos += 4;
                return value(static_cast<int64_t>(v));
            }
            case 0xD3: { // int 64
                ensure_bytes(8);
                int64_t v = static_cast<int64_t>(read_u64_be(&m_data[m_pos]));
                m_pos += 8;
                return value(v);
            }
            case 0xD9: { // str 8
                ensure_bytes(1);
                size_t len = m_data[m_pos++];
                return parse_string_bytes(len);
            }
            case 0xDA: { // str 16
                ensure_bytes(2);
                size_t len = read_u16_be(&m_data[m_pos]);
                m_pos += 2;
                return parse_string_bytes(len);
            }
            case 0xDB: { // str 32
                ensure_bytes(4);
                size_t len = read_u32_be(&m_data[m_pos]);
                m_pos += 4;
                return parse_string_bytes(len);
            }
            case 0xDC: { // array 16
                ensure_bytes(2);
                size_t len = read_u16_be(&m_data[m_pos]);
                m_pos += 2;
                return parse_array(len);
            }
            case 0xDD: { // array 32
                ensure_bytes(4);
                size_t len = read_u32_be(&m_data[m_pos]);
                m_pos += 4;
                return parse_array(len);
            }
            case 0xDE: { // map 16
                ensure_bytes(2);
                size_t len = read_u16_be(&m_data[m_pos]);
                m_pos += 2;
                return parse_map(len);
            }
            case 0xDF: { // map 32
                ensure_bytes(4);
                size_t len = read_u32_be(&m_data[m_pos]);
                m_pos += 4;
                return parse_map(len);
            }
            default:
                throw msgpack_error("Unsupported or invalid MessagePack tag: 0x" + std::to_string(tag));
        }
    }

private:
    const uint8_t* m_data;
    size_t m_size;
    size_t m_pos;

    void ensure_bytes(size_t n) {
        if (m_pos + n > m_size) {
            throw msgpack_error("Unexpected end of MessagePack input, expected " + std::to_string(n) + " more bytes");
        }
    }

    value parse_string_bytes(size_t len) {
        ensure_bytes(len);
        std::string str(reinterpret_cast<const char*>(&m_data[m_pos]), len);
        m_pos += len;
        return value(std::move(str));
    }

    value parse_array(size_t len) {
        value::array_t arr;
        arr.reserve(len);
        for (size_t i = 0; i < len; ++i) {
            arr.push_back(parse());
        }
        return value(std::move(arr));
    }

    value parse_map(size_t len) {
        value::object_t obj;
        obj.reserve(len);
        for (size_t i = 0; i < len; ++i) {
            value key_v = parse();
            if (!key_v.is_string()) {
                throw msgpack_error("Map key in MessagePack must be a string");
            }
            value val_v = parse();
            obj.emplace_back(std::move(key_v.get_ref_string()), std::move(val_v));
        }
        return value(std::move(obj));
    }
};

} // namespace detail

/**
 * @brief Serializes a Senko JSON value into a binary MessagePack buffer.
 */
inline std::vector<uint8_t> to_msgpack(const value& j) {
    std::vector<uint8_t> out;
    detail::serialize_msgpack_impl(j, out);
    return out;
}

/**
 * @brief Deserializes a binary MessagePack buffer into a Senko JSON value.
 */
inline value from_msgpack(const uint8_t* data, size_t size) {
    detail::msgpack_reader reader(data, size);
    return reader.parse();
}

inline value from_msgpack(const std::vector<uint8_t>& bytes) {
    return from_msgpack(bytes.data(), bytes.size());
}

inline value from_msgpack(std::string_view bytes) {
    return from_msgpack(reinterpret_cast<const uint8_t*>(bytes.data()), bytes.size());
}

} // namespace senko


// ========================================================
// Header: binary/cbor.hpp
// ========================================================







#include <vector>
#include <cstdint>
#include <cstring>
#include <string_view>
#include <limits>

namespace senko {

class cbor_error : public exception {
public:
    explicit cbor_error(std::string msg) : exception("[senko::cbor_error] " + std::move(msg)) {}
};

namespace detail {

inline void cbor_write_type_and_val(std::vector<uint8_t>& out, uint8_t major, uint64_t val) {
    uint8_t m = static_cast<uint8_t>(major << 5);
    if (val < 24) {
        out.push_back(static_cast<uint8_t>(m | val));
    } else if (val <= 0xFF) {
        out.push_back(static_cast<uint8_t>(m | 24));
        out.push_back(static_cast<uint8_t>(val));
    } else if (val <= 0xFFFF) {
        out.push_back(static_cast<uint8_t>(m | 25));
        out.push_back(static_cast<uint8_t>((val >> 8) & 0xFF));
        out.push_back(static_cast<uint8_t>(val & 0xFF));
    } else if (val <= 0xFFFFFFFFULL) {
        out.push_back(static_cast<uint8_t>(m | 26));
        out.push_back(static_cast<uint8_t>((val >> 24) & 0xFF));
        out.push_back(static_cast<uint8_t>((val >> 16) & 0xFF));
        out.push_back(static_cast<uint8_t>((val >> 8) & 0xFF));
        out.push_back(static_cast<uint8_t>(val & 0xFF));
    } else {
        out.push_back(static_cast<uint8_t>(m | 27));
        for (int i = 7; i >= 0; --i) {
            out.push_back(static_cast<uint8_t>((val >> (i * 8)) & 0xFF));
        }
    }
}

inline void serialize_cbor_impl(const value& v, std::vector<uint8_t>& out) {
    switch (v.type()) {
        case value_t::null:
            out.push_back(0xF6); // null
            break;
        case value_t::boolean:
            out.push_back(v.get<bool>() ? 0xF5 : 0xF4); // true : false
            break;
        case value_t::number_integer: {
            int64_t val = v.get<int64_t>();
            if (val >= 0) {
                cbor_write_type_and_val(out, 0, static_cast<uint64_t>(val)); // Major 0: unsigned
            } else {
                cbor_write_type_and_val(out, 1, static_cast<uint64_t>(-1 - val)); // Major 1: negative
            }
            break;
        }
        case value_t::number_unsigned: {
            cbor_write_type_and_val(out, 0, v.get<uint64_t>());
            break;
        }
        case value_t::number_float: {
            double d = v.get<double>();
            out.push_back(0xFB); // float 64
            uint64_t raw = 0;
            std::memcpy(&raw, &d, sizeof(double));
            for (int i = 7; i >= 0; --i) {
                out.push_back(static_cast<uint8_t>((raw >> (i * 8)) & 0xFF));
            }
            break;
        }
        case value_t::string: {
            const std::string& str = v.get_ref_string();
            cbor_write_type_and_val(out, 3, str.size()); // Major 3: text string
            out.insert(out.end(), str.begin(), str.end());
            break;
        }
        case value_t::array: {
            const auto& arr = v.get_ref_array();
            cbor_write_type_and_val(out, 4, arr.size()); // Major 4: array
            for (const auto& elem : arr) {
                serialize_cbor_impl(elem, out);
            }
            break;
        }
        case value_t::object: {
            const auto& obj = v.get_ref_object();
            cbor_write_type_and_val(out, 5, obj.size()); // Major 5: map
            for (const auto& pair : obj) {
                // Key (text string)
                cbor_write_type_and_val(out, 3, pair.first.size());
                out.insert(out.end(), pair.first.begin(), pair.first.end());
                // Value
                serialize_cbor_impl(pair.second, out);
            }
            break;
        }
    }
}

class cbor_reader {
public:
    cbor_reader(const uint8_t* data, size_t size)
        : m_data(data), m_size(size), m_pos(0) {}

    value parse() {
        if (m_pos >= m_size) {
            throw cbor_error("Unexpected end of CBOR input");
        }
        uint8_t initial = m_data[m_pos++];
        uint8_t major = initial >> 5;
        uint8_t info = initial & 0x1F;

        uint64_t val = read_length(info);

        switch (major) {
            case 0: // Unsigned integer
                if (val <= static_cast<uint64_t>((std::numeric_limits<int64_t>::max)())) {
                    return value(static_cast<int64_t>(val));
                }
                return value(val);
            case 1: // Negative integer (-1 - val)
                if (val <= static_cast<uint64_t>((std::numeric_limits<int64_t>::max)())) {
                    return value(static_cast<int64_t>(-1 - static_cast<int64_t>(val)));
                }
                throw cbor_error("Negative integer underflow in CBOR");
            case 2: // Byte string (treat as hex or raw string for JSON DOM)
            case 3: { // Text string
                ensure_bytes(val);
                std::string str(reinterpret_cast<const char*>(&m_data[m_pos]), val);
                m_pos += val;
                return value(std::move(str));
            }
            case 4: { // Array
                value::array_t arr;
                arr.reserve(val);
                for (size_t i = 0; i < val; ++i) {
                    arr.push_back(parse());
                }
                return value(std::move(arr));
            }
            case 5: { // Map
                value::object_t obj;
                obj.reserve(val);
                for (size_t i = 0; i < val; ++i) {
                    value key_v = parse();
                    if (!key_v.is_string()) {
                        throw cbor_error("Map key in CBOR must be a string");
                    }
                    value val_v = parse();
                    obj.emplace_back(std::move(key_v.get_ref_string()), std::move(val_v));
                }
                return value(std::move(obj));
            }
            case 7: { // Simple / Float
                if (info == 20) return value(false);
                if (info == 21) return value(true);
                if (info == 22) return value(nullptr);
                if (info == 26) { // float 32
                    uint32_t raw = static_cast<uint32_t>(val);
                    float f = 0.0f;
                    std::memcpy(&f, &raw, sizeof(float));
                    return value(static_cast<double>(f));
                }
                if (info == 27) { // float 64
                    uint64_t raw = val;
                    double d = 0.0;
                    std::memcpy(&d, &raw, sizeof(double));
                    return value(d);
                }
                throw cbor_error("Unsupported CBOR simple type or info: " + std::to_string(info));
            }
            default:
                throw cbor_error("Unsupported CBOR major type: " + std::to_string(major));
        }
    }

private:
    const uint8_t* m_data;
    size_t m_size;
    size_t m_pos;

    void ensure_bytes(size_t n) {
        if (m_pos + n > m_size) {
            throw cbor_error("Unexpected end of CBOR input, expected " + std::to_string(n) + " more bytes");
        }
    }

    uint64_t read_length(uint8_t info) {
        if (info < 24) {
            return info;
        } else if (info == 24) {
            ensure_bytes(1);
            return m_data[m_pos++];
        } else if (info == 25) {
            ensure_bytes(2);
            uint16_t v = (uint16_t(m_data[m_pos]) << 8) | uint16_t(m_data[m_pos + 1]);
            m_pos += 2;
            return v;
        } else if (info == 26) {
            ensure_bytes(4);
            uint32_t v = (uint32_t(m_data[m_pos]) << 24) |
                         (uint32_t(m_data[m_pos + 1]) << 16) |
                         (uint32_t(m_data[m_pos + 2]) << 8) |
                         uint32_t(m_data[m_pos + 3]);
            m_pos += 4;
            return v;
        } else if (info == 27) {
            ensure_bytes(8);
            uint64_t v = 0;
            for (int i = 0; i < 8; ++i) {
                v = (v << 8) | uint64_t(m_data[m_pos + i]);
            }
            m_pos += 8;
            return v;
        }
        throw cbor_error("Indefinite length or invalid CBOR additional info: " + std::to_string(info));
    }
};

} // namespace detail

/**
 * @brief Serializes a Senko JSON value into a binary CBOR buffer (RFC 8949).
 */
inline std::vector<uint8_t> to_cbor(const value& j) {
    std::vector<uint8_t> out;
    detail::serialize_cbor_impl(j, out);
    return out;
}

/**
 * @brief Deserializes a binary CBOR buffer into a Senko JSON value.
 */
inline value from_cbor(const uint8_t* data, size_t size) {
    detail::cbor_reader reader(data, size);
    return reader.parse();
}

inline value from_cbor(const std::vector<uint8_t>& bytes) {
    return from_cbor(bytes.data(), bytes.size());
}

inline value from_cbor(std::string_view bytes) {
    return from_cbor(reinterpret_cast<const uint8_t*>(bytes.data()), bytes.size());
}

} // namespace senko


// ========================================================
// Header: macro.hpp
// ========================================================






namespace senko {

// Helper macros for automatic to_json / from_json struct binding
#define SENKO_TO_JSON(v, key) j[#key] = v.key;
#define SENKO_FROM_JSON(v, key) if (j.contains(#key)) { j.at(#key).get_to(v.key); }

// Preprocessor counting and dispatch
#define SENKO_ARG_N( \
    _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, \
    _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, \
    _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, \
    _31, _32, N, ...) N

#define SENKO_RSEQ_N() \
    32, 31, 30, 29, 28, 27, 26, 25, 24, 23, \
    22, 21, 20, 19, 18, 17, 16, 15, 14, 13, \
    12, 11, 10, 9, 8, 7, 6, 5, 4, 3, \
    2, 1, 0

#define SENKO_NARGS_(...) SENKO_EXPAND(SENKO_ARG_N(__VA_ARGS__))
#define SENKO_NARGS(...) SENKO_NARGS_(__VA_ARGS__, SENKO_RSEQ_N())
#define SENKO_EXPAND(x) x
#define SENKO_CONCAT(x, y) SENKO_CONCAT_(x, y)
#define SENKO_CONCAT_(x, y) x##y

#define SENKO_TO_1(v, a) SENKO_TO_JSON(v, a)
#define SENKO_TO_2(v, a, b) SENKO_TO_JSON(v, a) SENKO_TO_JSON(v, b)
#define SENKO_TO_3(v, a, b, c) SENKO_TO_JSON(v, a) SENKO_TO_JSON(v, b) SENKO_TO_JSON(v, c)
#define SENKO_TO_4(v, a, b, c, d) SENKO_TO_JSON(v, a) SENKO_TO_JSON(v, b) SENKO_TO_JSON(v, c) SENKO_TO_JSON(v, d)
#define SENKO_TO_5(v, a, b, c, d, e) SENKO_TO_JSON(v, a) SENKO_TO_JSON(v, b) SENKO_TO_JSON(v, c) SENKO_TO_JSON(v, d) SENKO_TO_JSON(v, e)
#define SENKO_TO_6(v, a, b, c, d, e, f) SENKO_TO_JSON(v, a) SENKO_TO_JSON(v, b) SENKO_TO_JSON(v, c) SENKO_TO_JSON(v, d) SENKO_TO_JSON(v, e) SENKO_TO_JSON(v, f)
#define SENKO_TO_7(v, a, b, c, d, e, f, g) SENKO_TO_JSON(v, a) SENKO_TO_JSON(v, b) SENKO_TO_JSON(v, c) SENKO_TO_JSON(v, d) SENKO_TO_JSON(v, e) SENKO_TO_JSON(v, f) SENKO_TO_JSON(v, g)
#define SENKO_TO_8(v, a, b, c, d, e, f, g, h) SENKO_TO_JSON(v, a) SENKO_TO_JSON(v, b) SENKO_TO_JSON(v, c) SENKO_TO_JSON(v, d) SENKO_TO_JSON(v, e) SENKO_TO_JSON(v, f) SENKO_TO_JSON(v, g) SENKO_TO_JSON(v, h)
#define SENKO_TO_9(v, a, b, c, d, e, f, g, h, i) SENKO_TO_JSON(v, a) SENKO_TO_JSON(v, b) SENKO_TO_JSON(v, c) SENKO_TO_JSON(v, d) SENKO_TO_JSON(v, e) SENKO_TO_JSON(v, f) SENKO_TO_JSON(v, g) SENKO_TO_JSON(v, h) SENKO_TO_JSON(v, i)
#define SENKO_TO_10(v, a, b, c, d, e, f, g, h, i, j) SENKO_TO_JSON(v, a) SENKO_TO_JSON(v, b) SENKO_TO_JSON(v, c) SENKO_TO_JSON(v, d) SENKO_TO_JSON(v, e) SENKO_TO_JSON(v, f) SENKO_TO_JSON(v, g) SENKO_TO_JSON(v, h) SENKO_TO_JSON(v, i) SENKO_TO_JSON(v, j)
#define SENKO_TO_11(v, a, b, c, d, e, f, g, h, i, j, k) SENKO_TO_10(v, a, b, c, d, e, f, g, h, i, j) SENKO_TO_JSON(v, k)
#define SENKO_TO_12(v, a, b, c, d, e, f, g, h, i, j, k, l) SENKO_TO_11(v, a, b, c, d, e, f, g, h, i, j, k) SENKO_TO_JSON(v, l)
#define SENKO_TO_13(v, a, b, c, d, e, f, g, h, i, j, k, l, m) SENKO_TO_12(v, a, b, c, d, e, f, g, h, i, j, k, l) SENKO_TO_JSON(v, m)
#define SENKO_TO_14(v, a, b, c, d, e, f, g, h, i, j, k, l, m, n) SENKO_TO_13(v, a, b, c, d, e, f, g, h, i, j, k, l, m) SENKO_TO_JSON(v, n)
#define SENKO_TO_15(v, a, b, c, d, e, f, g, h, i, j, k, l, m, n, o) SENKO_TO_14(v, a, b, c, d, e, f, g, h, i, j, k, l, m, n) SENKO_TO_JSON(v, o)
#define SENKO_TO_16(v, a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p) SENKO_TO_15(v, a, b, c, d, e, f, g, h, i, j, k, l, m, n, o) SENKO_TO_JSON(v, p)

#define SENKO_FROM_1(v, a) SENKO_FROM_JSON(v, a)
#define SENKO_FROM_2(v, a, b) SENKO_FROM_JSON(v, a) SENKO_FROM_JSON(v, b)
#define SENKO_FROM_3(v, a, b, c) SENKO_FROM_JSON(v, a) SENKO_FROM_JSON(v, b) SENKO_FROM_JSON(v, c)
#define SENKO_FROM_4(v, a, b, c, d) SENKO_FROM_JSON(v, a) SENKO_FROM_JSON(v, b) SENKO_FROM_JSON(v, c) SENKO_FROM_JSON(v, d)
#define SENKO_FROM_5(v, a, b, c, d, e) SENKO_FROM_JSON(v, a) SENKO_FROM_JSON(v, b) SENKO_FROM_JSON(v, c) SENKO_FROM_JSON(v, d) SENKO_FROM_JSON(v, e)
#define SENKO_FROM_6(v, a, b, c, d, e, f) SENKO_FROM_JSON(v, a) SENKO_FROM_JSON(v, b) SENKO_FROM_JSON(v, c) SENKO_FROM_JSON(v, d) SENKO_FROM_JSON(v, e) SENKO_FROM_JSON(v, f)
#define SENKO_FROM_7(v, a, b, c, d, e, f, g) SENKO_FROM_JSON(v, a) SENKO_FROM_JSON(v, b) SENKO_FROM_JSON(v, c) SENKO_FROM_JSON(v, d) SENKO_FROM_JSON(v, e) SENKO_FROM_JSON(v, f) SENKO_FROM_JSON(v, g)
#define SENKO_FROM_8(v, a, b, c, d, e, f, g, h) SENKO_FROM_JSON(v, a) SENKO_FROM_JSON(v, b) SENKO_FROM_JSON(v, c) SENKO_FROM_JSON(v, d) SENKO_FROM_JSON(v, e) SENKO_FROM_JSON(v, f) SENKO_FROM_JSON(v, g) SENKO_FROM_JSON(v, h)
#define SENKO_FROM_9(v, a, b, c, d, e, f, g, h, i) SENKO_FROM_JSON(v, a) SENKO_FROM_JSON(v, b) SENKO_FROM_JSON(v, c) SENKO_FROM_JSON(v, d) SENKO_FROM_JSON(v, e) SENKO_FROM_JSON(v, f) SENKO_FROM_JSON(v, g) SENKO_FROM_JSON(v, h) SENKO_FROM_JSON(v, i)
#define SENKO_FROM_10(v, a, b, c, d, e, f, g, h, i, j) SENKO_FROM_JSON(v, a) SENKO_FROM_JSON(v, b) SENKO_FROM_JSON(v, c) SENKO_FROM_JSON(v, d) SENKO_FROM_JSON(v, e) SENKO_FROM_JSON(v, f) SENKO_FROM_JSON(v, g) SENKO_FROM_JSON(v, h) SENKO_FROM_JSON(v, i) SENKO_FROM_JSON(v, j)
#define SENKO_FROM_11(v, a, b, c, d, e, f, g, h, i, j, k) SENKO_FROM_10(v, a, b, c, d, e, f, g, h, i, j) SENKO_FROM_JSON(v, k)
#define SENKO_FROM_12(v, a, b, c, d, e, f, g, h, i, j, k, l) SENKO_FROM_11(v, a, b, c, d, e, f, g, h, i, j, k) SENKO_FROM_JSON(v, l)
#define SENKO_FROM_13(v, a, b, c, d, e, f, g, h, i, j, k, l, m) SENKO_FROM_12(v, a, b, c, d, e, f, g, h, i, j, k, l) SENKO_FROM_JSON(v, m)
#define SENKO_FROM_14(v, a, b, c, d, e, f, g, h, i, j, k, l, m, n) SENKO_FROM_13(v, a, b, c, d, e, f, g, h, i, j, k, l, m) SENKO_FROM_JSON(v, n)
#define SENKO_FROM_15(v, a, b, c, d, e, f, g, h, i, j, k, l, m, n, o) SENKO_FROM_14(v, a, b, c, d, e, f, g, h, i, j, k, l, m, n) SENKO_FROM_JSON(v, o)
#define SENKO_FROM_16(v, a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p) SENKO_FROM_15(v, a, b, c, d, e, f, g, h, i, j, k, l, m, n, o) SENKO_FROM_JSON(v, p)

/**
 * @brief Macro to define struct/class serialization & deserialization functions.
 * Usage:
 * struct User {
 *     std::string name;
 *     int age;
 * };
 * SENKO_BIND(User, name, age)
 */
#define SENKO_BIND(Type, ...) \
    inline void to_json(::senko::value& j, const Type& v) { \
        j = ::senko::value::object(); \
        SENKO_EXPAND(SENKO_CONCAT(SENKO_TO_, SENKO_NARGS(__VA_ARGS__))(v, __VA_ARGS__)) \
    } \
    inline void from_json(const ::senko::value& j, Type& v) { \
        SENKO_EXPAND(SENKO_CONCAT(SENKO_FROM_, SENKO_NARGS(__VA_ARGS__))(v, __VA_ARGS__)) \
    }

// Aliases for convenience & backwards compatibility
#define SENKO_DEFINE_TYPE(Type, ...) SENKO_BIND(Type, __VA_ARGS__)
#define COREJSON_BIND(Type, ...) SENKO_BIND(Type, __VA_ARGS__)
#define COREJSON_DEFINE_TYPE(Type, ...) SENKO_BIND(Type, __VA_ARGS__)

} // namespace senko


namespace senko {
namespace literals {

inline value operator""_json(const char* str, size_t len) {
    return value::parse(std::string_view(str, len));
}

inline json_pointer operator""_json_pointer(const char* str, size_t len) {
    return json_pointer(std::string_view(str, len));
}

} // namespace literals
} // namespace senko

#endif // SENKO_SINGLE_AMALGAMATION_HPP
