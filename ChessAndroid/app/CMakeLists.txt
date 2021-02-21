cmake_minimum_required(VERSION 3.18.1)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++20 -stdlib=libc++ -fno-exceptions -fno-rtti \
-Wall -Wextra -Wshadow -Wnon-virtual-dtor -Wold-style-cast -Wunused -Woverloaded-virtual -Wpedantic -Wnull-dereference")

set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS} -g -O0")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS} -DNDEBUG -O3 -flto=thin")

add_library(
        chess
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

find_library(
        log-lib
        log)

target_link_libraries(
        chess
        ${log-lib})
