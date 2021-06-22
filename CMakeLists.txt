# TODO This whole CMake file is a big WIP since there should be a way to merge this one and the Android one

cmake_minimum_required(VERSION 3.16)
set(PROJECT_NAME UciChessEngine)
project(${PROJECT_NAME} LANGUAGES CXX)

set(CMAKE_CXX_EXTENSIONS OFF)

find_package(Threads REQUIRED)

add_subdirectory(ChessAndroid/app/src/main/cpp/chess)

add_executable(${PROJECT_NAME}
        ChessLinux/src/Main.cpp

        ${SOURCE_FILES}
        )

target_link_libraries(${PROJECT_NAME} Threads::Threads)

target_compile_features(${PROJECT_NAME} PUBLIC cxx_std_20)

target_compile_options(${PROJECT_NAME} PUBLIC
        -fno-exceptions -fno-rtti
        -Wall -Wextra -Wshadow -Wnon-virtual-dtor -Wold-style-cast -Wcast-align -Wunused -Woverloaded-virtual
        -Wpedantic -Wnull-dereference -Wuninitialized
        )

set(FLAGS_DEBUG "-fsanitize=undefined")
set(FLAGS_RELWITHDEBINFO "-fsanitize=undefined" "-O1" "-UNDEBUG")
set(FLAGS_RELEASE "-DNDEBUG" "-flto" "-O3")
target_compile_options(${PROJECT_NAME} PUBLIC "$<$<CONFIG:DEBUG>:${FLAGS_DEBUG}>")
target_compile_options(${PROJECT_NAME} PUBLIC "$<$<CONFIG:RELWITHDEBINFO>:${FLAGS_RELWITHDEBINFO}>")
target_compile_options(${PROJECT_NAME} PUBLIC "$<$<CONFIG:RELEASE>:${FLAGS_RELEASE}>")

set(FLAGS_DEBUG_LINK "-fsanitize=undefined")
set(FLAGS_RELEASE_LINK "-static")

#target_link_options(${PROJECT_NAME} PUBLIC -fuse-ld=lld -stdlib=libc++ -lc++abi)
target_link_options(${PROJECT_NAME} PUBLIC "$<$<CONFIG:DEBUG>:${FLAGS_DEBUG_LINK}>")
target_link_options(${PROJECT_NAME} PUBLIC "$<$<CONFIG:RELWITHDEBINFO>:${FLAGS_DEBUG_LINK}>")
target_link_options(${PROJECT_NAME} PUBLIC "$<$<CONFIG:RELEASE>:${FLAGS_RELEASE_LINK}>")
