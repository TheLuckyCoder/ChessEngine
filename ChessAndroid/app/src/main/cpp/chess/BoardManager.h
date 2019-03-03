#pragma once

#include <functional>
#include <thread>
#include <vector>

#include "Settings.h"
#include "containers/StackVector.h"
#include "containers/TranspositionTable.h"
#include "data/Enums.h"
#include "data/Player.h"
#include "data/Piece.h"

class BoardManager final
{
public:
	using PieceChangeListener = std::function<void(State state, bool shouldRedraw, const StackVector<PosPair, 2> &moved)>;

private:
	inline static Settings m_Settings = Settings(4u, std::thread::hardware_concurrency() - 1u);
	inline static std::thread m_WorkerThread;
	inline static bool m_IsWorking = false;
	static PieceChangeListener m_Listener;
	static Board m_Board;
	static std::vector<PosPair> m_MovesHistory;

public:
    inline static TranspositionTable<SearchCache> searchCache{ 16777215 };
    inline static TranspositionTable<EvaluationCache> evaluationCache{ 16777215 };
	inline static bool isPlayerWhite = true;

	static void initBoardManager(const PieceChangeListener &listener);
	static void loadGame(std::vector<PosPair> &&moves);

	static Board &getBoard() { return m_Board; }
	static const auto &getMovesHistory() { return m_MovesHistory; }
	static bool isWorking() { return m_IsWorking; }
	static Piece::MaxMovesVector getPossibleMoves(const Pos &selectedPos);
	static void movePiece(const Pos &selectedPos, const Pos &destPos, bool movedByPlayer = true);
	static void movePieceInternal(const Pos &selectedPos, const Pos &destPos, Board &board, bool checkValid = true);
	static void setSettings(const Settings &settings) { m_Settings = settings; }

private:
	static void moveComputerPlayer(const Settings &settings);
	static bool movePawn(Piece &selectedPiece, const Pos &destPos);
	static PosPair moveKing(Piece &king, const Pos &selectedPos, const Pos &destPos, Board &board);
};
