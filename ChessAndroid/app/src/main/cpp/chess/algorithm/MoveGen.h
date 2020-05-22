#pragma once

#include <algorithm>
#include <string>

#include "../data/Move.h"
#include "../data/Board.h"

Move *generateMoves(const Board &board, Move *moveList) noexcept;

class MoveList
{
public:
	explicit MoveList(Board &board)
		: _board(board), _end(generateMoves(board, _moveList))
	{
	}

	constexpr Move *begin() noexcept { return _moveList; }
	constexpr const Move *begin() const noexcept { return _moveList; }

	constexpr Move *end() noexcept { return _end; }
	constexpr const Move *end() const noexcept { return _end; }

	constexpr Move &front() noexcept { return *begin(); }
	constexpr const Move &front() const noexcept { return *begin(); }

	constexpr Move &back() noexcept { return *(end() - 1); }
	constexpr const Move &back() const noexcept { return *(end() - 1); }

	constexpr void pop_back() noexcept { --_end; }

	constexpr size_t size() const noexcept { return _end - _moveList; }

	constexpr bool empty() const noexcept { return (_end - _moveList) == 0u; }

	void keepLegalMoves() noexcept
	{
		_end = std::remove_if(_moveList, _end, [&](const Move &move) -> bool
		{
			if (_board.makeMove(move))
			{
				_board.undoMove();
				return false;
			}
			return true;
		});
	}

private:
	Board &_board;
	Move _moveList[MAX_MOVES];
	Move *_end;
};

inline bool moveExists(Board &board, const Move &move) noexcept
{
	const MoveList moveList(board);

	for (const Move &m : moveList)
	{
		if (m == move)
		{
			if (board.makeMove(m))
			{
				board.undoMove();
				return true;
			}
			return false;
		}
	}

	return false;
}

inline Move parseMove(Board &board, const std::string &str)
{
	if (str[1] > '8' || str[1] < '1'
		|| str[3] > '8' || str[3] < '1'
		|| str[0] > 'h' || str[0] < 'a'
		|| str[2] > 'h' || str[2] < 'a') return {};

    const byte from = toSquare(str[0] - 'a', str[1] - '1');
    const byte to = toSquare(str[2] - 'a', str[3] - '1');	

	const MoveList moveList(board);

	for (const Move &move : moveList)
	{
		if (move.from() == from && move.to() == to)
		{
			if (move.flags().promotion())
			{
				const PieceType promoted = move.promotedPiece();
				if ((promoted == KNIGHT && str[4] == 'k')
					|| (promoted == BISHOP && str[4] == 'b')
					|| (promoted == ROOK && str[4] == 'r')
					|| (promoted == ROOK && str[4] == 'q'))
					return move;
				
				continue;
			}
			return move;
		}
	}

	return {};
}
