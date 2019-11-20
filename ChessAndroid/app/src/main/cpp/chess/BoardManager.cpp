#include "BoardManager.h"

#include <cassert>

#include "Stats.h"
#include "data/Board.h"
#include "algorithm/Evaluation.h"
#include "algorithm/Hash.h"
#include "algorithm/NegaMax.h"
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
	s_Listener = listener;

	s_MovesHistory.clear();
	s_MovesHistory.reserve(200);
	s_MovesHistory.emplace_back(Pos(), Pos(), s_Board);

	Stats::resetStats();

	s_IsPlayerWhite = isPlayerWhite;

	if (!isPlayerWhite)
	    s_WorkerThread = std::thread(moveComputerPlayer, s_Settings);
}

void BoardManager::loadGame(const std::vector<PosPair> &moves, const bool isPlayerWhite)
{
	s_IsPlayerWhite = isPlayerWhite;

	s_Board.initDefaultBoard();

	s_MovesHistory.emplace_back(Pos(), Pos(), s_Board);

	for (const PosPair &move : moves)
	{
		s_Board.doMove(move.first.toSquare(), move.second.toSquare());
		s_Board.score = Evaluation::evaluate(s_Board);
		s_MovesHistory.emplace_back(move.first, move.second, s_Board);
	}

	s_Listener(s_Board.state, true, {});
}

Piece::MaxMovesVector BoardManager::getPossibleMoves(const Pos &selectedPos)
{
	Piece::MaxMovesVector moves;

	const Piece &piece = s_Board[selectedPos];
	const auto possibleMoves = piece.getPossibleMoves(selectedPos.toSquare(), s_Board);

	for (const Pos &destPos : possibleMoves)
	{
		const Piece &destPiece = s_Board[destPos];
		if (destPiece.type == PieceType::KING)
			continue;

		Board board = s_Board;
		board.doMove(selectedPos.toSquare(), destPos.toSquare());

		if (board.state == State::INVALID)
			continue;
		if (piece.isWhite && (board.state == State::WHITE_IN_CHECK || board.state == State::WINNER_BLACK))
			continue;
		if (!piece.isWhite && (board.state == State::BLACK_IN_CHECK || board.state == State::WINNER_WHITE))
			continue;

		int count = 1;

		for (const auto &game : BoardManager::getMovesHistory())
		{
			if (board.colorToMove == game.board.colorToMove &&
				board.state == game.board.state &&
				board.zKey == game.board.zKey)
				count++;

			// TODO: Fix this
			if (count == 3)
			{
				board.score = 0;
				board.state = State::DRAW;
				break;
			}
		}

		moves.emplace_back(destPos);
	}

	return moves;
}

void BoardManager::movePiece(const Pos &selectedPos, const Pos &destPos, const bool movedByPlayer)
{
	assert(selectedPos.isValid() && destPos.isValid());

	const byte castledBefore = (s_Board.castlingRights & CASTLED_WHITE) | (s_Board.castlingRights & CASTLED_BLACK);
	s_Board.doMove(selectedPos.toSquare(), destPos.toSquare());
	const byte castledAfter = (s_Board.castlingRights & CASTLED_WHITE) | (s_Board.castlingRights & CASTLED_BLACK);

	s_Board.score = Evaluation::evaluate(s_Board);
	s_Board.zKey = Hash::compute(s_Board);

	const StackVector<PosPair, 2> piecesMoved{ {selectedPos, destPos} };
	const bool shouldRedraw = s_Board.isPromotion || (castledBefore != castledAfter);

	s_MovesHistory.emplace_back(selectedPos, destPos, s_Board);
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

	assert(s_Board.colorToMove != toColor(s_IsPlayerWhite));
	const RootMove bestMove = NegaMax::findBestMove(s_Board, settings);

	Stats::stopTimer();
	s_IsWorking = false;
	movePiece(bestMove.start, bestMove.dest, false);

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
			{ { engineMove.dest, engineMove.start }, { playerMove.dest, playerMove.start } });

	// Remove the last two moves from the vector
	s_MovesHistory.pop_back();
	s_MovesHistory.pop_back();
}
