#include "PieceAttacks.h"

#include "../data/Bitboard.h"
#include "../data/Pos.h"
#include "../data/Rays.h"

constexpr U64 getRayAttacksForwards(const byte square, const U64 occupied, const Rays::Dir direction)
{
	const U64 attacks = Rays::getRay(direction, square);
	const U64 blocker = attacks & occupied;
	const byte index = Bitboard::bitScanForward(blocker | 0x8000000000000000);
	return attacks ^ Rays::getRay(direction, index);
}

constexpr U64 getRayAttacksBackwards(const byte square, const U64 occupied, const Rays::Dir direction)
{
	const U64 attacks = Rays::getRay(direction, square);
	const U64 blocker = attacks & occupied;
	const byte index = Bitboard::bitScanReverse(blocker | 1ULL);
	return attacks ^ Rays::getRay(direction, index);
}

constexpr U64 generateBishopAttacks(const byte square, const U64 blockers) noexcept
{
	U64 attacks{};

	attacks |= getRayAttacksForwards(square, blockers, Rays::NORTH_WEST);
	attacks |= getRayAttacksForwards(square, blockers, Rays::NORTH_EAST);
	attacks |= getRayAttacksBackwards(square, blockers, Rays::SOUTH_EAST);
	attacks |= getRayAttacksBackwards(square, blockers, Rays::SOUTH_WEST);

	return attacks;
}

constexpr U64 generateRookAttacks(const byte square, const U64 blockers) noexcept
{
	U64 attacks{};

	attacks |= getRayAttacksForwards(square, blockers, Rays::NORTH);
	attacks |= getRayAttacksForwards(square, blockers, Rays::EAST);
	attacks |= getRayAttacksBackwards(square, blockers, Rays::SOUTH);
	attacks |= getRayAttacksBackwards(square, blockers, Rays::WEST);

	return attacks;
}
const std::array<U64, 64> PieceAttacks::s_KnightAttacks = [] {
	std::array<U64, 64> moves{};

	const auto addAttack = [&](const byte startSquare, const byte x, const byte y) {
		const Pos pos(row(startSquare) + x, col(startSquare) + y);

		if (pos.isValid())
			moves[startSquare] |= Bitboard::shiftedBoards[pos.toSquare()];
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

const std::array<U64, 64> PieceAttacks::s_KingAttacks = [] {
	std::array<U64, 64> moves{};

	const auto addAttack = [&](const byte startSquare, const byte x, const byte y) {
		const Pos pos(row(startSquare) + x, col(startSquare) + y);

		if (pos.isValid())
			moves[startSquare] |= Bitboard::shiftedBoards[pos.toSquare()];
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

U64 PieceAttacks::getKnightAttacks(const byte square) noexcept
{
	return s_KnightAttacks[square];
}

U64 PieceAttacks::getBishopAttacks(const byte square, U64 blockers) noexcept
{
	return generateBishopAttacks(square, blockers);
}

U64 PieceAttacks::getRookAttacks(const byte square, U64 blockers) noexcept
{
	return generateRookAttacks(square, blockers);
}

U64 PieceAttacks::getKingAttacks(const byte square) noexcept
{
	return s_KingAttacks[square];
}
