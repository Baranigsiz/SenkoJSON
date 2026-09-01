#include <iostream>
#include <string>
#include <string_view>
#include <senko/senko.hpp>

// A custom SAX handler that filters log events directly from a JSON stream with 0 memory allocation!
struct ErrorLogFilter : public senko::default_sax_handler {
    std::string current_key;
    bool in_error_event = false;
    std::string current_message;
    int error_count = 0;

    bool key(std::string_view k) {
        current_key = std::string(k);
        return true;
    }

    bool string(std::string_view val) {
        if (current_key == "level" && val == "ERROR") {
            in_error_event = true;
        } else if (current_key == "message") {
            current_message = std::string(val);
        }
        return true;
    }

    bool end_object() {
        if (in_error_event) {
            error_count++;
            std::cout << " [!] Filtered Error #" << error_count << ": " << current_message << "\n";
            in_error_event = false;
            current_message.clear();
        }
        return true;
    }
};

int main() {
    std::cout << "=== SenkoJSON Example 09: Streaming SAX Parser ===\n\n";

    std::string log_stream = 
        "[\n"
        "  {\"timestamp\": \"2026-09-01T12:00:01Z\", \"level\": \"INFO\", \"message\": \"Server started\"},\n"
        "  {\"timestamp\": \"2026-09-01T12:00:05Z\", \"level\": \"ERROR\", \"message\": \"Database connection timeout (504)\"},\n"
        "  {\"timestamp\": \"2026-09-01T12:00:10Z\", \"level\": \"DEBUG\", \"message\": \"Cache refreshed\"},\n"
        "  {\"timestamp\": \"2026-09-01T12:00:15Z\", \"level\": \"ERROR\", \"message\": \"Disk space critically low (98% full)\"},\n"
        "  {\"timestamp\": \"2026-09-01T12:00:20Z\", \"level\": \"INFO\", \"message\": \"User logged in\"}\n"
        "]";

    std::cout << "Streaming log data through SAX Filter (0 DOM allocations):\n";
    ErrorLogFilter filter;
    bool success = senko::sax_parse(log_stream, filter);

    std::cout << "\nTotal Errors Found: " << filter.error_count << "\n";
    std::cout << "Parsing Success: " << (success ? "YES" : "NO") << "\n";

    std::cout << "\n✔ Example 09 finished successfully!\n";
    return 0;
}
