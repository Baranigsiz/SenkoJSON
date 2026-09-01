#pragma once

#include <iostream>
#include <string>
#include <vector>
#include <functional>
#include <sstream>

namespace test_framework {

struct TestCase {
    std::string name;
    std::function<void()> func;
};

inline std::vector<TestCase>& get_registry() {
    static std::vector<TestCase> registry;
    return registry;
}

struct AutoRegister {
    AutoRegister(std::string name, std::function<void()> func) {
        get_registry().push_back({std::move(name), std::move(func)});
    }
};

inline int total_checks = 0;
inline int passed_checks = 0;
inline int failed_checks = 0;

inline void log_failure(const char* expr, const char* file, int line, const std::string& extra = "") {
    failed_checks++;
    std::cout << "\033[31m[FAILED]\033[0m " << file << ":" << line << " - Check failed: " << expr;
    if (!extra.empty()) {
        std::cout << " (" << extra << ")";
    }
    std::cout << "\n" << std::flush;
}

inline void log_success() {
    passed_checks++;
}

#define TEST_CONCAT_IMPL(x, y) x##y
#define TEST_CONCAT(x, y) TEST_CONCAT_IMPL(x, y)
#define TEST_UNIQUE_NAME(prefix) TEST_CONCAT(prefix, __LINE__)

#define TEST_CASE(name) \
    static void TEST_UNIQUE_NAME(test_func_)(); \
    namespace { \
        static ::test_framework::AutoRegister TEST_UNIQUE_NAME(test_reg_)(name, &TEST_UNIQUE_NAME(test_func_)); \
    } \
    static void TEST_UNIQUE_NAME(test_func_)()

#define CHECK(expr) \
    do { \
        ::test_framework::total_checks++; \
        if (!(expr)) { \
            ::test_framework::log_failure(#expr, __FILE__, __LINE__); \
        } else { \
            ::test_framework::log_success(); \
        } \
    } while (0)

#define CHECK_EQ(a, b) \
    do { \
        ::test_framework::total_checks++; \
        auto _val_a = (a); \
        auto _val_b = (b); \
        if (!(_val_a == _val_b)) { \
            std::ostringstream ss; \
            ss << "Left: " << _val_a << ", Right: " << _val_b; \
            ::test_framework::log_failure(#a " == " #b, __FILE__, __LINE__, ss.str()); \
        } else { \
            ::test_framework::log_success(); \
        } \
    } while (0)

#define CHECK_THROWS(expr) \
    do { \
        ::test_framework::total_checks++; \
        bool caught = false; \
        try { \
            (void)(expr); \
        } catch (...) { \
            caught = true; \
        } \
        if (!caught) { \
            ::test_framework::log_failure("Expected exception from " #expr, __FILE__, __LINE__); \
        } else { \
            ::test_framework::log_success(); \
        } \
    } while (0)

#define CHECK_NOTHROW(expr) \
    do { \
        ::test_framework::total_checks++; \
        bool threw = false; \
        try { \
            (void)(expr); \
        } catch (const std::exception& e) { \
            threw = true; \
            ::test_framework::log_failure("Unexpected exception from " #expr, __FILE__, __LINE__, e.what()); \
        } catch (...) { \
            threw = true; \
            ::test_framework::log_failure("Unexpected unknown exception from " #expr, __FILE__, __LINE__); \
        } \
        if (!threw) { \
            ::test_framework::log_success(); \
        } \
    } while (0)

inline int run_all_tests() {
    std::cout << std::unitbuf;
    auto& tests = get_registry();
    std::cout << "\033[36m====================================================\033[0m\n";
    std::cout << "\033[36m   SenkoJSON Test Suite Running " << tests.size() << " Test Cases   \033[0m\n";
    std::cout << "\033[36m====================================================\033[0m\n\n" << std::flush;

    int test_failures = 0;

    for (const auto& t : tests) {
        int initial_failed = failed_checks;
        std::cout << "▶ Running: " << t.name << "... " << std::flush;
        try {
            t.func();
            if (failed_checks == initial_failed) {
                std::cout << "\033[32m[PASSED]\033[0m\n" << std::flush;
            } else {
                std::cout << "\033[31m[FAILED]\033[0m\n" << std::flush;
                test_failures++;
            }
        } catch (const std::exception& e) {
            std::cout << "\033[31m[EXCEPTION: " << e.what() << "]\033[0m\n" << std::flush;
            failed_checks++;
            test_failures++;
        } catch (...) {
            std::cout << "\033[31m[UNKNOWN EXCEPTION]\033[0m\n" << std::flush;
            failed_checks++;
            test_failures++;
        }
    }

    std::cout << "\n\033[36m----------------------------------------------------\033[0m\n";
    std::cout << "Results: "
              << passed_checks << " passed checks, "
              << failed_checks << " failed checks across "
              << tests.size() << " test cases.\n";

    if (test_failures == 0 && failed_checks == 0) {
        std::cout << "\033[32m✔ ALL TESTS PASSED SUCCESSFULLY!\033[0m\n" << std::flush;
        return 0;
    } else {
        std::cout << "\033[31m✖ SOME TESTS FAILED!\033[0m\n" << std::flush;
        return 1;
    }
}

} // namespace test_framework
