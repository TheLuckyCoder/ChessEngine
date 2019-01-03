#pragma once

#include <functional>
#include <thread>
#include <atomic>

#include "GameState.h"
#include "data/Pos.h"
#include "data/Player.h"
#include "data/minimax/CacheTable.h"

class BoardManager
{
public:
	using PieceChangeListener = std::function<void(GameState state, bool shouldRedraw, const StackVector<PosPair, 2> &moved)>;

private:
	static std::thread *m_WorkerThread;
	static PieceChangeListener m_Listener;
	static Board m_Board;
	static CacheTable cache;
	static std::vector<PosPair> movesHistory;

public:
	static bool isPlayerWhite;
	static std::atomic_uint32_t boardsEvaluated;

	static void initBoardManager(const PieceChangeListener& listener);
	static void loadGame(const Board &board);
	static void loadGame(const std::vector<PosPair> &moves);

	static Board &getBoard() { return m_Board; }
	static const auto &getMovesHistory() { return movesHistory; };
	static bool isWorking() { return m_WorkerThread != nullptr; };
	static Piece::MovesReturnType getPossibleMoves(const Pos &selectedPos);
	static void movePiece(const Pos &selectedPos, const Pos &destPos, bool movedByPlayer = true);
	static GameState movePieceInternal(const Pos &selectedPos, const Pos &destPos, Board &board, bool checkValid = true);

private:
	static void moveComputerPlayer();
	static bool movePawn(Piece &selectedPiece, const Pos &destPos);
	static PosPair moveKing(Piece &king, Pos selectedPos, const Pos &destPos, Board &board);
};
