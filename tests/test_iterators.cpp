#include "test_framework.hpp"
#include <senko/senko.hpp>

using json = senko::json;

TEST_CASE("Iterators - Range-Based For Loop on Array") {
    json arr = {1, 2, 3, 4, 5};

    int sum = 0;
    for (const auto& item : arr) {
        sum += item.get<int>();
    }
    CHECK_EQ(sum, 15);

    // Mutable iteration
    for (auto& item : arr) {
        item = item.get<int>() * 2;
    }
    CHECK_EQ(arr[0].get<int>(), 2);
    CHECK_EQ(arr[4].get<int>(), 10);

    // Const iteration
    const json const_arr = arr;
    int count = 0;
    for (auto it = const_arr.cbegin(); it != const_arr.cend(); ++it) {
        count++;
    }
    CHECK_EQ(count, 5);

    // Non-array throws type_error
    json not_arr = "string";
    CHECK_THROWS(not_arr.begin());
}

TEST_CASE("Iterators - Object Items Proxy & Structured Binding") {
    json obj = {
        {"name", "Senko"},
        {"role", "Assistant"},
        {"speed", "Flash"}
    };

    int count = 0;
    for (auto& [key, val] : obj.items()) {
        count++;
        if (key == "name") {
            CHECK_EQ(val.get<std::string>(), "Senko");
            val = "Senko-san"; // test mutation
        }
    }
    CHECK_EQ(count, 3);
    CHECK_EQ(obj["name"].get<std::string>(), "Senko-san");

    // Const object items iteration
    const json const_obj = obj;
    bool found_role = false;
    for (const auto& [key, val] : const_obj.items()) {
        if (key == "role" && val.get<std::string>() == "Assistant") {
            found_role = true;
        }
    }
    CHECK(found_role);

    // Non-object throws type_error
    json arr = {1, 2, 3};
    CHECK_THROWS(arr.items());
}
