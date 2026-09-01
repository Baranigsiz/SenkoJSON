#include <iostream>
#include <chrono>
#include <vector>
#include <iomanip>
#include <numeric>
#include <corejson/corejson.hpp>

using json = corejson::json;

template <typename Func>
double benchmark(const std::string& name, Func&& func, size_t iterations, size_t data_bytes) {
    // Warmup
    for (size_t i = 0; i < std::min<size_t>(iterations / 5 + 1, 100); ++i) {
        func();
    }

    auto start = std::chrono::high_resolution_clock::now();
    for (size_t i = 0; i < iterations; ++i) {
        func();
    }
    auto end = std::chrono::high_resolution_clock::now();

    std::chrono::duration<double, std::milli> elapsed = end - start;
    double total_ms = elapsed.count();
    double avg_us = (total_ms * 1000.0) / iterations;
    double ops_per_sec = (iterations / total_ms) * 1000.0;
    double mb_per_sec = ((data_bytes * iterations) / (1024.0 * 1024.0)) / (total_ms / 1000.0);

    std::cout << std::left << std::setw(32) << name
              << " | " << std::right << std::setw(8) << std::fixed << std::setprecision(2) << avg_us << " us/op"
              << " | " << std::setw(10) << std::fixed << std::setprecision(0) << ops_per_sec << " ops/s"
              << " | " << std::setw(8) << std::fixed << std::setprecision(2) << mb_per_sec << " MB/s\n";

    return mb_per_sec;
}

int main() {
    std::cout << "========================================================================\n";
    std::cout << "                   CoreJSON v2.0 Performance Benchmarks                 \n";
    std::cout << "========================================================================\n";

    // 1. Small JSON Payload (REST API response)
    std::string small_json = R"({"id":12345,"name":"CoreJSON Library","active":true,"rating":4.95,"tags":["c++","json","fast"]})";
    
    // 2. Medium JSON Payload (Configuration / List of items)
    std::string medium_json = R"({
        "status": "success",
        "code": 200,
        "users": [
            {"id": 1, "username": "alice", "roles": ["admin", "editor"], "stats": {"posts": 142, "score": 98.6}},
            {"id": 2, "username": "bob", "roles": ["user"], "stats": {"posts": 12, "score": 45.2}},
            {"id": 3, "username": "charlie", "roles": ["user", "moderator"], "stats": {"posts": 88, "score": 79.1}},
            {"id": 4, "username": "diana", "roles": ["viewer"], "stats": {"posts": 3, "score": 22.0}},
            {"id": 5, "username": "eve", "roles": ["admin"], "stats": {"posts": 305, "score": 99.9}}
        ],
        "metadata": {
            "total_count": 5,
            "page": 1,
            "per_page": 20,
            "server_time": "2026-09-01T18:00:00Z"
        }
    })";

    // 3. Large Array of Numbers
    std::string large_array_json = "[";
    for (int i = 0; i < 5000; ++i) {
        large_array_json += std::to_string(i * 1.5);
        if (i < 4999) large_array_json += ",";
    }
    large_array_json += "]";

    std::cout << "\n--- Parsing Benchmarks ---\n";
    benchmark("Parse Small JSON (106 B)", [&]() {
        auto doc = json::parse(small_json);
        (void)doc;
    }, 50000, small_json.size());

    benchmark("Parse Medium JSON (630 B)", [&]() {
        auto doc = json::parse(medium_json);
        (void)doc;
    }, 20000, medium_json.size());

    benchmark("Parse Number Array (5000 items)", [&]() {
        auto doc = json::parse(large_array_json);
        (void)doc;
    }, 2000, large_array_json.size());

    std::cout << "\n--- Serialization Benchmarks ---\n";
    json medium_doc = json::parse(medium_json);

    benchmark("Dump Minified Medium JSON", [&]() {
        std::string s = medium_doc.dump(-1);
        (void)s;
    }, 20000, medium_json.size());

    benchmark("Dump Pretty Medium JSON (indent 4)", [&]() {
        std::string s = medium_doc.dump(4);
        (void)s;
    }, 15000, medium_json.size());

    std::cout << "\n========================================================================\n";
    return 0;
}
