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

class HistoryBoard
{
public:
	HistoryBoard(const Board &board)
		: _fen(board.getFen()), _colorToMove(board.colorToMove)
	{
		_pieces.reserve(32);

		for (u8 square{}; square < SQUARE_NB; ++square)
		{
			auto &&piece = board.data[square];
			if (piece.isValid())
				_pieces.emplace_back(i32(square), toSquare(square), piece.color(), piece.type());
		}
	}

	HistoryBoard(const HistoryBoard &historyBoard, const Board &board, Move move)
		: _fen(board.getFen()), _pieces(historyBoard.generateIndexedPieces(move)), _move(move), _colorToMove(board.colorToMove)
	{
	}

private:
	std::vector<IndexedPiece> generateIndexedPieces(const Move move) const noexcept
	{
		Board board;
		board.setToFen(_fen);
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

		movePiece(from, to);

		if (move.flags().promotion())
			promotePawn(to, move.promotedPiece());

		return pieces;
	}


public:
	const auto &getFen() const noexcept { return _fen; }

	const auto &getIndexedPieces() const noexcept { return _pieces; }

	auto getMove() const noexcept { return _move; }

	auto colorToMove() const noexcept { return _colorToMove; }

private:
	std::string _fen;
	IndexedPieces _pieces;
	Move _move;
	Color _colorToMove;
};

struct HistoryBoardPair
{

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

	const HistoryBoard &peek() noexcept
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
	std::vector<HistoryBoard> _data;
	std::vector<HistoryBoard>::iterator _current = _data.begin();
};
