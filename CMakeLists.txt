cmake_minimum_required(VERSION 3.14)
project(CoreJSON VERSION 2.0.0 LANGUAGES CXX)

# ==============================================================================
# C++ Standard & Compiler Flags
# ==============================================================================
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Option to build tests and examples
option(COREJSON_BUILD_TESTS "Build CoreJSON unit tests" ON)
option(COREJSON_BUILD_BENCHMARKS "Build CoreJSON benchmarks" ON)
option(COREJSON_BUILD_EXAMPLES "Build CoreJSON examples" ON)

# ==============================================================================
# Core Library Target (Header-Only Interface)
# ==============================================================================
add_library(corejson INTERFACE)
add_library(corejson::corejson ALIAS corejson)

target_include_directories(corejson INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

target_compile_features(corejson INTERFACE cxx_std_17)

# ==============================================================================
# Main Application
# ==============================================================================
add_executable(corejson_app src/main.cpp)
target_link_libraries(corejson_app PRIVATE corejson::corejson)

# ==============================================================================
# Examples
# ==============================================================================
if(COREJSON_BUILD_EXAMPLES)
    add_executable(example_01_basic_usage examples/01_basic_usage.cpp)
    target_link_libraries(example_01_basic_usage PRIVATE corejson::corejson)

    add_executable(example_02_struct_serialization examples/02_struct_serialization.cpp)
    target_link_libraries(example_02_struct_serialization PRIVATE corejson::corejson)

    add_executable(example_03_json_pointer examples/03_json_pointer.cpp)
    target_link_libraries(example_03_json_pointer PRIVATE corejson::corejson)
endif()

# ==============================================================================
# Tests
# ==============================================================================
if(COREJSON_BUILD_TESTS)
    enable_testing()

    add_executable(corejson_tests
        tests/test_main.cpp
        tests/test_primitives.cpp
        tests/test_objects_arrays.cpp
        tests/test_unicode.cpp
        tests/test_errors.cpp
        tests/test_json_pointer.cpp
        tests/test_struct_binding.cpp
    )
    target_link_libraries(corejson_tests PRIVATE corejson::corejson)
    target_include_directories(corejson_tests PRIVATE tests)

    add_test(NAME CoreJSON_UnitTests COMMAND corejson_tests)
endif()

# ==============================================================================
# Benchmarks
# ==============================================================================
if(COREJSON_BUILD_BENCHMARKS)
    add_executable(corejson_benchmarks benchmarks/benchmark_suite.cpp)
    target_link_libraries(corejson_benchmarks PRIVATE corejson::corejson)
endif()

# ==============================================================================
# Installation & CMake Packaging
# ==============================================================================
include(GNUInstallDirs)
include(CMakePackageConfigHelpers)

install(TARGETS corejson
    EXPORT CoreJSONTargets
    INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

install(DIRECTORY include/corejson DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
install(DIRECTORY single_include/corejson DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/single_include OPTIONAL)

install(EXPORT CoreJSONTargets
    FILE CoreJSONTargets.cmake
    NAMESPACE corejson::
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/CoreJSON
)

configure_package_config_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/cmake/CoreJSONConfig.cmake.in
    ${CMAKE_CURRENT_BINARY_DIR}/CoreJSONConfig.cmake
    INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/CoreJSON
)

write_basic_package_version_file(
    ${CMAKE_CURRENT_BINARY_DIR}/CoreJSONConfigVersion.cmake
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY SameMajorVersion
)

install(FILES
    ${CMAKE_CURRENT_BINARY_DIR}/CoreJSONConfig.cmake
    ${CMAKE_CURRENT_BINARY_DIR}/CoreJSONConfigVersion.cmake
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/CoreJSON
)