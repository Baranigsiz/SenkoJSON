#include "test_framework.hpp"
#include <senko/senko.hpp>

using json = senko::json;

TEST_CASE("JSON Patch - RFC 6902 Operations (Add, Remove, Replace)") {
    json doc = json::parse(R"({
        "foo": "bar",
        "numbers": [1, 2, 3]
    })");

    // 1. Add key
    json patch_add = json::parse(R"([
        {"op": "add", "path": "/baz", "value": "qux"},
        {"op": "add", "path": "/numbers/-", "value": 4}
    ])");
    doc.patch_in_place(patch_add);
    CHECK_EQ(doc["baz"].get<std::string>(), "qux");
    CHECK_EQ(doc["numbers"].size(), 4);
    CHECK_EQ(doc["numbers"][3].get<int>(), 4);

    // 2. Replace key
    json patch_replace = json::parse(R"([
        {"op": "replace", "path": "/foo", "value": "updated_bar"}
    ])");
    doc.patch_in_place(patch_replace);
    CHECK_EQ(doc["foo"].get<std::string>(), "updated_bar");

    // 3. Remove key & array index
    json patch_remove = json::parse(R"([
        {"op": "remove", "path": "/baz"},
        {"op": "remove", "path": "/numbers/0"}
    ])");
    doc.patch_in_place(patch_remove);
    CHECK(!doc.contains("baz"));
    CHECK_EQ(doc["numbers"].size(), 3);
    CHECK_EQ(doc["numbers"][0].get<int>(), 2);
}

TEST_CASE("JSON Patch - Move, Copy, Test Operations") {
    json doc = json::parse(R"({
        "original": "value123",
        "list": [10, 20]
    })");

    // Copy operation
    json patch_copy = json::parse(R"([
        {"op": "copy", "from": "/original", "path": "/copied"}
    ])");
    doc.patch_in_place(patch_copy);
    CHECK_EQ(doc["copied"].get<std::string>(), "value123");
    CHECK_EQ(doc["original"].get<std::string>(), "value123");

    // Move operation
    json patch_move = json::parse(R"([
        {"op": "move", "from": "/copied", "path": "/moved"}
    ])");
    doc.patch_in_place(patch_move);
    CHECK(!doc.contains("copied"));
    CHECK_EQ(doc["moved"].get<std::string>(), "value123");

    // Test operation (success)
    json patch_test_pass = json::parse(R"([
        {"op": "test", "path": "/moved", "value": "value123"}
    ])");
    CHECK_NOTHROW(doc.patch_in_place(patch_test_pass));

    // Test operation (failure)
    json patch_test_fail = json::parse(R"([
        {"op": "test", "path": "/moved", "value": "wrong_value"}
    ])");
    CHECK_THROWS(doc.patch_in_place(patch_test_fail));
}

TEST_CASE("JSON Diff - RFC 6902 Diff Generation & Re-application") {
    json source = json::parse(R"({
        "server": "alpha",
        "port": 80,
        "features": ["auth", "logs"],
        "deprecated": true
    })");

    json target = json::parse(R"({
        "server": "alpha",
        "port": 443,
        "features": ["auth", "logs", "ssl"],
        "status": "ready"
    })");

    // Generate diff
    json generated_patch = json::diff(source, target);
    CHECK(generated_patch.is_array());
    CHECK(!generated_patch.empty());

    // Apply diff to source
    json transformed = source.patch(generated_patch);

    // Verify transformed exactly equals target!
    CHECK_EQ(transformed, target);
}
