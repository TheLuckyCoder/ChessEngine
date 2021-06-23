# TODO This whole CMake file is a big WIP since there should be a way to merge this one and the Android one

cmake_minimum_required(VERSION 3.18)
set(PROJECT_NAME chess)
project(${PROJECT_NAME} LANGUAGES CXX)

find_package(Threads REQUIRED)

add_subdirectory(ChessAndroid/app/src/main/cpp/chess)

add_executable(${PROJECT_NAME}
        ChessLinux/src/Main.cpp
        ${SOURCE_FILES}
        )

target_link_libraries(${PROJECT_NAME} Threads::Threads)

target_compile_features(${PROJECT_NAME} PUBLIC ${COMPILE_FEATURES})

target_compile_options(${PROJECT_NAME} PUBLIC
        ${FLAGS_COMPILE}
        )

target_compile_options(${PROJECT_NAME} PUBLIC "$<$<CONFIG:DEBUG>:${FLAGS_DEBUG_COMPILE}>")
target_compile_options(${PROJECT_NAME} PUBLIC "$<$<CONFIG:RELWITHDEBINFO>:${FLAGS_RELWITHDEBINFO_COMPILE}>")
target_compile_options(${PROJECT_NAME} PUBLIC "$<$<CONFIG:RELEASE>:${FLAGS_RELEASE_COMPILE}>")

#target_link_options(${PROJECT_NAME} PUBLIC -fuse-ld=lld -stdlib=libc++ -lc++abi)
target_link_options(${PROJECT_NAME} PUBLIC "$<$<CONFIG:DEBUG>:${FLAGS_DEBUG_LINK}>")
target_link_options(${PROJECT_NAME} PUBLIC "$<$<CONFIG:RELWITHDEBINFO>:${FLAGS_DEBUG_LINK}>")
target_link_options(${PROJECT_NAME} PUBLIC "$<$<CONFIG:RELEASE>:${FLAGS_RELEASE_LINK}>")
