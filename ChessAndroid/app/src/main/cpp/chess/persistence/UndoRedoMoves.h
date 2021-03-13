#pragma once

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

class IndexedBoard
{
public:
	IndexedBoard(const Board &board)
		: _fen(board.getFen())
	{
		_pieces.reserve(32);

		for (u8 square{}; square < SQUARE_NB; ++square)
		{
			auto &&piece = board.data[square];
			if (piece.isValid())
				_pieces.emplace_back(i32(square), toSquare(square), piece.color(), piece.type());
		}
	}

	IndexedBoard(const IndexedBoard &indexedBoard, const Board &board, Move move)
		: _fen(board.getFen()), _pieces(indexedBoard.generateIndexedPieces(board, move))
	{
	}

private:
	std::vector<IndexedPiece> generateIndexedPieces(const Board &board, const Move move) const noexcept
	{
		auto pieces = _pieces;

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

		const Square from = move.from();
		const Square to = move.to();
		const Color side = board.colorToMove;
		const auto flags = move.flags();

		// Handle en passant capture and castling
		if (flags.enPassant())
		{
			const Square capturedSq = toSquare(u8(to) + static_cast<u8>(side ? -8 : 8));

			removePiece(capturedSq);
		} else if (flags.kSideCastle())
		{
			switch (to)
			{
				case SQ_G1:
					movePiece(SQ_H1, SQ_F1);
					break;
				case SQ_G8:
					movePiece(SQ_H8, SQ_F8);
					break;
				default:
					break;
			}
		} else if (flags.qSideCastle())
		{
			switch (to)
			{
				case SQ_C1:
					movePiece(SQ_A1, SQ_D1);
					break;
				case SQ_C8:
					movePiece(SQ_A8, SQ_D8);
					break;
				default:
					break;
			}
		}

		if (const PieceType capturedType = move.capturedPiece();
			capturedType != PieceType::NO_PIECE_TYPE)
			removePiece(to);

		movePiece(from, to);

		if (move.flags().promotion())
			promotePawn(to, move.promotedPiece());

		return pieces;
	}


public:
	const auto &getFen() const noexcept { return _fen; }
	const auto &getIndexedPieces() const noexcept { return _pieces; }

private:
	std::string _fen;
	IndexedPieces _pieces;
};

class UndoRedoHistory
{
public:
	UndoRedoHistory()
	{
		_data.reserve(64);
	}

	void add(const Board &board, const Move move) noexcept
	{
		assert(!move.empty());

		if (_current != _data.end() - 1)
			_data.erase(_current, _data.end());

		_data.emplace_back(_data.back(), board, move);

		_current = _data.end() - 1;
		colorToMove = board.colorToMove;
	}

	bool undo() noexcept
	{
		if (_current != _data.begin())
		{
			--_current;
			colorToMove = ~colorToMove;
			return true;
		}

		return false;
	}

	bool redo() noexcept
	{
		if (_current != _data.end() - 1)
		{
			++_current;
			colorToMove = ~colorToMove;
			return true;
		}

		return false;
	}

	const IndexedBoard &peek() noexcept
	{
		return *_current;
	}

	void reset(const Board &board) noexcept
	{
		_data.clear();
		_data.emplace_back(board);
		_current = _data.begin();
		colorToMove = board.colorToMove;
	}

	Color colorToMove{};

private:
	std::vector<IndexedBoard> _data;
	std::vector<IndexedBoard>::iterator _current = _data.begin();
};
