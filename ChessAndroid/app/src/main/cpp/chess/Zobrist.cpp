#include "Zobrist.h"

#include "Board.h"

using HashArray = std::array<std::array<std::array<u64, 7>, 2>, SQUARE_NB>;

static HashArray _pieces{};
static u64 _side;
static std::array<u64, 6> _castlingRights;
static std::array<u64, 8> _enPassant;

/*
 * Generate random numbers based on this paper: http://vigna.di.unimi.it/ftp/papers/xorshift.pdf
 */
u64 random64() noexcept
{
	static u64 seed = 1070372ull;

	seed ^= seed >> 12u;
	seed ^= seed << 25u;
	seed ^= seed >> 27u;

	return seed * 2685821657736338717ull;
}

void Zobrist::init()
{
	static bool initialized = false;
	if (initialized) return;
	initialized = true;

	for (auto &i : _pieces)
		for (auto &color : i)
			for (auto &piece : color)
				piece = random64();

	for (auto &rights : _castlingRights)
		rights = random64();

	for (auto &square : _enPassant)
		square = random64();

	_side = random64();
}

u64 Zobrist::compute(const Board &board) noexcept
{
	u64 hash{};

	for (u8 sq = 0; sq < SQUARE_NB; ++sq)
		if (const Piece &piece = board.getPiece(toSquare(sq)); piece)
			hash ^= _pieces[sq][piece.color()][piece.type()];

	xorCastlingRights(hash, static_cast<CastlingRights>(board.castlingRights));
	xorEnPassant(hash, board.enPassantSq);

	if (board.colorToMove)
		flipSide(hash);

	return hash;
}

void Zobrist::xorPiece(u64 &key, const Square square, const Piece piece) noexcept
{
	key ^= _pieces[square][piece.color()][piece.type()];
}

void Zobrist::flipSide(u64 &key) noexcept
{
	key ^= _side;
}

void Zobrist::xorCastlingRights(u64 &key, const CastlingRights rights) noexcept
{
	if (rights & CASTLE_WHITE_BOTH)
		key ^= _castlingRights[0];
	else if (rights & CASTLE_WHITE_KING)
		key ^= _castlingRights[1];
	else if (rights & CASTLE_WHITE_QUEEN)
		key ^= _castlingRights[2];

	if (rights & CASTLE_BLACK_BOTH)
		key ^= _castlingRights[3];
	else if (rights & CASTLE_BLACK_KING)
		key ^= _castlingRights[4];
	else if (rights & CASTLE_BLACK_QUEEN)
		key ^= _castlingRights[5];
}

void Zobrist::xorEnPassant(u64 &key, const Square square) noexcept
{
	if (square <= SQUARE_NB)
		key ^= _enPassant[fileOf(square)];
}
