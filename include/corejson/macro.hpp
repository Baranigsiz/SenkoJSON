#pragma once

#include "fwd.hpp"
#include "value.hpp"

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
