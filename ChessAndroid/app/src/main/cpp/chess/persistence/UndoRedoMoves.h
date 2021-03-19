#pragma once

#include <optional>
#include <vector>

#include "../Board.h"

struct IndexedPiece
{
	IndexedPiece(const u8 id, const Square square, const Color pieceColor, const PieceType pieceType)
		: id(id), square(square), pieceColor(pieceColor), pieceType(pieceType)
	{
	}

	/**
	 * An unique ID assigned to this piece
	 */
	u8 id;
	/**
	 * The square on which the Piece is located
	 */
	Square square;
	Color pieceColor;
	PieceType pieceType;
};

using IndexedPieces = std::vector<IndexedPiece>;

namespace UndoRedo
{
	static std::vector<IndexedPiece> incrementIndexedPieces(
		IndexedPieces pieces, const Color colorToMove, const Move move) noexcept
	{
		const auto findPiece = [&](const Square square)
		{
			const auto it = std::find_if(pieces.begin(), pieces.end(), [&square](const IndexedPiece &piece)
			{
				return piece.square == square;
			});
			assert(it != pieces.end());
			return it;
		};

		const auto movePiece = [&](const Square from, const Square to) { findPiece(from)->square = to; };
		const auto promotePawn = [&](const Square square, const PieceType to) { findPiece(square)->pieceType = to; };
		const auto removePiece = [&](const Square square) { pieces.erase(findPiece(square)); };

		const Square to = move.to();
		const auto flags = move.flags();

		// Handle en passant capture and castling
		if (flags.enPassant())
		{
			const Square capturedSq = toSquare(u8(to) + static_cast<u8>(colorToMove ? -8 : 8));
			removePiece(capturedSq);
		} else if (flags.kSideCastle())
		{
			if (to == SQ_G1)
				movePiece(SQ_H1, SQ_F1);
			else if (to == SQ_G8)
				movePiece(SQ_H8, SQ_F8);
		} else if (flags.qSideCastle())
		{
			if (to == SQ_C1)
				movePiece(SQ_A1, SQ_D1);
			else if (to == SQ_C8)
				movePiece(SQ_A8, SQ_D8);
		}

		if (const PieceType capturedType = move.capturedPiece();
			capturedType != PieceType::NO_PIECE_TYPE)
			removePiece(to);

		movePiece(move.from(), to);

		if (move.flags().promotion())
			promotePawn(to, move.promotedPiece());

		return pieces;
	}

	static IndexedPieces makeIndexedPieces(const Board &board) noexcept
	{
		IndexedPieces pieces;
		pieces.reserve(board.occupied.popcount());

		for (u8 square{}; square < SQUARE_NB; ++square)
		{
			auto &&piece = board.data[square];
			if (piece.isValid())
				pieces.emplace_back(square, toSquare(square), piece.color(), piece.type());
		}

		return pieces;
	}

	class HistoryBoard
	{
	public:
		HistoryBoard(const Board &board, const Move &move)
			: _pieces(makeIndexedPieces(board)), _move(move), _colorToMove(board.colorToMove)
		{
		}

		HistoryBoard(const IndexedPieces &previousPieces, const Board &board, const Move &move)
			: _pieces(incrementIndexedPieces(previousPieces, board.colorToMove, move)), _move(move),
			  _colorToMove(board.colorToMove)
		{
		}

		const auto &getIndexedPieces() const noexcept { return _pieces; }

		Move getMove() const noexcept { return _move; }

		bool colorToMove() const noexcept { return _colorToMove; }

	private:
		IndexedPieces _pieces;
		Move _move;
		Color _colorToMove;
	};

	class HistoryStack
	{
	public:
		HistoryStack() = default;

		HistoryStack(const Board &board)
			: _initialPieces(makeIndexedPieces(board))
		{
			_data.reserve(64);
		}

		void push(const Board &board, const Move move) noexcept
		{
			assert(!move.empty());

			if (_index < static_cast<i64>(_data.size()) && !_data.empty())
			{
				const auto it = _data.begin() + _index;
				_data.erase(it + 1, _data.end());
			}

			if (!_data.empty())
				_data.emplace_back(peek().getIndexedPieces(), board, move);
			else
				_data.emplace_back(_initialPieces, board, move);

			_index = _data.size() - 1u;
		}

		Move undo() noexcept
		{
			if (_index >= 0 && _index < static_cast<i64>(_data.size()))
			{
				const Move move = peek().getMove();
				--_index;
				return move;
			}

			return {};
		}

		Move redo() noexcept
		{
			if (_index + 1 < static_cast<i64>(_data.size()))
			{
				++_index;
				return peek().getMove();
			}

			return {};
		}

		auto size() const noexcept { return _data.size(); }

		const HistoryBoard &peek() const noexcept
		{
			if (_index < 0)
				return _data.front();
			if (_index >= static_cast<i64>(_data.size()))
				return _data.back();
			return _data[_index];
		}

		const auto &getIndexedPieces() const noexcept
		{
			return (_index >= 0 && _index < static_cast<i64>(_data.size()))
				   ? peek().getIndexedPieces() : _initialPieces;
		}

		auto begin() const noexcept { return _data.begin(); }

		auto end() const noexcept { return _data.end(); }

	private:
		IndexedPieces _initialPieces;
		std::vector<HistoryBoard> _data;
		i64 _index{ -1l };
	};
}
