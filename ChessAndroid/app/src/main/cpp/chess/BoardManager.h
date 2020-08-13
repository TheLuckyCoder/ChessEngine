#pragma once

#include <atomic>
#include <functional>
#include <thread>
#include <vector>

#include "Settings.h"
#include "Board.h"

enum class GameState : byte
{
	NONE,
	WINNER_WHITE,
	WINNER_BLACK,
	DRAW,
	WHITE_IN_CHECK,
	BLACK_IN_CHECK,
	INVALID = 10
};

class BoardManager final
{
public:
	using PieceChangeListener = std::function<void(GameState state, bool shouldRedraw, const std::vector<std::pair<byte, byte>> &moved)>;

private:
	static Settings _settings;
	inline static std::thread _workerThread;
	inline static std::atomic_bool _isWorking{ false };
	inline static bool _isPlayerWhite;
	static PieceChangeListener _listener;
	static Board _board;
	
public:
	static void initBoardManager(const PieceChangeListener &listener, bool isPlayerWhite = true);
	static bool loadGame(bool isPlayerWhite, const std::string &fen);
	static void loadGame(const std::vector<Move> &moves, bool isPlayerWhite);
	static std::string exportFen();
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
	static GameState getBoardState();
};
