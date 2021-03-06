cmake_minimum_required(VERSION 3.10)
set(LIB_NAME "chess")

add_library(
        ${LIB_NAME}
        SHARED

        src/main/cpp/Main.cpp
        src/main/cpp/chess/BoardManager.cpp
        src/main/cpp/chess/Stats.cpp
        src/main/cpp/chess/Uci.cpp
        src/main/cpp/chess/Board.cpp
        src/main/cpp/chess/algorithm/Attacks.cpp
        src/main/cpp/chess/algorithm/Evaluation.cpp
        src/main/cpp/chess/algorithm/MoveGen.cpp
        src/main/cpp/chess/MoveOrdering.cpp
        src/main/cpp/chess/PawnStructureTable.cpp
        src/main/cpp/chess/algorithm/Search.cpp
        src/main/cpp/chess/TranspositionTable.cpp
        src/main/cpp/chess/Zobrist.cpp
        src/main/cpp/chess/persistence/FenParser.cpp
        src/main/cpp/chess/persistence/MovesPersistence.cpp
        src/main/cpp/chess/polyglot/PolyBook.cpp
)

# CMake 3.12 is required for std_20
#target_compile_features(${LIB_NAME} PUBLIC cxx_std_20)

target_compile_options(${LIB_NAME} PUBLIC
        -std=c++20 -stdlib=libc++ -fno-exceptions -fno-rtti
        -Wall -Wextra -Wshadow -Wnon-virtual-dtor -Wold-style-cast -Wunused -Woverloaded-virtual
        -Wpedantic -Wnull-dereference
        )

set(FLAGS_DEBUG "-O2")
set(FLAGS_RELEASE "-DNDEBUG" "-flto=thin" "-O3")
target_compile_options(${LIB_NAME} PUBLIC "$<$<CONFIG:DEBUG>:${FLAGS_DEBUG}>")
target_compile_options(${LIB_NAME} PUBLIC "$<$<CONFIG:RELWITHDEBINFO>:${FLAGS_RELEASE}>")
target_compile_options(${LIB_NAME} PUBLIC "$<$<CONFIG:RELEASE>:${FLAGS_RELEASE}>")

find_library(
        log-lib
        log)

target_link_libraries(
        ${LIB_NAME}
        ${log-lib})
