#include "ComputerPlayer.h"

#include "../../BoardManager.h"
#include "MiniMax.h"

std::pair<Pos, Pos> ComputerPlayer::getNextMove() const
{
	constexpr int depth = 13;
	return BoardManager::isWhiteAtBottom ?
		MiniMax::MinMove(BoardManager::getBoard(), depth) : MiniMax::MaxMove(BoardManager::getBoard(), depth);
}
