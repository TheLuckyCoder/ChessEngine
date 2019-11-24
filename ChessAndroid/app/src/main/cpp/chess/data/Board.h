#pragma once

#include <array>
#include <string>
#include <vector>

#include "Player.h"
#include "Piece.h"
#include "../BoardManager.h"
#include "../algorithm/Evaluation.h"

using U64 = std::uint64_t;

class Board final
{
	friend class FenParser;

public:
	std::array<Piece, 64> data{};
	U64 zKey{};
	State state = State::NONE;
	Color colorToMove = WHITE;
	bool isPromotion = false;
	bool isCapture = false;
	byte castlingRights = CastlingRights::CASTLE_WHITE | CastlingRights::CASTLE_BLACK;
	U64 enPassant{};
	byte halfMoveClock{};
	U64 occupied{};
	std::array<U64, 2> allPieces{};
	std::array<std::array<U64, 7>, 2> pieces{};
	short score{};
	short npm{};

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

	void initDefaultBoard() noexcept;
	void setToFen(const std::string &fen);

	bool canCastle(Color color) const noexcept;
	bool canCastleKs(Color color) const noexcept;
	bool canCastleQs(Color color) const noexcept;
	bool isCastled(Color color) const noexcept;

	Piece &getPiece(byte squareIndex) noexcept;
	const Piece &getPiece(byte squareIndex) const noexcept;
	Piece &getPiece(byte x, byte y) noexcept;
	const Piece &getPiece(byte x, byte y) const noexcept;
	const Piece &at(byte x, byte y) const noexcept;
	U64 &getType(Color color, PieceType type) noexcept;
	U64 getType(Color color, PieceType type) const noexcept;

	void updateState() noexcept;
	bool hasValidState() const noexcept;
	Phase getPhase() const noexcept;
	std::vector<std::pair<Pos, Piece>> getAllPieces() const;

	template<class T> // RootMove or Board
	std::vector<T> listValidMoves() const noexcept;
	std::vector<Board> listQuiescenceMoves() const;

	void doMove(byte startSq, byte destSq, bool updateState = true) noexcept;

private:
	bool movePawn(byte startSq, byte destSq);
	void moveRook(byte startSq);
	void moveKing(const Piece &king, byte startSq, byte destSq);

	void updateNonPieceBitboards();
};

class RootMove final
{
public:
	byte startSq;
	byte destSq;
	Board board;

	RootMove() = default;

	RootMove(const byte startSq, const byte destSq, const Board &board) noexcept
		: startSq(startSq), destSq(destSq), board(board) {}

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
std::vector<T> Board::listValidMoves() const noexcept
{
	const auto pieces = Player::getAllOwnedPieces(colorToMove, *this);
	std::vector<T> moves;
	moves.reserve(100);

	for (const auto &pair : pieces)
	{
		const byte startSq = pair.first;
		const Piece &selectedPiece = pair.second;
		U64 possibleMoves = selectedPiece.getPossibleMoves(startSq, *this);

		while (possibleMoves)
		{
			const byte destSq = Bitboard::findNextSquare(possibleMoves);
			const auto &destPiece = getPiece(destSq);
			if (destPiece.type == PieceType::KING)
				continue;

			Board board = *this;
			board.doMove(startSq, destSq);

            if (!board.hasValidState())
                continue;

			board.score = Evaluation::simpleEvaluation(board);

			if constexpr (std::is_same_v<T, RootMove>)
			{
				int count = 1;

				for (const auto &game : BoardManager::getMovesHistory())
				{
					if (board.colorToMove == game.board.colorToMove &&
						board.state == game.board.state &&
						board.zKey == game.board.zKey)
						count++;

					if (count == 3)
					{
						board.score = 0;
						board.state = State::DRAW;
						break;
					}
				}

				moves.emplace_back(startSq, destSq, board);
			}
			else if (std::is_same_v<T, Board>)
				moves.push_back(board);
		}
	}

	if (colorToMove)
		std::sort(moves.begin(), moves.end(), std::greater<>());
	else
		std::sort(moves.begin(), moves.end());

	return moves;
}
