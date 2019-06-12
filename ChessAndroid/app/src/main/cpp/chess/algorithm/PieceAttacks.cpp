#include "PieceAttacks.h"

#include "../data/Pos.h"

const std::array<U64, 64> PieceAttacks::s_KnightMoves = [] {
	std::array<U64, 64> moves{};

	const auto addAttack = [&](const byte startSquare, const byte x, const byte y) {
		Pos pos(startSquare / 8u + x, startSquare % 8u + y);

		if (pos.isValid())
			moves[startSquare] |= (1ull << pos.toSquare());
	};

	for (byte i = 0u; i < 64u; i++)
	{
		addAttack(i, -2, -1);
		addAttack(i, -2, 1u);

		addAttack(i, -1, -2);
		addAttack(i, -1, 2u);

		addAttack(i, 1u, -2);
		addAttack(i, 1u, 2u);

		addAttack(i, 2u, -1);
		addAttack(i, 2u, 1u);
	}

	return moves;
}();

const std::array<U64, 64> PieceAttacks::s_KingMoves = [] {
	std::array<U64, 64> moves{};

	const auto addAttack = [&](const byte startSquare, const byte x, const byte y) {
		Pos pos(startSquare / 8u + x, startSquare % 8u + y);

		if (pos.isValid())
			moves[startSquare] |= (1ull << pos.toSquare());
	};

	for (byte i = 0u; i < 64u; i++)
	{
		// Vertical and Horizontal
		addAttack(i, -1, 0u);
		addAttack(i, 1u, 0u);
		addAttack(i, 0u, -1);
		addAttack(i, 0u, 1u);

		// Diagonals
		addAttack(i, 1u, -1);
		addAttack(i, 1u, 1u);
		addAttack(i, -1, -1);
		addAttack(i, -1, 1u);
	}

	return moves;
}();

void PieceAttacks::init() noexcept
{
	static bool initialized = false;
	if (initialized) return;
	initialized = true;

	// TODO: Implement Slider Pieces
}

U64 PieceAttacks::getKnightAttacks(const byte square) noexcept
{
	return s_KnightMoves[square];
}

U64 PieceAttacks::getKingAttacks(const byte square) noexcept
{
	return s_KingMoves[square];
}
