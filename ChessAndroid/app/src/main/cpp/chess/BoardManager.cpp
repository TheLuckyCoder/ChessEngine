#include "BoardManager.h"

#include "Stats.h"
#include "data/Board.h"
#include "algorithm/Hash.h"
#include "algorithm/Search.h"
#include "algorithm/PieceAttacks.h"

Settings BoardManager::s_Settings(4u, std::thread::hardware_concurrency() - 1u, 100, true);
BoardManager::PieceChangeListener BoardManager::s_Listener;
Board BoardManager::s_Board;
std::vector<RootMove> BoardManager::s_MovesHistory;

void BoardManager::initBoardManager(const PieceChangeListener &listener, const bool isPlayerWhite)
{
    Hash::init();
	PieceAttacks::init();

	s_Board.initDefaultBoard();
	//s_Board.setToFen("r1b1k2r/ppppnppp/2n2q2/2b5/3NP3/2P1B3/PP3PPP/RN1QKB1R w KQkq - 0 1");
	s_Listener = listener;

	s_MovesHistory.clear();
	s_MovesHistory.reserve(200);
	s_MovesHistory.emplace_back(64u, 64u, s_Board);

	Stats::resetStats();

	s_IsPlayerWhite = isPlayerWhite;

	if (!isPlayerWhite)
	    s_WorkerThread = std::thread(moveComputerPlayer, s_Settings);
}

void BoardManager::loadGame(const std::vector<std::pair<byte, byte>> &moves, const bool isPlayerWhite)
{
	s_IsPlayerWhite = isPlayerWhite;

	s_Board.initDefaultBoard();

	s_MovesHistory.clear();
	s_MovesHistory.emplace_back(64u, 64u, s_Board);

    try {
        for (const auto &move : moves)
        {
			s_Board.doMove(move.first, move.second);
			s_Board.score = Evaluation::evaluate(s_Board);
			s_MovesHistory.emplace_back(move.first, move.second, s_Board);
        }
    } catch (...) {
        // Couldn't load all moves correctly, fallback to the original board
        s_Board.initDefaultBoard();
        s_MovesHistory.clear();
		s_MovesHistory.emplace_back(64u, 64u, s_Board);
	}

	s_Listener(s_Board.state, true, {});
}

std::vector<Pos> BoardManager::getPossibleMoves(const Pos &selectedPos)
{
	std::vector<Pos> moves;
	moves.reserve(27);

	const byte startSq = selectedPos.toSquare();
	const Piece &piece = s_Board.getPiece(startSq);
	U64 possibleMoves = piece.getPossibleMoves(startSq, s_Board);

	// Make sure we are not capturing the king
	possibleMoves &= ~s_Board.getType(s_Board.colorToMove, KING);

	while (possibleMoves)
	{
		const byte destSq = Bitboard::findNextSquare(possibleMoves);

		Board board = s_Board;
		board.doMove(startSq, destSq);

		if (!board.hasValidState())
			continue;

		moves.emplace_back(Pos(destSq));
	}

	return moves;
}

void BoardManager::movePiece(const byte startSq, const byte destSq, const bool movedByPlayer)
{
	assert(startSq != destSq);
	assert(startSq < 64 && destSq < 64);

	const byte castledBefore = (s_Board.castlingRights & CASTLED_WHITE) | (s_Board.castlingRights & CASTLED_BLACK);
	s_Board.doMove(startSq, destSq);
	assert(s_Board.hasValidState());
	const byte castledAfter = (s_Board.castlingRights & CASTLED_WHITE) | (s_Board.castlingRights & CASTLED_BLACK);

	s_Board.score = Evaluation::evaluate(s_Board);
	s_Board.zKey = Hash::compute(s_Board);

	const std::vector piecesMoved{ std::make_pair(startSq, destSq) };
	const bool shouldRedraw = s_Board.isPromotion || (castledBefore != castledAfter);

	s_MovesHistory.emplace_back(startSq, destSq, s_Board);
	s_Listener(s_Board.state, shouldRedraw, piecesMoved);

	if (movedByPlayer && (s_Board.state == State::NONE || s_Board.state == State::WHITE_IN_CHECK || s_Board.state == State::BLACK_IN_CHECK))
		s_WorkerThread = std::thread(moveComputerPlayer, s_Settings);
}

// This function should only be called through the Worker Thread
void BoardManager::moveComputerPlayer(const Settings &settings)
{
    s_IsWorking = true;
	Stats::resetStats();
	Stats::startTimer();

	const RootMove bestMove = Search::findBestMove(s_Board, settings);

	Stats::stopTimer();
	movePiece(bestMove.startSq, bestMove.destSq, false);

	s_IsWorking = false;
	s_WorkerThread.detach();
}

void BoardManager::undoLastMoves()
{
	if (isWorking() || s_MovesHistory.size() < 3) return;

	const auto end = s_MovesHistory.end();
	// Undo the last move, which should have been made by the engine
	const RootMove &engineMove = s_MovesHistory.back();
	const Board &engineBoard = engineMove.board;

	// Undo the move before the last move so that it is the player's turn again
	const RootMove &playerMove = *(end - 2);
	const Board &playerBoard = playerMove.board;

	// Restore the move before the last two moves
	const RootMove &previousMove = *(end - 3);
	const Board &previousBoard = previousMove.board;

	// Redraw if a Promotion or castling happened in the last three moves
	const bool shouldRedraw = engineBoard.isPromotion || engineBoard.isCapture ||
			playerBoard.isPromotion || playerBoard.isCapture ||
			engineBoard.isCastled(WHITE) != previousBoard.isCastled(WHITE) ||
			engineBoard.isCastled(BLACK) != previousBoard.isCastled(BLACK);

	s_Board = previousBoard;
	s_Listener(previousBoard.state, shouldRedraw,
			{ { engineMove.destSq, engineMove.startSq }, { playerMove.destSq, playerMove.startSq } });

	// Remove the last two moves from the vector
	s_MovesHistory.pop_back();
	s_MovesHistory.pop_back();
}
