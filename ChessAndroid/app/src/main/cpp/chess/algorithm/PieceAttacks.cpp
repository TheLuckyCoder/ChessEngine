#include "PieceAttacks.h"

#include "../data/Pos.h"

U64 PieceAttacks::m_KnightMoves[64]{};
U64 PieceAttacks::m_KingMoves[64]{};

void PieceAttacks::init() noexcept
{
	initKnightAttacks();
	initKingAttacks();
}

void PieceAttacks::initKnightAttacks() noexcept
{
	const auto addAttack = [&](const byte startSquare, const byte x, const byte y) {
		Pos pos(startSquare / 8u + x, startSquare % 8u + y);

		if (pos.isValid())
			m_KnightMoves[startSquare] |= (1ull << pos.toSquare());
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
}

void PieceAttacks::initKingAttacks() noexcept
{
	const auto addAttack = [&](const byte startSquare, const byte x, const byte y) {
		Pos pos(startSquare / 8u + x, startSquare % 8u + y);

		if (pos.isValid())
			m_KingMoves[startSquare] |= (1ull << pos.toSquare());
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
}

U64 PieceAttacks::getKnightAttacks(const byte square) noexcept
{
	return m_KnightMoves[square];
}

U64 PieceAttacks::getKingAttacks(const byte square) noexcept
{
	return m_KingMoves[square];
}
