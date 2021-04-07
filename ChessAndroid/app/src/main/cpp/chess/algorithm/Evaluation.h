#pragma once

#include <array>
#include <string>

#include "../Board.h"
#include "../Score.h"

class Evaluation final
{
	static constexpr short PieceValue[] = { 0, 128, 781, 825, 1276, 2538, 0 };

public:
    static constexpr short TEMPO_BONUS = 20;

    struct Trace
    {
        // Pieces
        std::array<Score, COLOR_NB> pawns{};
        std::array<Score, COLOR_NB> knights{};
        std::array<Score, COLOR_NB> bishops{};
        std::array<Score, COLOR_NB> rooks{};
        std::array<Score, COLOR_NB> queen{};
        std::array<Score, COLOR_NB> king{};
        std::array<Score, COLOR_NB> mobility{};
        std::array<Score, COLOR_NB> piecesTotal{};

        std::array<Score, COLOR_NB> kingProtector{};
        std::array<Score, COLOR_NB> minorPawnShield{};

        // Threats
        std::array<Score, COLOR_NB> threatsByMinor{};
        std::array<Score, COLOR_NB> threatsByRook{};
        std::array<Score, COLOR_NB> threatsByKing{};
        std::array<Score, COLOR_NB> threatBySafePawn{};
        std::array<Score, COLOR_NB> piecesHanging{};
        std::array<Score, COLOR_NB> weakQueenProtection{};
        std::array<Score, COLOR_NB> queenThreatByKnight{};
        std::array<Score, COLOR_NB> queenThreatBySlider{};
        std::array<Score, COLOR_NB> restrictedMovement{};
        std::array<Score, COLOR_NB> attacksTotal{};

        std::array<Score, COLOR_NB> kingSafety{};

        std::array<Score, COLOR_NB> total{};
    };

    static int value(const Board &board) noexcept;
    static int invertedValue(const Board &board) noexcept;
    static std::string traceValue(const Board &board);

    static constexpr short getPieceValue(const PieceType type) noexcept
    {
        return PieceValue[type];
    }
};
