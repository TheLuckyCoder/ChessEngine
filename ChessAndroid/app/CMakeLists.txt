cmake_minimum_required(VERSION 3.10)
set(LIB_NAME "chess")
project(${LIB_NAME})

set(CMAKE_CXX_EXTENSIONS OFF)

add_subdirectory(src/main/cpp/chess)

add_library(
        ${LIB_NAME}
        SHARED

        src/main/cpp/Main.cpp
        ${SOURCE_FILES}
)

# CMake 3.12 is required for std_20
#target_compile_features(${LIB_NAME} PUBLIC cxx_std_20)

target_compile_options(${LIB_NAME} PUBLIC
        -std=c++20 -stdlib=libc++ -fno-exceptions -fno-rtti
        -Wall -Wextra -Wshadow -Wnon-virtual-dtor -Wold-style-cast -Wunused -Woverloaded-virtual
        -Wpedantic -Wnull-dereference
        )

set(FLAGS_RELEASE "-DNDEBUG" "-O3" "-flto")
target_compile_options(${LIB_NAME} PUBLIC "$<$<CONFIG:RELWITHDEBINFO>:${FLAGS_RELEASE}>")
target_compile_options(${LIB_NAME} PUBLIC "$<$<CONFIG:RELEASE>:${FLAGS_RELEASE}>")

find_library(
        log-lib
        log)

target_link_libraries(
        ${LIB_NAME}
        ${log-lib})
