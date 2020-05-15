#include "BoardManager.h"

#include <iostream>

#include "algorithm/Evaluation.h"
#include "Stats.h"
#include "data/Board.h"
#include "algorithm/Hash.h"
#include "algorithm/MoveGen.h"
#include "algorithm/Search.h"
#include "algorithm/Attacks.h"

Settings BoardManager::_settings(6u, std::thread::hardware_concurrency() - 1u, 64, true);
BoardManager::PieceChangeListener BoardManager::_listener;
Board BoardManager::_board;

void BoardManager::initBoardManager(const PieceChangeListener &listener, const bool isPlayerWhite)
{
	Hash::init();
	Attacks::init();

	_board.initDefaultBoard();
	_listener = listener;

	Stats::resetStats();

	_isPlayerWhite = isPlayerWhite;

	if (!isPlayerWhite)
		_workerThread = std::thread(moveComputerPlayer, _settings);
}

void BoardManager::loadGame(const std::string &fen)
{
	_board.setToFen(fen);
	_listener(getBoardState(), true, {});
}

void BoardManager::loadGame(const std::vector<Move> &moves, const bool isPlayerWhite)
{
	_isPlayerWhite = isPlayerWhite;

	_board.initDefaultBoard();

	assert(moves.size() < MAX_MOVES);

	for (const Move &move : moves)
	{
		if (move.empty() || !moveExists(_board, move) || !_board.makeMove(move))
			break;
	}

	_listener(getBoardState(), true, {});
}

std::vector<Move> BoardManager::getMovesHistory()
{
	std::vector<Move> moves(static_cast<size_t>(_board.historyPly));

	for (size_t i{}; i < moves.size(); ++i)
		moves[i] = _board.history[i].getMove();

	return moves;
}

std::vector<Move> BoardManager::getPossibleMoves(const byte from)
{
	std::vector<Move> moves;
	moves.reserve(27);

	Board tempBoard = _board;
	const MoveList allMoves(tempBoard);

	for (const Move &move : allMoves)
	{
		if (move.from() == from && tempBoard.makeMove(move))
		{
			moves.push_back(move);

			tempBoard.undoMove();
		}
	}

	return moves;
}


void BoardManager::makeMove(const Move move, const bool movedByPlayer)
{
	_board.makeMove(move);

	const auto flags = move.flags();
	const bool shouldRedraw = flags & Move::PROMOTION || flags & Move::CASTLE
							  || flags & Move::EN_PASSANT;
	const State state = getBoardState();

	std::cout << "Made the Move: " << move.toString()
			  << "; Evaluated at: " << Evaluation::value(_board) << '\n';
	_listener(state, shouldRedraw, {{ move.from(), move.to() }});

	if (movedByPlayer &&
		(state == State::NONE || state == State::WHITE_IN_CHECK || state == State::BLACK_IN_CHECK))
		_workerThread = std::thread(moveComputerPlayer, _settings);
}

void BoardManager::forceMove()
{
	if (!_isWorking)
		_workerThread = std::thread(moveComputerPlayer, _settings);
}

// This function should only be called through the Worker Thread
void BoardManager::moveComputerPlayer(const Settings &settings)
{
	_isWorking = true;
	Stats::resetStats();
	Stats::startTimer();

	const Move bestMove = Search::findBestMove(_board, settings);

	Stats::stopTimer();
	makeMove(bestMove, false);

	_isWorking = false;
	_workerThread.detach();
}

bool BoardManager::undoLastMoves()
{
	if (isWorking() || _board.historyPly < 3) return false;

	// Undo the last move, which should have been made by the engine
	const UndoMove engineMove = _board.history[_board.historyPly - 1];
	_board.undoMove();

	// Undo the move before the last move so that it is the player's turn again
	const UndoMove playerMove = _board.history[_board.historyPly - 1];
	if (playerMove.getMove().empty())
		_board.undoNullMove();
	else
		_board.undoMove();

	_listener(getBoardState(), true,
			  {{ engineMove.getMove().to(), engineMove.getMove().from() },
			   { playerMove.getMove().to(), playerMove.getMove().from() }});

	return true;
}

State BoardManager::getBoardState()
{
	if (_board.isDrawn())
		return State::DRAW;

	const bool whiteInCheck = _board.allKingAttackers<WHITE>();
	const bool blackInCheck = _board.allKingAttackers<BLACK>();

	if (whiteInCheck && blackInCheck)
		return State::INVALID;

	auto state = State::NONE;

	if (whiteInCheck)
		state = State::WHITE_IN_CHECK;
	else if (blackInCheck)
		state = State::BLACK_IN_CHECK;

	MoveList moveList(_board);
	moveList.keepLegalMoves();

	if (moveList.empty())
	{
		if (_board.colorToMove)
			state = whiteInCheck ? State::WINNER_BLACK : State::DRAW;
		else
			state = blackInCheck ? State::WINNER_WHITE : State::DRAW;
	}

	return state;
}
