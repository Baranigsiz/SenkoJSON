cmake_minimum_required(VERSION 3.10)
project(CoreJSON VERSION 1.0)

# C++17 standardını zorunlu kılıyoruz
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# include klasörünü başlık (header) dosyaları için yola ekliyoruz
include_directories(include)

# Source files for the library
set(LIB_SOURCES 
    src/Lexer.cpp 
    src/Parser.cpp 
    src/JsonNode.cpp
)

# Core library
add_library(corejson_lib ${LIB_SOURCES})
target_include_directories(corejson_lib PUBLIC include)

# Example application
add_executable(corejson_app src/main.cpp)
target_link_libraries(corejson_app corejson_lib)