#include "test_framework.hpp"
#include <senko/senko.hpp>
#include <cstdio>

using json = senko::json;

TEST_CASE("File I/O - Dump and Parse File") {
    const std::string test_path = "temp_test_config.json";

    json doc = {
        {"project", "SenkoJSON"},
        {"version", "2.2.0"},
        {"features", {"speed", "cbor", "msgpack", "jsonpath"}},
        {"stars", 1500}
    };

    // Dump to file
    CHECK_NOTHROW(doc.dump_file(test_path, 2));

    // Parse from file
    json loaded;
    CHECK_NOTHROW(loaded = json::parse_file(test_path));

    CHECK_EQ(loaded["project"].get<std::string>(), "SenkoJSON");
    CHECK_EQ(loaded["stars"].get<int>(), 1500);
    CHECK_EQ(loaded["features"].size(), 4);
    CHECK_EQ(loaded, doc);

    // Clean up test file
    std::remove(test_path.c_str());

    // Non-existent file should throw
    CHECK_THROWS(json::parse_file("non_existent_file_12345.json"));
}
