set(ROOT ${CMAKE_CURRENT_SOURCE_DIR})

set(SOURCE_FILES
        ${SOURCE_FILES}
        ${ROOT}/BoardManager.cpp
        ${ROOT}/Stats.cpp
        ${ROOT}/Uci.cpp
        ${ROOT}/Board.cpp
        ${ROOT}/algorithm/Attacks.cpp
        ${ROOT}/algorithm/Evaluation.cpp
        ${ROOT}/MoveGen.cpp
        ${ROOT}/MoveOrdering.cpp
        ${ROOT}/PawnStructureTable.cpp
        ${ROOT}/algorithm/Search.cpp
        ${ROOT}/Tests.cpp
        ${ROOT}/TranspositionTable.cpp
        ${ROOT}/Zobrist.cpp
        ${ROOT}/persistence/FenParser.cpp
        ${ROOT}/polyglot/PolyBook.cpp
        PARENT_SCOPE
)
