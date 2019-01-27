#pragma once

#include <atomic>
#include <functional>
#include <thread>
#include <vector>

#include "memory/StackVector.h"
#include "data/Game.h"
#include "data/Player.h"
#include "data/pieces/Piece.h"
#include "minimax/HashTable.h"

class Cache;

class BoardManager final
{
public:
	using PieceChangeListener = std::function<void(GameState state, bool shouldRedraw, const StackVector<PosPair, 2> &moved)>;

private:
	inline static std::thread *m_WorkerThread = nullptr;
	static PieceChangeListener m_Listener;
	static Board m_Board;
	static std::vector<PosPair> movesHistory;

public:
	static HashTable<Cache> cacheTable;
	inline static bool isPlayerWhite = true;
	static std::atomic_size_t boardsEvaluated;

	static void initBoardManager(const PieceChangeListener& listener);
	static void loadGame(const Board &board);
	static void loadGame(const std::vector<PosPair> &moves);

	static Board &getBoard() { return m_Board; }
	static const auto &getMovesHistory() { return movesHistory; }
	static bool isWorking() { return m_WorkerThread != nullptr; }
	static Piece::MovesReturnType getPossibleMoves(const Pos &selectedPos);
	static void movePiece(const Pos &selectedPos, const Pos &destPos, bool movedByPlayer = true);
	static void movePieceInternal(const Pos &selectedPos, const Pos &destPos, Board &board, bool checkValid = true);

private:
	static void moveComputerPlayer();
	static bool movePawn(Piece &selectedPiece, const Pos &destPos);
	static PosPair moveKing(Piece &king, const Pos &selectedPos, const Pos &destPos, Board &board);
};
