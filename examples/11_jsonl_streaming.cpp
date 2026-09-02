#include <iostream>
#include <senko/senko.hpp>

using json = senko::json;

int main() {
    std::cout << "=== SenkoJSON - JSONL / NDJSON Streaming Demo ===\n\n";

    // Simulating an LLM training dataset or high-throughput log file
    std::string dataset = 
        "{\"prompt\": \"What is C++?\", \"completion\": \"A high-performance compiled language.\", \"tokens\": 12}\n"
        "{\"prompt\": \"Explain JSON.\", \"completion\": \"JavaScript Object Notation format.\", \"tokens\": 10}\n"
        "\n" // Blank line automatically skipped
        "{\"prompt\": \"Why SenkoJSON?\", \"completion\": \"Because it is fast, header-only, and feature-rich!\", \"tokens\": 18}\n";

    senko::jsonl_reader reader(dataset);

    int total_tokens = 0;
    int index = 1;

    std::cout << "Streaming records line-by-line (constant memory usage):\n";
    for (const auto& record : reader) {
        std::cout << " [" << index++ << "] " << record["prompt"].get<std::string>()
                  << " -> " << record["completion"].get<std::string>() << "\n";
        total_tokens += record["tokens"].get<int>();
    }

    std::cout << "\nTotal Tokens: " << total_tokens << "\n";

    return 0;
}
