cmake_minimum_required(VERSION 3.6)

set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS} -fno-exceptions -fno-rtti -g -O0 -std=c++17 -Wall -Wextra -pedantic")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS} -fno-exceptions -fno-rtti -O3 -std=c++17 -Wall -Wextra -pedantic -DNDEBUG")

file(GLOB_RECURSE cpp_srcs "src/main/cpp/chess/*.cpp")
add_library(
        chess
        SHARED
        src/main/cpp/Main.cpp
        ${cpp_srcs})

find_library(
        log-lib
        log)

target_link_libraries(
        chess
        ${log-lib})
