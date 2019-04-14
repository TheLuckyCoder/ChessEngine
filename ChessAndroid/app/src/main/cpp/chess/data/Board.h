#pragma once

#include <cstdint>

#include "Player.h"
#include "Piece.h"
#include "../BoardManager.h"

using U64 = std::uint64_t;

class Board final
{
public:
	Piece data[8][8];
	U64 key = 0;
	bool whiteCastled = false;
	bool blackCastled = false;
	byte whiteKingSquare = 0;
	byte blackKingSquare = 0;
	State state = State::NONE;
	bool whiteToMove = true;
	int score = 0;
	int npm = 0;

	Board() = default;
	Board(Board&&) = default;
	Board(const Board &board) = default;
	~Board() = default;

	Board &operator=(Board&&) = default;
	Board &operator=(const Board &other) = default;

	Piece &operator[](const Pos &pos) noexcept;
	const Piece &operator[](const Pos &pos) const noexcept;

	bool operator<(const Board &other) const noexcept;
	bool operator>(const Board &other) const noexcept;

	const Piece &getPieceSafely(byte x, byte y) const noexcept;

	void initDefaultBoard() noexcept;
	void updateState() noexcept;
	Phase getPhase() const noexcept;
	StackVector<std::pair<Pos, Piece>, 32> getAllPieces() const noexcept;

	template<class T> // Move or Board
	StackVector<T, 150> listValidMoves(bool isWhite) const noexcept;
	StackVector<Board, 50> listValidCaptures(bool isWhite) const noexcept;
};

class Move final
{
public:
	Pos start;
	Pos dest;
	Board board;

	Move() = default;

	Move(const Pos start, const Pos dest, const Board &board)
		: start(start), dest(dest), board(board) {}

	bool operator<(const Move &other) const
	{
		return board.score < other.board.score;
	}

	bool operator>(const Move &other) const
	{
		return board.score > other.board.score;
	}
};

template<class T>
StackVector<T, 150> Board::listValidMoves(const bool isWhite) const noexcept
{
	const auto pieces = Player::getAllOwnedPieces(isWhite, *this);
	StackVector<T, 150> moves;

	for (const auto &pair : pieces)
	{
		const auto &startPos = pair.first;
		const auto &selectedPiece = pair.second;
		const auto possibleMoves = selectedPiece.getPossibleMoves(startPos, *this);

		for (const auto &destPos : possibleMoves)
		{
			if ((*this)[destPos].type == Type::KING)
				continue;

			Board board = *this;
			BoardManager::movePieceInternal(startPos, destPos, board);

			if (board.state == State::INVALID)
				continue;
			if (isWhite && (board.state == State::WHITE_IN_CHESS || board.state == State::WINNER_BLACK))
				continue;
			if (!isWhite && (board.state == State::BLACK_IN_CHESS || board.state == State::WINNER_WHITE))
				continue;

			if constexpr (std::is_same_v<T, Move>)
				moves.emplace_back(startPos, destPos, board);
			else if (std::is_same_v<T, Board>)
				moves.push_back(board);
		}
	}

	if (isWhite)
		std::sort(moves.begin(), moves.end(), std::greater<>());
	else
		std::sort(moves.begin(), moves.end());

	return moves;
}
