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

TEST_CASE("File I/O - Stream-Based Serialization") {
    json doc = {
        {"name", "Senko"},
        {"age", 800},
        {"nested", {{"fluffy", true}, {"tail_count", 1}}},
        {"list", {10, 20, 30}}
    };

    // 1. Compact stream serialization
    std::ostringstream oss_compact;
    doc.dump(oss_compact, -1);
    CHECK_EQ(oss_compact.str(), doc.dump(-1));

    // 2. Indented stream serialization (indent = 2)
    std::ostringstream oss_indented;
    doc.dump(oss_indented, 2);
    CHECK_EQ(oss_indented.str(), doc.dump(2));

    // 3. Operator << output
    std::ostringstream oss_op;
    oss_op << doc;
    CHECK_EQ(oss_op.str(), doc.dump());
}

