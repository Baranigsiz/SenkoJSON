#include <iostream>
#include <fstream>
#include <sstream>
#include <corejson/corejson.hpp>

using json = corejson::json;

int main() {
    std::string inputFile = "apex_config.json";
    std::string outputFile = "optimized_config.json";

    std::cout << "====================================================\n";
    std::cout << "         CoreJSON v2.0 - Configuration Optimizer    \n";
    std::cout << "====================================================\n\n";

    // 1. Read input configuration
    std::ifstream fileIn(inputFile);
    if (!fileIn.is_open()) {
        std::cerr << "[ERROR] Could not open '" << inputFile << "'. Creating default.\n";
        // Create sample config on the fly
        json sample_config = {
            {"project", "ApexLegendsOptimizer"},
            {"version", "2.0.0"},
            {"performance", {
                {"max_fps", 144},
                {"reduce_lag", false},
                {"resolution_scale", 1.0}
            }},
            {"audio", {
                {"master_volume", 0.8},
                {"spatial_audio", true}
            }}
        };
        std::ofstream fileInit(inputFile);
        fileInit << sample_config.dump(4);
        fileInit.close();
        fileIn.open(inputFile);
    }

    try {
        // 2. Parse directly with json::parse
        std::cout << "[INFO] Parsing configuration with CoreJSON v2.0...\n";
        json root = json::parse(fileIn);
        fileIn.close();

        // 3. Display current settings
        std::cout << "[INFO] Current Config Loaded:\n" << root.dump(2) << "\n\n";

        // 4. Modify settings with clean, modern syntax
        std::cout << "[INFO] Applying high-performance optimizations...\n";
        root["performance"]["max_fps"] = 240;
        root["performance"]["reduce_lag"] = true;
        root["performance"]["adaptive_supersampling"] = true;
        root["metadata"]["last_optimized"] = "2026-09-01T18:30:00Z";

        // 5. Save back to output file
        std::cout << "[INFO] Saving optimized configuration to '" << outputFile << "'...\n";
        std::ofstream fileOut(outputFile);
        if (fileOut.is_open()) {
            fileOut << root.dump(4);
            fileOut.close();
            std::cout << "[SUCCESS] File saved successfully!\n\n";
        }

        std::cout << "--- Optimized Result Preview ---\n";
        std::cout << root.dump(2) << "\n";

    } catch (const std::exception& e) {
        std::cerr << "[ERROR] Exception caught: " << e.what() << "\n";
        return 1;
    }

    return 0;
}