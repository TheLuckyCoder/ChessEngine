cmake_minimum_required(VERSION 3.6)

set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS} -g -O0 -std=c++17 -Wall -Wextra -pedantic")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS} -O3 -std=c++17 -Wall -Wextra -pedantic -DNDEBUG")

file(GLOB_RECURSE cpp_srcs "src/main/cpp/chess/*.cpp")
add_library(
        chess
        SHARED
        src/main/cpp/Main.cpp
        ${cpp_srcs})

set(ANDROID_TOOLCHAIN_NAME "arm64-v8a")

find_library(
        log-lib
        log)

target_link_libraries(
        chess
        ${log-lib})
