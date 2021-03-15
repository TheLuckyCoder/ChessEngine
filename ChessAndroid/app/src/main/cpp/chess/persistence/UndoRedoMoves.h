#pragma once

#include <optional>
#include <vector>

#include "../Board.h"

struct IndexedPiece
{
	IndexedPiece(const i32 id, const Square square, const Color pieceColor, const PieceType pieceType)
		: id(id), square(square), pieceColor(pieceColor), pieceType(pieceType)
	{
	}

	/**
	 * An unique ID assigned to this piece
	 */
	i32 id;
	/**
	 * The square on which the Piece is located
	 */
	Square square;
	Piece piece;
	Color pieceColor;
	PieceType pieceType;
};

using IndexedPieces = std::vector<IndexedPiece>;

namespace UndoRedo
{
	static std::vector<IndexedPiece> incrementIndexedPieces(IndexedPieces pieces, const std::string fen, const Move move) noexcept
	{
		Board board;
		board.setToFen(fen);

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
			const Square capturedSq = toSquare(u8(to) + static_cast<u8>(board.colorToMove ? -8 : 8));
			removePiece(capturedSq);
		} else if (flags.kSideCastle())
		{
			if (to == SQ_G1)
				movePiece(SQ_A1, SQ_D1);
			else if (to == SQ_G8)
				movePiece(SQ_A8, SQ_D8);
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
				pieces.emplace_back(i32(square), toSquare(square), piece.color(), piece.type());
		}

		return pieces;
	}

	class HistoryBoard
	{
	public:
		HistoryBoard(const Board &board, const Move &move)
			: _fen(board.getFen()), _pieces(makeIndexedPieces(board)), _move(move), _colorToMove(board.colorToMove)
		{
		}

		HistoryBoard(const IndexedPieces &previousPieces, const Board &board, const Move &move)
			: _fen(board.getFen()), _pieces(incrementIndexedPieces(previousPieces, _fen, move)), _move(move),
			  _colorToMove(board.colorToMove)
		{
		}

		const auto &getFen() const noexcept { return _fen; }

		const auto &getIndexedPieces() const noexcept { return _pieces; }

		Move getMove() const noexcept { return _move; }

		bool colorToMove() const noexcept { return _colorToMove; }

	private:
		std::string _fen;
		IndexedPieces _pieces;
		Move _move;
		Color _colorToMove;
	};

	using HistoryBoardPair = std::pair<HistoryBoard, std::optional<HistoryBoard>>;

	class HistoryStack
	{
	public:
		HistoryStack() = default;

		HistoryStack(const Board &board)
			: _initialPieces(makeIndexedPieces(board))
		{
			_data.reserve(64);
		}

		void add(const Board &board, const Move move) noexcept
		{
			assert(!move.empty());

			if (!_data.empty() && _current != std::prev(_data.end()))
				_data.erase(_current, _data.end());

			_current = std::prev(_data.end());

			if (!empty())
			{
				auto &&pair = peekPair();
				if (pair.second.has_value() || pair.first.colorToMove() == board.colorToMove)
					_data.emplace_back(HistoryBoard{ pair.second.value_or(pair.first).getIndexedPieces(), board, move }, std::nullopt);
				else
					pair.second = HistoryBoard{ pair.first.getIndexedPieces(), board, move };
			} else
				_data.emplace_back(HistoryBoard{ _initialPieces, board, move }, std::nullopt);

			_current = std::prev(_data.end());
		}

		bool undo() noexcept
		{
			if (_current != _data.begin())
			{
				--_current;
				return true;
			}

			return false;
		}

		bool redo() noexcept
		{
			if (_current != std::prev(_data.end()))
			{
				++_current;
				return true;
			}

			return false;
		}

		bool empty() const noexcept { return _data.empty(); }

		HistoryBoardPair &peekPair() noexcept
		{
			assert(!empty());
			return *_current;
		}

		HistoryBoard peek() noexcept
		{
			auto &&pair = peekPair();
			return pair.second.value_or(pair.first);
		}

		const auto &getInitialPieces() const noexcept { return _initialPieces; }

	private:
		IndexedPieces _initialPieces;
		std::vector<HistoryBoardPair> _data;
		std::vector<HistoryBoardPair>::iterator _current = _data.begin();
	};
}