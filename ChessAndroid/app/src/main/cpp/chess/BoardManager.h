#pragma once

#include <functional>
#include <thread>

#include "GameState.h"
#include "data/Pos.h"
#include "data/Player.h"

class BoardManager
{
public:
	using PieceChangeListener = std::function<void(GameState state, bool shouldRedraw, const std::vector<PosPair> &moved)>;
private:
	static BoardManager &m_Instance;
	static std::thread *m_WorkerThread;

	Board m_Board{};
	PieceChangeListener m_Listener;

    static bool whitePlayersTurn;

public:
	static bool isWhiteAtBottom;

	static void initBoardManager(const PieceChangeListener& listener);
	static void loadJsonGame(Board &&board);
	static Board &getBoard();
	static bool isWorking();
	
	static GameState movePieceInternal(const Pos &selectedPos, const Pos &destPos, Board &board);
	static void movePiece(const Pos &selectedPos, const Pos &destPos);

private:
	static void moveComputerPlayer();
	static bool movePawn(Piece &selectedPiece, const Pos &destPos);
	static PosPair moveKing(Piece &king, Pos selectedPos, const Pos &destPos, Board &board);
};
