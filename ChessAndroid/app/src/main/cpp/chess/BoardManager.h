#pragma once

#include <atomic>
#include <functional>
#include <thread>
#include <vector>

#include "Settings.h"
#include "containers/StackVector.h"
#include "data/Enums.h"
#include "data/Piece.h"

class RootMove;
class Board;

class BoardManager final
{
public:
	using PieceChangeListener = std::function<void(State state, bool shouldRedraw, const std::vector<std::pair<byte, byte>> &moved)>;

private:
	static Settings s_Settings;
	inline static std::thread s_WorkerThread;
	inline static std::atomic_bool s_IsWorking{ false };
	inline static bool s_IsPlayerWhite;
	static PieceChangeListener s_Listener;
	static Board s_Board;
	static std::vector<RootMove> s_MovesHistory;
	
public:
	static void initBoardManager(const PieceChangeListener &listener, bool isPlayerWhite = true);
	static void loadGame(const std::vector<std::pair<byte, byte>> &moves, bool isPlayerWhite);
	static void undoLastMoves();

	static Board &getBoard() { return s_Board; }
	static const auto &getMovesHistory() { return s_MovesHistory; }
	static bool isWorking() { return s_IsWorking; }
	static bool isPlayerWhite() { return s_IsPlayerWhite; }
	static StackVector<Pos, 27> getPossibleMoves(const Pos &selectedPos);
	static void movePiece(byte startSq, byte destSq, bool movedByPlayer = true);
	static void setSettings(const Settings &settings) { s_Settings = settings; }

private:
	static void moveComputerPlayer(const Settings &settings);
};
