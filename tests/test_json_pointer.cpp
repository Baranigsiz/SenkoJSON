#include "test_framework.hpp"
#include <corejson/corejson.hpp>

using json = corejson::json;
using namespace corejson::literals;

TEST_CASE("JSON Pointer - RFC 6901 Specification Tests") {
    // Official RFC 6901 Example Document
    std::string rfc_doc = R"({
        "foo": ["bar", "baz"],
        "": 0,
        "a/b": 1,
        "c%d": 2,
        "e^f": 3,
        "g|h": 4,
        "i\\j": 5,
        "k\"l": 6,
        " ": 7,
        "m~n": 8
    })";

    json doc = json::parse(rfc_doc);

    // "" -> whole document
    CHECK_EQ(doc[""_json_pointer]["foo"][0].get<std::string>(), "bar");

    // "/foo" -> ["bar", "baz"]
    CHECK_EQ(doc["/foo"_json_pointer].size(), 2);

    // "/foo/0" -> "bar"
    CHECK_EQ(doc["/foo/0"_json_pointer].get<std::string>(), "bar");

    // "/" -> 0
    CHECK_EQ(doc["/"_json_pointer].get<int>(), 0);

    // "/a~1b" -> 1
    CHECK_EQ(doc["/a~1b"_json_pointer].get<int>(), 1);

    // "/c%d" -> 2
    CHECK_EQ(doc["/c%d"_json_pointer].get<int>(), 2);

    // "/e^f" -> 3
    CHECK_EQ(doc["/e^f"_json_pointer].get<int>(), 3);

    // "/g|h" -> 4
    CHECK_EQ(doc["/g|h"_json_pointer].get<int>(), 4);

    // "/i\\j" -> 5
    CHECK_EQ(doc["/i\\j"_json_pointer].get<int>(), 5);

    // "/k\"l" -> 6
    CHECK_EQ(doc["/k\"l"_json_pointer].get<int>(), 6);

    // "/ " -> 7
    CHECK_EQ(doc["/ "_json_pointer].get<int>(), 7);

    // "/m~0n" -> 8
    CHECK_EQ(doc["/m~0n"_json_pointer].get<int>(), 8);
}

TEST_CASE("JSON Pointer - Error Handling") {
    json doc = json::parse(R"({"a": {"b": [10, 20]}})");

    // Invalid start char
    CHECK_THROWS(doc["invalid_start"_json_pointer]);

    // Key not found
    CHECK_THROWS(doc["/a/non_existent"_json_pointer]);

    // Array index out of range
    CHECK_THROWS(doc["/a/b/5"_json_pointer]);

    // Non-integer index into array
    CHECK_THROWS(doc["/a/b/invalid"_json_pointer]);
}
