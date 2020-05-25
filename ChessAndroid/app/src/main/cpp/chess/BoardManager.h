#pragma once

#include <atomic>
#include <functional>
#include <thread>
#include <vector>

#include "Settings.h"
#include "Move.h"
#include "Board.h"

class BoardManager final
{
public:
	using PieceChangeListener = std::function<void(State state, bool shouldRedraw, const std::vector<std::pair<byte, byte>> &moved)>;

private:
	static Settings _settings;
	inline static std::thread _workerThread;
	inline static std::atomic_bool _isWorking{ false };
	inline static bool _isPlayerWhite;
	static PieceChangeListener _listener;
	static Board _board;
	
public:
	static void initBoardManager(const PieceChangeListener &listener, bool isPlayerWhite = true);
	static void loadGame(const std::string &fen);
	static void loadGame(const std::vector<Move> &moves, bool isPlayerWhite);
	static bool undoLastMoves();

	static const Board &getBoard() { return _board; }
	static std::vector<Move> getMovesHistory();
	static bool isWorking() { return _isWorking; }
	static bool isPlayerWhite() { return _isPlayerWhite; }
	static std::vector<Move> getPossibleMoves(byte from);
	static void makeMove(Move move, bool movedByPlayer = true);
	static void setSettings(const Settings &settings) { _settings = settings; }
	static void forceMove();

private:
	static void moveComputerPlayer(const Settings &settings);
	static State getBoardState();
};
