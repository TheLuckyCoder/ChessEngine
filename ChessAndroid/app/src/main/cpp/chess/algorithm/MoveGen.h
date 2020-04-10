#pragma once

#include "../data/Move.h"
#include "../data/Board.h"

enum GenType : byte
{
	ALL,
	CAPTURES
};

template <GenType T>
Move *generateMoves(const Board &board, Move *moveList) noexcept;

template <GenType T>
class MoveList
{
public:
	explicit MoveList(const Board& board) : _last(generateMoves<T>(board, _moveList)) {}
	
	constexpr const Move *begin() const noexcept { return _moveList; }
	constexpr const Move *end() const noexcept { return _last; }

	constexpr Move *begin() noexcept { return _moveList; }
	constexpr Move *end() noexcept { return _last; }
	
	constexpr size_t size() const noexcept { return _last - _moveList; }
	constexpr bool empty() const noexcept { return (_last - _moveList) == 0u; }

private:
	Move *_end;
	Move _moveList[MAX_MOVES];
};

inline bool moveExists(Board &board, const Move &move)
{
	const MoveList<ALL> allMoves(board);

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
