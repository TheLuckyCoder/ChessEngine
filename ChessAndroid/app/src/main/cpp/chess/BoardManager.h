#pragma once

#include <atomic>
#include <functional>
#include <thread>
#include <vector>

#include "containers/StackVector.h"
#include "containers/HashTable.h"
#include "data/Enums.h"
#include "data/Player.h"
#include "data/Piece.h"

class Cache;

class BoardManager final
{
public:
	using PieceChangeListener = std::function<void(State state, bool shouldRedraw, const StackVector<PosPair, 2> &moved)>;

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
	static void loadGame(std::vector<PosPair> &&moves);

	static Board &getBoard() { return m_Board; }
	static const auto &getMovesHistory() { return movesHistory; }
	static bool isWorking() { return m_WorkerThread != nullptr; }
	static Piece::MaxMovesVector getPossibleMoves(const Pos &selectedPos);
	static void movePiece(const Pos &selectedPos, const Pos &destPos, bool movedByPlayer = true);
	static void movePieceInternal(const Pos &selectedPos, const Pos &destPos, Board &board, bool checkValid = true);

private:
	static void moveComputerPlayer();
	static bool movePawn(Piece &selectedPiece, const Pos &destPos);
	static PosPair moveKing(Piece &king, const Pos &selectedPos, const Pos &destPos, Board &board);
};
