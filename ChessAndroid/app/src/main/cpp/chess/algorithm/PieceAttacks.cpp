#include "PieceAttacks.h"

#include "../data/Pos.h"
#include "../data/Rays.h"

constexpr std::array<byte, 64> rookIndexBits = {
	12, 11, 11, 11, 11, 11, 11, 12,
	11, 10, 10, 10, 10, 10, 10, 11,
	11, 10, 10, 10, 10, 10, 10, 11,
	11, 10, 10, 10, 10, 10, 10, 11,
	11, 10, 10, 10, 10, 10, 10, 11,
	11, 10, 10, 10, 10, 10, 10, 11,
	11, 10, 10, 10, 10, 10, 10, 11,
	12, 11, 11, 11, 11, 11, 11, 12
};

constexpr std::array<byte, 64> bishopIndexBits = {
	6, 5, 5, 5, 5, 5, 5, 6,
	5, 5, 5, 5, 5, 5, 5, 5,
	5, 5, 7, 7, 7, 7, 5, 5,
	5, 5, 7, 9, 9, 7, 5, 5,
	5, 5, 7, 9, 9, 7, 5, 5,
	5, 5, 7, 7, 7, 7, 5, 5,
	5, 5, 5, 5, 5, 5, 5, 5,
	6, 5, 5, 5, 5, 5, 5, 6
};

/**
 * Rook and bishop magic values for magic table lookups
 */
constexpr std::array<U64, 64> bishopMagics = {
	0x89a1121896040240ULL, 0x2004844802002010ULL, 0x2068080051921000ULL, 0x62880a0220200808ULL, 0x4042004000000ULL,
	0x100822020200011ULL, 0xc00444222012000aULL, 0x28808801216001ULL, 0x400492088408100ULL, 0x201c401040c0084ULL,
	0x840800910a0010ULL, 0x82080240060ULL, 0x2000840504006000ULL, 0x30010c4108405004ULL, 0x1008005410080802ULL,
	0x8144042209100900ULL, 0x208081020014400ULL, 0x4800201208ca00ULL, 0xf18140408012008ULL, 0x1004002802102001ULL,
	0x841000820080811ULL, 0x40200200a42008ULL, 0x800054042000ULL, 0x88010400410c9000ULL, 0x520040470104290ULL,
	0x1004040051500081ULL, 0x2002081833080021ULL, 0x400c00c010142ULL, 0x941408200c002000ULL, 0x658810000806011ULL,
	0x188071040440a00ULL, 0x4800404002011c00ULL, 0x104442040404200ULL, 0x511080202091021ULL, 0x4022401120400ULL,
	0x80c0040400080120ULL, 0x8040010040820802ULL, 0x480810700020090ULL, 0x102008e00040242ULL, 0x809005202050100ULL,
	0x8002024220104080ULL, 0x431008804142000ULL, 0x19001802081400ULL, 0x200014208040080ULL, 0x3308082008200100ULL,
	0x41010500040c020ULL, 0x4012020c04210308ULL, 0x208220a202004080ULL, 0x111040120082000ULL, 0x6803040141280a00ULL,
	0x2101004202410000ULL, 0x8200000041108022ULL, 0x21082088000ULL, 0x2410204010040ULL, 0x40100400809000ULL,
	0x822088220820214ULL, 0x40808090012004ULL, 0x910224040218c9ULL, 0x402814422015008ULL, 0x90014004842410ULL,
	0x1000042304105ULL, 0x10008830412a00ULL, 0x2520081090008908ULL, 0x40102000a0a60140ULL,
};
constexpr std::array<U64, 64> rookMagics = {
	0xa8002c000108020ULL, 0x6c00049b0002001ULL, 0x100200010090040ULL, 0x2480041000800801ULL, 0x280028004000800ULL,
	0x900410008040022ULL, 0x280020001001080ULL, 0x2880002041000080ULL, 0xa000800080400034ULL, 0x4808020004000ULL,
	0x2290802004801000ULL, 0x411000d00100020ULL, 0x402800800040080ULL, 0xb000401004208ULL, 0x2409000100040200ULL,
	0x1002100004082ULL, 0x22878001e24000ULL, 0x1090810021004010ULL, 0x801030040200012ULL, 0x500808008001000ULL,
	0xa08018014000880ULL, 0x8000808004000200ULL, 0x201008080010200ULL, 0x801020000441091ULL, 0x800080204005ULL,
	0x1040200040100048ULL, 0x120200402082ULL, 0xd14880480100080ULL, 0x12040280080080ULL, 0x100040080020080ULL,
	0x9020010080800200ULL, 0x813241200148449ULL, 0x491604001800080ULL, 0x100401000402001ULL, 0x4820010021001040ULL,
	0x400402202000812ULL, 0x209009005000802ULL, 0x810800601800400ULL, 0x4301083214000150ULL, 0x204026458e001401ULL,
	0x40204000808000ULL, 0x8001008040010020ULL, 0x8410820820420010ULL, 0x1003001000090020ULL, 0x804040008008080ULL,
	0x12000810020004ULL, 0x1000100200040208ULL, 0x430000a044020001ULL, 0x280009023410300ULL, 0xe0100040002240ULL,
	0x200100401700ULL, 0x2244100408008080ULL, 0x8000400801980ULL, 0x2000810040200ULL, 0x8010100228810400ULL,
	0x2000009044210200ULL, 0x4080008040102101ULL, 0x40002080411d01ULL, 0x2005524060000901ULL, 0x502001008400422ULL,
	0x489a000810200402ULL, 0x1004400080a13ULL, 0x4000011008020084ULL, 0x26002114058042ULL
};

/**
 * Rook and bishop sliding attack masks indexed by [square]
 * These do not include edge squares
 */
constexpr auto bishopMasks = [] {
	std::array<U64, 64> masks{};

	for (byte square = 0u; square < 64u; ++square)
	{
		masks[square] = Rays::getRay(Rays::NORTH_EAST, square)
						| Rays::getRay(Rays::NORTH_WEST, square)
						| Rays::getRay(Rays::SOUTH_EAST, square)
						| Rays::getRay(Rays::SOUTH_WEST, square);
	}

	return masks;
}();

constexpr auto rookMasks = [] {
	std::array<U64, 64> masks{};

	for (byte square = 0u; square < 64u; ++square)
	{
		masks[square] = (Rays::getRay(Rays::NORTH, square) & ~RANK_8)
						| (Rays::getRay(Rays::SOUTH, square) & ~RANK_1)
						| (Rays::getRay(Rays::EAST, square) & ~FILE_H)
						| (Rays::getRay(Rays::WEST, square) & ~FILE_A);
	}

	return masks;
}();

constexpr U64 getBlockersFromIndex(const int index, U64 mask)
{
	U64 blockers{};
	const int bits = Bitboard::popCount(mask);

	for (int i = 0; i < bits; i++)
	{
		const byte bitPos = Bitboard::popLsb(mask);
		if (index & (1 << i))
			blockers |= Bitboard::shiftedBoards[bitPos];
	}

	return blockers;
}

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

const std::array<std::array<U64, 64>, 2> PieceAttacks::s_PawnAttacks = [] {
	std::array<std::array<U64, 64>, 2> moves{};

	for (byte i = 0u; i < 64u; i++)
	{
		const Pos start(i);

		U64 whiteAttackBb{};
		{
			const Pos leftPos = start + Pos(-1, 1u);
			const Pos rightPos = start + Pos(1u, 1u);

			if (leftPos.isValid())
				whiteAttackBb |= leftPos.toBitboard();
			if (rightPos.isValid())
				whiteAttackBb |= rightPos.toBitboard();
		}
		
		U64 blackAttackBb{};
		{
			const Pos leftPos = start + Pos(-1, -1);
			const Pos rightPos = start + Pos(1u, -1);

			if (leftPos.isValid())
				blackAttackBb |= leftPos.toBitboard();
			if (rightPos.isValid())
				blackAttackBb |= rightPos.toBitboard();
		}

	    moves[true][i] = whiteAttackBb;
	    moves[false][i] = blackAttackBb;
	}

	return moves;
}();

const std::array<U64, 64> PieceAttacks::s_KnightAttacks = [] {
	std::array<U64, 64> moves{};

	const auto addAttack = [&](const byte startSquare, const byte x, const byte y) {
		const Pos pos(col(startSquare) + x, row(startSquare) + y);

		if (pos.isValid())
			moves[startSquare] |= pos.toBitboard();
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

std::array<std::array<U64, 1024>, 64> PieceAttacks::s_BishopAttacks{};

std::array<std::array<U64, 4096>, 64> PieceAttacks::s_RookAttacks{};

const std::array<U64, 64> PieceAttacks::s_KingAttacks = [] {
	std::array<U64, 64> moves{};

	const auto addAttack = [&](const byte startSquare, const byte x, const byte y) {
		const Pos pos(col(startSquare) + x, row(startSquare) + y);

		if (pos.isValid())
			moves[startSquare] |= pos.toBitboard();
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

	// Init Bishop Moves
	for (byte square = 0u; square < 64u; square++)
	{
		const int indexBit = bishopIndexBits[square];
		
		// For all possible blockers for this square
		for (int blockerIndex = 0; blockerIndex < (1 << indexBit); ++blockerIndex)
		{
			const U64 blockers = getBlockersFromIndex(blockerIndex, bishopMasks[square]);
			const U64 attacks = generateBishopAttacks(square, blockers);
			const U64 index = (blockers * bishopMagics[square]) >> (64 - indexBit);

			s_BishopAttacks[square][index] = attacks;
		}
	}

	// Init Rook Moves
	for (byte square = 0u; square < 64u; square++)
	{
		const int indexBit = rookIndexBits[square];

		// For all possible blockers for this square
		for (int blockerIndex = 0; blockerIndex < (1 << indexBit); ++blockerIndex)
		{
			const U64 blockers = getBlockersFromIndex(blockerIndex, rookMasks[square]);
			const U64 attacks = generateRookAttacks(square, blockers);
			const U64 index = (blockers * rookMagics[square]) >> (64 - indexBit);

			s_RookAttacks[square][index] = attacks;
		}
	}
}

U64 PieceAttacks::getPawnAttacks(const bool isWhite, const byte square) noexcept
{
	return s_PawnAttacks[isWhite][square];
}

U64 PieceAttacks::getKnightAttacks(const byte square) noexcept
{
	return s_KnightAttacks[square];
}

U64 PieceAttacks::getBishopAttacks(const byte square, U64 blockers) noexcept
{
	return generateBishopAttacks(square, blockers);
	blockers &= bishopMasks[square];
	const U64 key = (blockers * bishopMagics[square]) >> (64u - bishopIndexBits[square]);
	return s_BishopAttacks[square][key];
}

U64 PieceAttacks::getRookAttacks(const byte square, U64 blockers) noexcept
{
	blockers &= rookMasks[square];
	const U64 key = (blockers * rookMagics[square]) >> (64u - rookIndexBits[square]);
	return s_RookAttacks[square][key];
}

U64 PieceAttacks::getKingAttacks(const byte square) noexcept
{
	return s_KingAttacks[square];
}
