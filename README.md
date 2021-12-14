# ChessEngine

[![Actions Status](https://github.com/TheLuckyCoder/ChessEngine/workflows/Android%20CI/badge.svg)](https://github.com/TheLuckyCoder/ChessEngine/actions)
[![Actions Status](https://github.com/TheLuckyCoder/ChessEngine/workflows/C++%20CI/badge.svg)](https://github.com/TheLuckyCoder/ChessEngine/actions)

An UCI-compatible Chess Engine written in C++20 with an Android and WearOS UI written in Jetpack Compose.

This project uses the evaluation values (in the file Evaluation.cpp) from the Stockfish Chess Engine, licensed under GNU GPL 3.0.

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
  - Lazy SMP
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

## Building

### Android and WearOS

To build the for project for Android you will need at least:
 - Android Studio Arctic Fox
 - Android SDK Version 31 (Android 12)
 - NDK 23.1.7779620

### Linux

Please note that the Linux version of the engine has no UI but can be used with any UCI-compatible chess interfaces.<br>
To build the for project for Linux you will need at least:
 - CMake 3.18
 - C++20 Compiler:
    * GCC 10
    * Clang 13

## Compatible Interfaces

- [Arena](http://www.playwitharena.com)
- [Cutechess](https://github.com/cutechess/cutechess)

## License

Copyright (C) 2019-2021 Filea Răzvan Gheorghe

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
