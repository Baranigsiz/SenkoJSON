cmake_minimum_required(VERSION 3.10)
project(CoreJSON VERSION 1.0)

# C++17 standardını zorunlu kılıyoruz
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# include klasörünü başlık (header) dosyaları için yola ekliyoruz
include_directories(include)

# Çalıştırılabilir dosyamızı src/main.cpp üzerinden oluşturuyoruz
add_executable(CoreJSON src/main.cpp)