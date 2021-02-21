#include "Zobrist.h"

#include <array>

#include "Board.h"

class RandomGenerator
{
public:
	/*
	 * Generate random numbers based on this paper: http://vigna.di.unimi.it/ftp/papers/xorshift.pdf
	 */
	constexpr u64 random64() noexcept
	{
		seed ^= seed >> 12u;
		seed ^= seed << 25u;
		seed ^= seed >> 27u;

		return seed * 2685821657736338717ull;
	}

	template <usize N>
	constexpr auto randomArray() noexcept
	{
		std::array<u64, N> array{};

		for (auto &item : array)
			item = random64();

		return array;
	}

private:
	u64 seed = 1070372ull;
};

constinit RandomGenerator _rand;

static auto _pieces = [] {
	std::array<std::array<std::array<u64, 2>, 7>, SQUARE_NB> array{};

	for (auto &sq : array)
		for (auto &piece : sq)
			piece = _rand.randomArray<2>();

	return array;
}();
static const u64 _side{ _rand.random64() };
static const auto _castlingRights{ _rand.randomArray<4>() };
static const auto _enPassant{ _rand.randomArray<8>() };

u64 Zobrist::compute(const Board &board) noexcept
{
	u64 hash{};

	for (u8 sq = 0; sq < SQUARE_NB; ++sq)
		if (const Piece &piece = board.getPiece(toSquare(sq)); piece)
			hash ^= _pieces[sq][piece.type()][piece.color()];

	xorCastlingRights(hash, static_cast<CastlingRights>(board.castlingRights));
	xorEnPassant(hash, board.enPassantSq);

	if (board.colorToMove)
		flipSide(hash);

	return hash;
}

void Zobrist::xorPiece(u64 &key, const Square square, const Piece piece) noexcept
{
	key ^= _pieces[square][piece.type()][piece.color()];
}

void Zobrist::flipSide(u64 &key) noexcept
{
	key ^= _side;
}

void Zobrist::xorCastlingRights(u64 &key, const CastlingRights rights) noexcept
{
	if (rights & CASTLE_WHITE_KING)
		key ^= _castlingRights[1];
	if (rights & CASTLE_WHITE_QUEEN)
		key ^= _castlingRights[2];

	if (rights & CASTLE_BLACK_KING)
		key ^= _castlingRights[4];
	if (rights & CASTLE_BLACK_QUEEN)
		key ^= _castlingRights[5];
}

void Zobrist::xorEnPassant(u64 &key, const Square square) noexcept
{
	if (square <= SQUARE_NB)
		key ^= _enPassant[fileOf(square)];
}
