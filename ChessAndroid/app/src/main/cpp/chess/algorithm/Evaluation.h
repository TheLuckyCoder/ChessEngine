#pragma once

#include <array>
#include <string>

#include "../data/Board.h"
#include "../data/Score.h"
#include "../containers/PawnStructureTable.h"

class Evaluation final
{
	static constexpr short _PIECE_VALUE[] = { 0, 128, 781, 825, 1276, 2538, 0 };

public:
	static constexpr short TEMPO_BONUS = 20;

	struct Trace
	{
		// Pieces
		std::array<Score, 2> pawnValue{};
		std::array<Score, 2> knightValue{};
		std::array<Score, 2> bishopValue{};
		std::array<Score, 2> rookValue{};
		std::array<Score, 2> queenValue{};
		std::array<Score, 2> kingValue{};

		std::array<Score, 2> kingProtector{};
		std::array<Score, 2> minorPawnShield{};

		// Threats
		std::array<Score, 2> threatsByMinor{};
		std::array<Score, 2> threatsByRook{};
		std::array<Score, 2> threatsByKing{};
		std::array<Score, 2> threatBySafePawn{};
		std::array<Score, 2> piecesHanging{};
		std::array<Score, 2> weakQueenProtection{};
		std::array<Score, 2> queenThreatByKnight{};
		std::array<Score, 2> queenThreatBySlider{};
		std::array<Score, 2> restrictedMovement{};
		std::array<Score, 2> attacksTotal{};

		std::array<Score, 2> total{};
	};

	static int value(const Board &board) noexcept;
	static int invertedValue(const Board &board) noexcept;
	static std::string traceValue(const Board &board);

	static constexpr short getPieceValue(const PieceType type) noexcept
	{
		return _PIECE_VALUE[type];
	}
};
