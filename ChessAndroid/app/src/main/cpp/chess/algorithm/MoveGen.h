#pragma once

#include <algorithm>

#include "../data/Move.h"
#include "../data/Board.h"

Move *generateMoves(const Board &board, Move *moveList) noexcept;

class MoveList
{
public:
	explicit MoveList(const Board& board) : _end(generateMoves(board, _moveList)) {}
	
	constexpr const Move *begin() const noexcept { return _moveList; }
	constexpr const Move *end() const noexcept { return _end; }

	constexpr Move *begin() noexcept { return _moveList; }
	constexpr Move *end() noexcept { return _end; }
	
	constexpr size_t size() const noexcept { return _end - _moveList; }
	constexpr bool empty() const noexcept { return (_end - _moveList) == 0u; }

	void keepLegalMoves(Board &board) noexcept
	{
		_end = std::remove_if(_moveList, _end, [&] (const Move &move) -> bool
		{
			if (board.makeMove(move))
			{
				board.undoMove();
				return false;
			}
			return true;
		});
	}

private:
	Move _moveList[MAX_MOVES];
	Move *_end;
};

inline bool moveExists(Board &board, const Move &move) noexcept
{
	const MoveList allMoves(board);

	for (const Move &m : allMoves)
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
