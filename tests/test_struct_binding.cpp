#include "test_framework.hpp"
#include <senko/senko.hpp>

using json = senko::json;

struct Player {
    std::string name;
    int level;
    double health;
    bool is_online;
};
SENKO_BIND(Player, name, level, health, is_online)

struct Team {
    std::string team_name;
    int rank;
};
SENKO_BIND(Team, team_name, rank)

TEST_CASE("Struct Binding - Serialization & Deserialization") {
    Player p1{"DragonSlayer", 42, 98.5, true};

    // Serialize
    json j = p1;
    CHECK(j.is_object());
    CHECK_EQ(j["name"].get<std::string>(), "DragonSlayer");
    CHECK_EQ(j["level"].get<int>(), 42);
    CHECK(std::abs(j["health"].get<double>() - 98.5) < 0.01);
    CHECK_EQ(j["is_online"].get<bool>(), true);

    // Deserialize
    Player p2 = j.get<Player>();
    CHECK_EQ(p2.name, "DragonSlayer");
    CHECK_EQ(p2.level, 42);
    CHECK(std::abs(p2.health - 98.5) < 0.01);
    CHECK_EQ(p2.is_online, true);

    // Partial JSON Deserialization
    json partial = json::parse(R"({"name": "Noob", "level": 1})");
    Player p3 = partial.get<Player>();
    CHECK_EQ(p3.name, "Noob");
    CHECK_EQ(p3.level, 1);

    // Deserializing struct from non-object throws type_error
    json not_an_obj = 123;
    CHECK_THROWS(not_an_obj.get<Player>());
    json array_json = json::array({1, 2, 3});
    CHECK_THROWS(array_json.get<Player>());
}

struct BigConfig {
    int f1, f2, f3, f4, f5, f6, f7, f8, f9, f10;
    int f11, f12, f13, f14, f15, f16, f17, f18, f19, f20;
};
SENKO_BIND(BigConfig, f1, f2, f3, f4, f5, f6, f7, f8, f9, f10, f11, f12, f13, f14, f15, f16, f17, f18, f19, f20)

TEST_CASE("Struct Binding - Large Struct (20 Fields)") {
    BigConfig cfg{1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20};
    json j = cfg;
    CHECK_EQ(j.size(), 20);
    CHECK_EQ(j["f1"].get<int>(), 1);
    CHECK_EQ(j["f20"].get<int>(), 20);

    BigConfig loaded = j.get<BigConfig>();
    CHECK_EQ(loaded.f1, 1);
    CHECK_EQ(loaded.f20, 20);
}

