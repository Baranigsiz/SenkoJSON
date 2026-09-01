#include "test_framework.hpp"
#include <corejson/corejson.hpp>

using json = corejson::json;

struct Player {
    std::string name;
    int level;
    double health;
    bool is_online;
};
COREJSON_BIND(Player, name, level, health, is_online)

struct Team {
    std::string team_name;
    int rank;
};
COREJSON_BIND(Team, team_name, rank)

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
}
