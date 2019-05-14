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

class BoardManager final
{
public:
	using PieceChangeListener = std::function<void(State state, bool shouldRedraw, const StackVector<PosPair, 2> &moved)>;

private:
	static Settings m_Settings;
	inline static std::thread m_WorkerThread;
	inline static std::atomic_bool m_IsWorking{ false };
	inline static bool m_IsPlayerWhite;
	static PieceChangeListener m_Listener;
	static Board m_Board;
	static std::vector<RootMove> m_MovesHistory;
	
public:
	static void initBoardManager(const PieceChangeListener &listener, bool isPlayerWhite = true);
	static void loadGame(const std::vector<PosPair> &moves);
	static void undoLastMoves();

	static Board &getBoard() { return m_Board; }
	static const auto &getMovesHistory() { return m_MovesHistory; }
	static bool isWorking() { return m_IsWorking; }
	static bool isPlayerWhite() { return m_IsPlayerWhite; }
	static Piece::MaxMovesVector getPossibleMoves(const Pos &selectedPos);
	static void movePiece(const Pos &selectedPos, const Pos &destPos, bool movedByPlayer = true);
	static void movePieceInternal(const Pos &selectedPos, const Pos &destPos, Board &board, bool updateState = true);
	static void setSettings(const Settings &settings) { m_Settings = settings; }

private:
	static void moveComputerPlayer(const Settings &settings);
	static bool movePawn(Board &board, const Pos &startPos, const Pos &destPos, const Pos &enPassantPos);
	static PosPair moveKing(Piece &king, const Pos &selectedPos, const Pos &destPos, Board &board);
};
