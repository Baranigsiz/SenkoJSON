#include "test_framework.hpp"
#include <senko/senko.hpp>
#include <sstream>

using json = senko::json;
using namespace senko::literals;

TEST_CASE("JSONC - Comments and Trailing Commas") {
    std::string config_text = R"({
        // Server network configuration
        "host": "127.0.0.1",
        "port": 9000, /* Default listener port */
        "tls": true,
        // Allowed client origins:
        "origins": [
            "https://localhost",
            "https://example.com", // trailing comma
        ],
    })";

    json doc = senko::jsonc::parse(config_text);
    CHECK_EQ(doc["host"].get<std::string>(), "127.0.0.1");
    CHECK_EQ(doc["port"].get<int>(), 9000);
    CHECK_EQ(doc["tls"].get<bool>(), true);
    CHECK_EQ(doc["origins"].size(), 2);
    CHECK_EQ(doc["origins"][1].get<std::string>(), "https://example.com");

    // Using _jsonc literal
    auto jc = R"({
        "debug": true, // toggle logging
        "workers": 8,
    })"_jsonc;
    CHECK_EQ(jc["debug"].get<bool>(), true);
    CHECK_EQ(jc["workers"].get<int>(), 8);
}

TEST_CASE("JSONL - Streaming Lines & Datasets") {
    std::string jsonl_data = 
        "{\"id\": 1, \"prompt\": \"Hello, how are you?\", \"rating\": 5}\n"
        "{\"id\": 2, \"prompt\": \"What is SenkoJSON?\", \"rating\": 10}\n"
        "\n" // Blank line should be skipped
        "{\"id\": 3, \"prompt\": \"Write C++ code.\", \"rating\": 9}\n";

    senko::jsonl_reader reader(jsonl_data);
    std::vector<int> ids;
    std::vector<int> ratings;

    for (const auto& line_doc : reader) {
        ids.push_back(line_doc["id"].get<int>());
        ratings.push_back(line_doc["rating"].get<int>());
    }

    CHECK_EQ(ids.size(), 3);
    CHECK_EQ(ids[0], 1);
    CHECK_EQ(ids[1], 2);
    CHECK_EQ(ids[2], 3);

    CHECK_EQ(ratings.size(), 3);
    CHECK_EQ(ratings[1], 10);

    // Stream-based test
    std::istringstream iss(jsonl_data);
    auto stream_reader = senko::jsonl::parse(iss);
    int count = 0;
    for (const auto& doc : stream_reader) {
        (void)doc;
        count++;
    }
    CHECK_EQ(count, 3);
}
