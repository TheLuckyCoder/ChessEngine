#pragma once

#include <array>

#include "Player.h"
#include "pieces/Evaluation.h"
#include "pieces/Piece.h"
#include "../BoardManager.h"

class Board final
{
public:
	std::array<std::array<Piece, 8>, 8> data;
	std::uint64_t hash = 0;
	bool whiteCastled = false;
	bool blackCastled = false;
	State state = State::NONE;
	int value = 0;

	Board() = default;
	Board(Board&&) = default;
	Board(const Board &board) noexcept;
	~Board() = default;

	Board &operator=(Board&&) = default;
	Board &operator=(const Board &other) noexcept;

	Piece &operator[](const Pos &pos) noexcept;
	const Piece &operator[](const Pos &pos) const noexcept;

	bool operator<(const Board &other) const noexcept;
	bool operator>(const Board &other) const noexcept;

	void initDefaultBoard() noexcept;
	StackVector<std::pair<Pos, Piece>, 32> getAllPieces() const noexcept;

	template<class T> // Move or Board
	StackVector<T, 150> listValidMoves(bool isWhite) const noexcept;
	StackVector<Board, 50> listValidMovesQ(bool isWhite) const noexcept;
};

class Move final
{
public:
	Pos start;
	Pos dest;
	Board board;

	Move(const Pos start, const Pos dest, Board &&board)
		: start(start), dest(dest), board(std::move(board)) {}

	bool operator<(const Move &other) const
	{
		return board.value < other.board.value;
	}

	bool operator>(const Move &other) const
	{
		return board.value > other.board.value;
	}
};

class Cache final
{
public:
	State state;
	int value;
};

template<class T>
StackVector<T, 150> Board::listValidMoves(const bool isWhite) const noexcept
{
	const auto pieces = Player::getAllOwnedPieces(isWhite, *this);
	StackVector<T, 150> moves;

	for (const auto &pair : pieces)
	{
		const auto &selectedPos = pair.first;
		const auto &selectedPiece = pair.second;
		const auto possibleMoves = selectedPiece.getPossibleMoves(selectedPos, *this);

		for (const auto &destPos : possibleMoves)
		{
			if ((*this)[destPos].type == Piece::Type::KING)
				continue;

			Board board = *this;
			BoardManager::movePieceInternal(selectedPos, destPos, board);

			if ((isWhite && board.state == State::WHITE_IN_CHESS) ||
				(!isWhite && board.state == State::BLACK_IN_CHESS))
				continue;

			if constexpr (std::is_same_v<T, Move>)
				moves.emplace_back(selectedPos, destPos, std::move(board));
			else if constexpr (std::is_same_v<T, Board>)
				moves.push_back(std::move(board));
		}
	}

	if (isWhite)
		std::sort(moves.begin(), moves.end(), std::greater<>());
	else
		std::sort(moves.begin(), moves.end());

	return moves;
}
