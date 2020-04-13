#pragma once

#include <algorithm>

#include "../data/Move.h"
#include "../data/Board.h"

Move *generateMoves(const Board &board, Move *moveList) noexcept;

class MoveList
{
public:
	explicit MoveList(Board& board)
		: _board(board), _end(generateMoves(board, _moveList))
	{
	}

	constexpr const Move &front() const noexcept { return *_moveList; }
	constexpr Move &front() noexcept { return *_moveList; }
	
	constexpr const Move *begin() const noexcept { return _moveList; }
	constexpr Move *begin() noexcept { return _moveList; }

	constexpr const Move *end() const noexcept { return _end; }
	constexpr Move *end() noexcept { return _end; }
	
	constexpr size_t size() const noexcept { return _end - _moveList; }
	constexpr bool empty() const noexcept { return (_end - _moveList) == 0u; }

	void keepLegalMoves() noexcept
	{
		_end = std::remove_if(_moveList, _end, [&] (const Move &move) -> bool
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
