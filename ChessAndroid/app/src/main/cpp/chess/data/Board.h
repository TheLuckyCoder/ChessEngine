#pragma once

#include <array>
#include <unordered_map>

#include "Player.h"
#include "pieces/Evaluation.h"
#include "pieces/Piece.h"
#include "../minimax/MiniMax.h"
#include "../BoardManager.h"

// Class that keeps a 2D Array
class Board final
{
public:
	std::array<std::array<Piece, 8>, 8> data;
	std::uint64_t hash = 0;
	bool whiteCastled = false;
	bool blackCastled = false;
	int value;
	GameState state = GameState::NONE;

	Board() = default;
	Board(Board&&) = default;
	Board(const Board &board);
	~Board() = default;

	Board &operator=(Board&&) = default;
	Board &operator=(const Board &other);
	Piece &operator[](const Pos &pos);
	const Piece &operator[](const Pos &pos) const;
	bool operator<(const Board &other) const;

	void initDefaultBoard();
	std::unordered_map<Pos, Piece> getAllPieces() const;

	template<class T> // Move or Board
	StackVector<T, 90> listValidMoves(bool isWhite) const;
};

class Move
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
};

template<class T>
StackVector<T, 90> Board::listValidMoves(const bool isWhite) const
{
	const auto pieces = Player::getAllOwnedPieces(isWhite, *this);
	StackVector<T, 90> moves;

	for (const auto &pair : pieces)
	{
		const auto &startPos = pair.first;
		const auto possibleMoves = pair.second.getPossibleMoves(startPos, *this);

		for (const auto &destPos : possibleMoves)
		{
			if ((*this)[destPos].type == Piece::Type::KING)
				continue;

			Board board = *this;
			BoardManager::movePieceInternal(startPos, destPos, board);

			if ((isWhite && board.state == GameState::WHITE_IN_CHESS) ||
				(!isWhite && board.state == GameState::BLACK_IN_CHESS))
				continue;

			switch (board.state)
			{
			case GameState::NONE:
			case GameState::WHITE_IN_CHESS:
			case GameState::BLACK_IN_CHESS:
				board.value = Evaluation::evaluate(board);
				break;
			case GameState::DRAW:
				board.value = 0;
				break;
			case GameState::WINNER_WHITE:
				board.value = MiniMax::VALUE_WINNER_WHITE;
				break;
			case GameState::WINNER_BLACK:
				board.value = MiniMax::VALUE_WINNER_BLACK;
				break;
			}

			if constexpr (std::is_same_v<T, Move>)
				moves.emplace_back(startPos, destPos, std::move(board));
			else if constexpr (std::is_same_v<T, Board>)
				moves.push_back(std::move(board));

			++BoardManager::boardsEvaluated;
		}
	}

	std::sort(moves.begin(), moves.end());

	return moves;
}
