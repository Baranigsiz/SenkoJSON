#pragma once

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
