#include "test_framework.hpp"
#include <senko/senko.hpp>

using json = senko::json;

TEST_CASE("Containers - Array Operations") {
    json arr = json::array();
    CHECK(arr.is_array());
    CHECK_EQ(arr.size(), 0);
    CHECK(arr.empty());

    arr.push_back(10);
    arr.push_back("second");
    arr.push_back(true);

    CHECK_EQ(arr.size(), 3);
    CHECK(!arr.empty());
    CHECK_EQ(arr[0].get<int>(), 10);
    CHECK_EQ(arr[1].get<std::string>(), "second");
    CHECK_EQ(arr[2].get<bool>(), true);

    // Initializer list
    json arr_init = {1, 2, 3, 4, 5};
    CHECK(arr_init.is_array());
    CHECK_EQ(arr_init.size(), 5);
    CHECK_EQ(arr_init[4].get<int>(), 5);

    // Out of range access
    CHECK_THROWS(arr_init.at(10));

    // Erase element
    CHECK(arr_init.erase(0));
    CHECK_EQ(arr_init.size(), 4);
    CHECK_EQ(arr_init[0].get<int>(), 2);
}

TEST_CASE("Containers - Object Operations") {
    json obj = json::object();
    CHECK(obj.is_object());
    CHECK_EQ(obj.size(), 0);

    obj["name"] = "SenkoJSON";
    obj["version"] = 2.0;
    obj["enabled"] = true;

    CHECK_EQ(obj.size(), 3);
    CHECK(obj.contains("name"));
    CHECK(obj.contains("version"));
    CHECK(!obj.contains("unknown_key"));

    CHECK_EQ(obj["name"].get<std::string>(), "SenkoJSON");
    CHECK_EQ(obj.value_or("missing", 999), 999);
    CHECK_EQ(obj.value_or("name", std::string("default")), "SenkoJSON");

    // at() throws on missing
    CHECK_THROWS(obj.at("missing_key"));

    // Erase key
    CHECK(obj.erase("version"));
    CHECK_EQ(obj.size(), 2);
    CHECK(!obj.contains("version"));
}

TEST_CASE("Containers - Nested Structures") {
    std::string json_str = R"({
        "user": {
            "id": 101,
            "profile": {
                "username": "coder_pro",
                "emails": ["a@b.com", "c@d.com"]
            }
        },
        "tags": ["c++", "json", "fast"]
    })";

    json doc = json::parse(json_str);
    CHECK_EQ(doc["user"]["id"].get<int>(), 101);
    CHECK_EQ(doc["user"]["profile"]["username"].get<std::string>(), "coder_pro");
    CHECK_EQ(doc["user"]["profile"]["emails"][1].get<std::string>(), "c@d.com");
    CHECK_EQ(doc["tags"].size(), 3);
    CHECK_EQ(doc["tags"][2].get<std::string>(), "fast");

    // Modify nested
    doc["user"]["profile"]["username"] = "super_coder";
    CHECK_EQ(doc["user"]["profile"]["username"].get<std::string>(), "super_coder");
}
