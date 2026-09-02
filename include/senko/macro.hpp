#pragma once

#include "fwd.hpp"
#include "value.hpp"

namespace senko {

// Helper macros for automatic to_json / from_json struct binding
#define SENKO_TO_JSON(v, key) j[#key] = v.key;
#define SENKO_FROM_JSON(v, key) if (const auto* _senko_ptr = j.find(#key)) { _senko_ptr->get_to(v.key); }

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
#define SENKO_TO_17(v, a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p, q) SENKO_TO_16(v, a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p) SENKO_TO_JSON(v, q)
#define SENKO_TO_18(v, a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p, q, r) SENKO_TO_17(v, a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p, q) SENKO_TO_JSON(v, r)
#define SENKO_TO_19(v, a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p, q, r, s) SENKO_TO_18(v, a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p, q, r) SENKO_TO_JSON(v, s)
#define SENKO_TO_20(v, a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p, q, r, s, t) SENKO_TO_19(v, a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p, q, r, s) SENKO_TO_JSON(v, t)
#define SENKO_TO_21(v, a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p, q, r, s, t, u) SENKO_TO_20(v, a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p, q, r, s, t) SENKO_TO_JSON(v, u)
#define SENKO_TO_22(v, a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p, q, r, s, t, u, w) SENKO_TO_21(v, a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p, q, r, s, t, u) SENKO_TO_JSON(v, w)
#define SENKO_TO_23(v, a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p, q, r, s, t, u, w, x) SENKO_TO_22(v, a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p, q, r, s, t, u, w) SENKO_TO_JSON(v, x)
#define SENKO_TO_24(v, a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p, q, r, s, t, u, w, x, y) SENKO_TO_23(v, a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p, q, r, s, t, u, w, x) SENKO_TO_JSON(v, y)
#define SENKO_TO_25(v, a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p, q, r, s, t, u, w, x, y, z) SENKO_TO_24(v, a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p, q, r, s, t, u, w, x, y) SENKO_TO_JSON(v, z)
#define SENKO_TO_26(v, a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p, q, r, s, t, u, w, x, y, z, _1) SENKO_TO_25(v, a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p, q, r, s, t, u, w, x, y, z) SENKO_TO_JSON(v, _1)
#define SENKO_TO_27(v, a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p, q, r, s, t, u, w, x, y, z, _1, _2) SENKO_TO_26(v, a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p, q, r, s, t, u, w, x, y, z, _1) SENKO_TO_JSON(v, _2)
#define SENKO_TO_28(v, a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p, q, r, s, t, u, w, x, y, z, _1, _2, _3) SENKO_TO_27(v, a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p, q, r, s, t, u, w, x, y, z, _1, _2) SENKO_TO_JSON(v, _3)
#define SENKO_TO_29(v, a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p, q, r, s, t, u, w, x, y, z, _1, _2, _3, _4) SENKO_TO_28(v, a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p, q, r, s, t, u, w, x, y, z, _1, _2, _3) SENKO_TO_JSON(v, _4)
#define SENKO_TO_30(v, a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p, q, r, s, t, u, w, x, y, z, _1, _2, _3, _4, _5) SENKO_TO_29(v, a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p, q, r, s, t, u, w, x, y, z, _1, _2, _3, _4) SENKO_TO_JSON(v, _5)
#define SENKO_TO_31(v, a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p, q, r, s, t, u, w, x, y, z, _1, _2, _3, _4, _5, _6) SENKO_TO_30(v, a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p, q, r, s, t, u, w, x, y, z, _1, _2, _3, _4, _5) SENKO_TO_JSON(v, _6)
#define SENKO_TO_32(v, a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p, q, r, s, t, u, w, x, y, z, _1, _2, _3, _4, _5, _6, _7) SENKO_TO_31(v, a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p, q, r, s, t, u, w, x, y, z, _1, _2, _3, _4, _5, _6) SENKO_TO_JSON(v, _7)

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
#define SENKO_FROM_17(v, a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p, q) SENKO_FROM_16(v, a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p) SENKO_FROM_JSON(v, q)
#define SENKO_FROM_18(v, a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p, q, r) SENKO_FROM_17(v, a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p, q) SENKO_FROM_JSON(v, r)
#define SENKO_FROM_19(v, a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p, q, r, s) SENKO_FROM_18(v, a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p, q, r) SENKO_FROM_JSON(v, s)
#define SENKO_FROM_20(v, a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p, q, r, s, t) SENKO_FROM_19(v, a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p, q, r, s) SENKO_FROM_JSON(v, t)
#define SENKO_FROM_21(v, a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p, q, r, s, t, u) SENKO_FROM_20(v, a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p, q, r, s, t) SENKO_FROM_JSON(v, u)
#define SENKO_FROM_22(v, a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p, q, r, s, t, u, w) SENKO_FROM_21(v, a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p, q, r, s, t, u) SENKO_FROM_JSON(v, w)
#define SENKO_FROM_23(v, a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p, q, r, s, t, u, w, x) SENKO_FROM_22(v, a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p, q, r, s, t, u, w) SENKO_FROM_JSON(v, x)
#define SENKO_FROM_24(v, a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p, q, r, s, t, u, w, x, y) SENKO_FROM_23(v, a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p, q, r, s, t, u, w, x) SENKO_FROM_JSON(v, y)
#define SENKO_FROM_25(v, a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p, q, r, s, t, u, w, x, y, z) SENKO_FROM_24(v, a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p, q, r, s, t, u, w, x, y) SENKO_FROM_JSON(v, z)
#define SENKO_FROM_26(v, a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p, q, r, s, t, u, w, x, y, z, _1) SENKO_FROM_25(v, a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p, q, r, s, t, u, w, x, y, z) SENKO_FROM_JSON(v, _1)
#define SENKO_FROM_27(v, a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p, q, r, s, t, u, w, x, y, z, _1, _2) SENKO_FROM_26(v, a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p, q, r, s, t, u, w, x, y, z, _1) SENKO_FROM_JSON(v, _2)
#define SENKO_FROM_28(v, a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p, q, r, s, t, u, w, x, y, z, _1, _2, _3) SENKO_FROM_27(v, a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p, q, r, s, t, u, w, x, y, z, _1, _2) SENKO_FROM_JSON(v, _3)
#define SENKO_FROM_29(v, a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p, q, r, s, t, u, w, x, y, z, _1, _2, _3, _4) SENKO_FROM_28(v, a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p, q, r, s, t, u, w, x, y, z, _1, _2, _3) SENKO_FROM_JSON(v, _4)
#define SENKO_FROM_30(v, a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p, q, r, s, t, u, w, x, y, z, _1, _2, _3, _4, _5) SENKO_FROM_29(v, a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p, q, r, s, t, u, w, x, y, z, _1, _2, _3, _4) SENKO_FROM_JSON(v, _5)
#define SENKO_FROM_31(v, a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p, q, r, s, t, u, w, x, y, z, _1, _2, _3, _4, _5, _6) SENKO_FROM_30(v, a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p, q, r, s, t, u, w, x, y, z, _1, _2, _3, _4, _5) SENKO_FROM_JSON(v, _6)
#define SENKO_FROM_32(v, a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p, q, r, s, t, u, w, x, y, z, _1, _2, _3, _4, _5, _6, _7) SENKO_FROM_31(v, a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p, q, r, s, t, u, w, x, y, z, _1, _2, _3, _4, _5, _6) SENKO_FROM_JSON(v, _7)

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
        if (!j.is_object()) { \
            throw ::senko::type_error("Expected object for struct deserialization, got " + std::string(j.type_name())); \
        } \
        SENKO_EXPAND(SENKO_CONCAT(SENKO_FROM_, SENKO_NARGS(__VA_ARGS__))(v, __VA_ARGS__)) \
    }

// Aliases for convenience & backwards compatibility
#define SENKO_DEFINE_TYPE(Type, ...) SENKO_BIND(Type, __VA_ARGS__)
#define COREJSON_BIND(Type, ...) SENKO_BIND(Type, __VA_ARGS__)
#define COREJSON_DEFINE_TYPE(Type, ...) SENKO_BIND(Type, __VA_ARGS__)

} // namespace senko
