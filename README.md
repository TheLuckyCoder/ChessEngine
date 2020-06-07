# ChessEngine

An UCI-compatible Chess Engine written in C++17 with an Android UI written in Kotlin.

This project uses the evaluation function values (in the file Evaluation.cpp) from the Stockfish Chess Engine, licensed under GNU GPL 3.0.

## Features

- Search:
  - AlphaBeta Pruning
  - Aspiration Window
  - Iterative Deepening
  - Futility Pruning
  - Reverse Futility Pruning
  - Null Move Pruning
  - Late Move Reductions
  - Quiescence Search
- Move Ordering:
  - Hash Move
  - MVV/LVA
  - Killer heuristic
  - History heuristic
- Evaluation:
  - Evaluation Tapering
  - Piece Square Tables
  - Threats/Attacks
  - Pawn Structure (Connected/Isolated/Double/Passed)
  - Piece Mobility
  - Long Diagonal Bishop
  - Bishop Pairs
  - Rooks on Open Files
  - Rooks on Queen Files
  - Pawn Shield
- Other:
  - Zobrist Hashing
  - Transposition Table
  - Pawn Structure Table

## License

Copyright (C) 2019-2020 Filea Razvan Gheorghe

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
