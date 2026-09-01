#pragma once

#include "fwd.hpp"
#include "value.hpp"

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

// Per-count macro expansions
#define SENKO_TO_1(v, a) SENKO_TO_JSON(v, a)
#define SENKO_TO_2(v, a, b) SENKO_TO_1(v, a) SENKO_TO_JSON(v, b)
#define SENKO_TO_3(v, a, b, c) SENKO_TO_2(v, a, b) SENKO_TO_JSON(v, c)
#define SENKO_TO_4(v, a, b, c, d) SENKO_TO_3(v, a, b, c) SENKO_TO_JSON(v, d)
#define SENKO_TO_5(v, a, b, c, d, e) SENKO_TO_4(v, a, b, c, d) SENKO_TO_JSON(v, e)
#define SENKO_TO_6(v, a, b, c, d, e, f) SENKO_TO_5(v, a, b, c, d, e) SENKO_TO_JSON(v, f)
#define SENKO_TO_7(v, a, b, c, d, e, f, g) SENKO_TO_6(v, a, b, c, d, e, f) SENKO_TO_JSON(v, g)
#define SENKO_TO_8(v, a, b, c, d, e, f, g, h) SENKO_TO_7(v, a, b, c, d, e, f, g) SENKO_TO_JSON(v, h)
#define SENKO_TO_9(v, a, b, c, d, e, f, g, h, i) SENKO_TO_8(v, a, b, c, d, e, f, g, h) SENKO_TO_JSON(v, i)
#define SENKO_TO_10(v, a, b, c, d, e, f, g, h, i, j) SENKO_TO_9(v, a, b, c, d, e, f, g, h, i) SENKO_TO_JSON(v, j)

#define SENKO_FROM_1(v, a) SENKO_FROM_JSON(v, a)
#define SENKO_FROM_2(v, a, b) SENKO_FROM_1(v, a) SENKO_FROM_JSON(v, b)
#define SENKO_FROM_3(v, a, b, c) SENKO_FROM_2(v, a, b) SENKO_FROM_JSON(v, c)
#define SENKO_FROM_4(v, a, b, c, d) SENKO_FROM_3(v, a, b, c) SENKO_FROM_JSON(v, d)
#define SENKO_FROM_5(v, a, b, c, d, e) SENKO_FROM_4(v, a, b, c, d, e) SENKO_FROM_JSON(v, e)
#define SENKO_FROM_6(v, a, b, c, d, e, f) SENKO_FROM_5(v, a, b, c, d, e, f) SENKO_FROM_JSON(v, f)
#define SENKO_FROM_7(v, a, b, c, d, e, f, g) SENKO_FROM_6(v, a, b, c, d, e, f, g) SENKO_FROM_JSON(v, g)
#define SENKO_FROM_8(v, a, b, c, d, e, f, g, h) SENKO_FROM_7(v, a, b, c, d, e, f, g) SENKO_FROM_JSON(v, h)
#define SENKO_FROM_9(v, a, b, c, d, e, f, g, h, i) SENKO_FROM_8(v, a, b, c, d, e, f, g, h, i) SENKO_FROM_JSON(v, i)
#define SENKO_FROM_10(v, a, b, c, d, e, f, g, h, i, j) SENKO_FROM_9(v, a, b, c, d, e, f, g, h, i) SENKO_FROM_JSON(v, j)

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
