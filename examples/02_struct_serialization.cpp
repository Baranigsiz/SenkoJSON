#include <iostream>
#include <vector>
#include <corejson/corejson.hpp>

using json = corejson::json;

// Define custom structs
struct ServerConfig {
    std::string host;
    int port;
    bool ssl_enabled;
};
COREJSON_BIND(ServerConfig, host, port, ssl_enabled)

struct DatabaseConfig {
    std::string engine;
    std::string db_name;
    int max_connections;
};
COREJSON_BIND(DatabaseConfig, engine, db_name, max_connections)

struct AppConfig {
    std::string app_name;
    ServerConfig server;
    DatabaseConfig database;
};
COREJSON_BIND(AppConfig, app_name, server, database)

int main() {
    std::cout << "=== CoreJSON v2.0 - Struct Serialization Demo ===\n\n";

    // 1. Convert C++ Struct to JSON
    AppConfig config{
        "MyAwesomeMicroservice",
        ServerConfig{"0.0.0.0", 8080, true},
        DatabaseConfig{"PostgreSQL", "production_db", 50}
    };

    json serialized = config;
    std::cout << "Serialized C++ Struct to JSON:\n";
    std::cout << serialized.dump(4) << "\n\n";

    // 2. Deserializing from JSON text directly into C++ struct
    std::string json_input = R"({
        "app_name": "UpdatedService",
        "server": {
            "host": "127.0.0.1",
            "port": 9000,
            "ssl_enabled": false
        },
        "database": {
            "engine": "MySQL",
            "db_name": "users_db",
            "max_connections": 100
        }
    })";

    json parsed = json::parse(json_input);
    AppConfig loaded = parsed.get<AppConfig>();

    std::cout << "Deserialized back into C++ Struct:\n";
    std::cout << "App Name: " << loaded.app_name << "\n";
    std::cout << "Server: " << loaded.server.host << ":" << loaded.server.port << " (SSL: " << (loaded.server.ssl_enabled ? "Yes" : "No") << ")\n";
    std::cout << "DB Engine: " << loaded.database.engine << " on db '" << loaded.database.db_name << "'\n";

    return 0;
}
