#pragma once

#include <cstdint>

#include "Player.h"
#include "Piece.h"
#include "../BoardManager.h"
#include "../algorithm/Evaluation.h"

using U64 = std::uint64_t;

class Board final
{
public:
	Piece data[8][8];
	U64 key = 0u;
	bool whiteCastled = false;
	bool blackCastled = false;
	State state = State::NONE;
	bool whiteToMove = true;
	short score = 0;
	short npm = 0;
	Pos enPassantPos{};
	// Index 0 -> black, index 1 -> white
	byte kingSquare[2]{};
	U64 pieces[2]{};
	bool isPromotion = false;
	bool isCapture = false;

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

	inline Piece &getPiece(const byte x, const byte y) noexcept
	{
		return data[x][y];
	}
	inline const Piece &getPiece(const byte x, const byte y) const noexcept
	{
		return data[x][y];
	}
	const Piece &at(const byte x, const byte y) const noexcept;

	void initDefaultBoard() noexcept;
	void updateState() noexcept;
	Phase getPhase() const noexcept;
	StackVector<std::pair<Pos, Piece>, 32> getAllPieces() const noexcept;

	template<class T> // RootMove or Board
	StackVector<T, 150> listValidMoves(bool isWhite) const noexcept;
	StackVector<Board, 50> listQuiescenceMoves(bool isWhite) const noexcept;
};

class RootMove final
{
public:
	Pos start;
	Pos dest;
	Board board;

	RootMove() = default;

	RootMove(const Pos start, const Pos dest, const Board &board) noexcept
		: start(start), dest(dest), board(board) {}

	bool operator<(const RootMove &other) const noexcept
	{
		return board < other.board;
	}

	bool operator>(const RootMove &other) const noexcept
	{
		return board > other.board;
	}
};

template<class T>
StackVector<T, 150> Board::listValidMoves(const bool isWhite) const noexcept
{
	const auto pieces = Player::getAllOwnedPieces(isWhite, *this);
	StackVector<T, 150> moves;

	for (const auto &pair : pieces)
	{
		const Pos &startPos = pair.first;
		const Piece &selectedPiece = pair.second;
		const auto possibleMoves = selectedPiece.getPossibleMoves(startPos, *this);

		for (const auto &destPos : possibleMoves)
		{
			const auto &destPiece = (*this)[destPos];
			if (destPiece.type == Type::KING)
				continue;

			Board board = *this;
			BoardManager::movePieceInternal(startPos, destPos, board);

			if (board.state == State::INVALID)
				continue;
			if (isWhite && (board.state == State::WHITE_IN_CHECK || board.state == State::WINNER_BLACK))
				continue;
			if (!isWhite && (board.state == State::BLACK_IN_CHECK || board.state == State::WINNER_WHITE))
				continue;

			board.score = Evaluation::simpleEvaluation(board);

			if constexpr (std::is_same_v<T, RootMove>)
			{
				int count = 1;

				for (const auto &game : BoardManager::getMovesHistory())
				{
					if (board.whiteToMove == game.board.whiteToMove &&
						board.state == game.board.state &&
						board.key == game.board.key)
						count++;

					if (count == 3)
					{
						board.score = 0;
						board.state = State::DRAW;
						break;
					}
				}

				moves.emplace_back(startPos, destPos, board);
			}
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
