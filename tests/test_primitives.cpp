#include "test_framework.hpp"
#include <senko/senko.hpp>

using json = senko::json;
using namespace senko::literals;

TEST_CASE("Primitives - Null Type") {
    json j_null;
    CHECK(j_null.is_null());
    CHECK_EQ(j_null.type_name(), "null");
    CHECK_EQ(j_null.dump(), "null");

    json parsed = json::parse("null");
    CHECK(parsed.is_null());
}

TEST_CASE("Primitives - Boolean Type") {
    json j_true = true;
    json j_false = false;

    CHECK(j_true.is_boolean());
    CHECK(j_false.is_boolean());
    CHECK_EQ(j_true.get<bool>(), true);
    CHECK_EQ(j_false.get<bool>(), false);
    CHECK_EQ(j_true.dump(), "true");
    CHECK_EQ(j_false.dump(), "false");

    json parsed_t = json::parse("true");
    json parsed_f = json::parse("false");
    CHECK_EQ(parsed_t.get<bool>(), true);
    CHECK_EQ(parsed_f.get<bool>(), false);
}

TEST_CASE("Primitives - Integer & Unsigned Numbers") {
    json j_pos = 42;
    json j_neg = -100;
    json j_large_u = uint64_t(18446744073709551615ULL);

    CHECK(j_pos.is_number_integer());
    CHECK_EQ(j_pos.get<int>(), 42);
    CHECK_EQ(j_neg.get<int>(), -100);
    CHECK_EQ(j_pos.dump(), "42");
    CHECK_EQ(j_neg.dump(), "-100");

    json parsed_int = json::parse("-987654321");
    CHECK_EQ(parsed_int.get<int64_t>(), -987654321LL);

    json parsed_large = json::parse("18446744073709551615");
    CHECK(parsed_large.is_number_unsigned());
    CHECK_EQ(parsed_large.get<uint64_t>(), 18446744073709551615ULL);
}

TEST_CASE("Primitives - Floating Point Numbers") {
    json j_float = 3.14159;
    CHECK(j_float.is_number_float());
    CHECK(std::abs(j_float.get<double>() - 3.14159) < 1e-5);

    json parsed_sci = json::parse("1.25e-3");
    CHECK(std::abs(parsed_sci.get<double>() - 0.00125) < 1e-6);

    json parsed_zero = json::parse("0.0");
    CHECK(std::abs(parsed_zero.get<double>() - 0.0) < 1e-9);
}

TEST_CASE("Primitives - String Type & Escapes") {
    json j_str = "Hello, World!";
    CHECK(j_str.is_string());
    CHECK_EQ(j_str.get<std::string>(), "Hello, World!");
    CHECK_EQ(j_str.dump(), "\"Hello, World!\"");

    json parsed = json::parse(R"("Quotes \"and\" newlines \n and tabs \t")");
    CHECK_EQ(parsed.get<std::string>(), "Quotes \"and\" newlines \n and tabs \t");

    // Check round-trip serialization of string with special chars
    json special = "Line 1\nLine 2\t\"Tabbed\"";
    std::string dumped = special.dump();
    json reparsed = json::parse(dumped);
    CHECK_EQ(reparsed.get<std::string>(), special.get<std::string>());
}
