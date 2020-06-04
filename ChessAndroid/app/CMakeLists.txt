cmake_minimum_required(VERSION 3.6)

set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS} -fno-exceptions -fno-rtti -g -O0 -std=c++17 -Wall -Wextra -pedantic")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS} -fno-exceptions -fno-rtti -O3 -std=c++17 -Wall -Wextra -pedantic -DNDEBUG")

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
        src/main/cpp/chess/algorithm/Hash.cpp
        src/main/cpp/chess/algorithm/MoveGen.cpp
        src/main/cpp/chess/algorithm/MoveOrdering.cpp
        src/main/cpp/chess/algorithm/Search.cpp
        src/main/cpp/chess/containers/PawnStructureTable.cpp
        src/main/cpp/chess/containers/TranspositionTable.cpp
        src/main/cpp/chess/persistence/FenParser.cpp
        src/main/cpp/chess/persistence/MovesPersistence.cpp)

find_library(
        log-lib
        log)

target_link_libraries(
        chess
        ${log-lib})
