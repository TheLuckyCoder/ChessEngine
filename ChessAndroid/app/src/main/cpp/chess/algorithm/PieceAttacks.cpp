#include "PieceAttacks.h"

#include <array>

#include "../data/Pos.h"

constexpr static std::array<U64, 64> knightMoves = [] {
	std::array<U64, 64> moves{};

	const auto addAttack = [&](const byte startSquare, const byte x, const byte y) {
		Pos pos(startSquare / 8u + x, startSquare % 8u + y);

		if (pos.isValid())
			moves[startSquare] |= (1ull << pos.toSquare());
	};

	for (byte i = 0u; i < 64u; i++)
	{
		addAttack(i, -2u, -1u);
		addAttack(i, -2u, 1u);

		addAttack(i, -1u, -2u);
		addAttack(i, -1u, 2u);

		addAttack(i, 1u, -2u);
		addAttack(i, 1u, 2u);

		addAttack(i, 2u, -1u);
		addAttack(i, 2u, 1u);
	}

	return moves;
}();

constexpr std::array<U64, 64> kingMoves = [] {
	std::array<U64, 64> moves{};

	const auto addAttack = [&](const byte startSquare, const byte x, const byte y) {
		Pos pos(startSquare / 8u + x, startSquare % 8u + y);

		if (pos.isValid())
			moves[startSquare] |= (1ull << pos.toSquare());
	};

	for (byte i = 0u; i < 64u; i++)
	{
		// Vertical and Horizontal
		addAttack(i, -1u, 0u);
		addAttack(i, 1u, 0u);
		addAttack(i, 0u, -1u);
		addAttack(i, 0u, 1u);

		// Diagonals
		addAttack(i, 1u, -1u);
		addAttack(i, 1u, 1u);
		addAttack(i, -1u, -1u);
		addAttack(i, -1u, 1u);
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
	return knightMoves[square];
}

U64 PieceAttacks::getKingAttacks(const byte square) noexcept
{
	return kingMoves[square];
}
