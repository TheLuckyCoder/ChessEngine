#include "BoardManager.h"

#include <iostream>

#include "algorithm/Evaluation.h"
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

	_board.setToStartPos();
	_listener = listener;

	_isPlayerWhite = isPlayerWhite;

	if (!isPlayerWhite)
		_workerThread = std::thread(moveComputerPlayer, _settings);
}

bool BoardManager::loadGame(const bool isPlayerWhite, const std::string &fen)
{
	Board tempBoard = _board;

	if (tempBoard.setToFen(fen))
	{
		_isPlayerWhite = isPlayerWhite;
		_board = tempBoard;
		_listener(getBoardState(), true, {});
		return true;
	}

	return false;
}

void BoardManager::loadGame(const std::vector<Move> &moves, const bool isPlayerWhite)
{
	_isPlayerWhite = isPlayerWhite;

	_board.setToStartPos();

	assert(moves.size() < MAX_MOVES);

	for (const Move &move : moves)
	{
		if (move.empty() || !moveExists(_board, move) || !_board.makeMove(move))
			break;
	}

	_listener(getBoardState(), true, {});
}

std::string BoardManager::exportFen()
{
	return _board.getFen();
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
	const bool shouldRedraw = flags.promotion() | flags.enPassant();
	const GameState state = getBoardState();

	std::cout << "Made the Move: " << move.toString()
			  << "; Evaluated at: " << Evaluation::value(_board) << std::endl;

	std::vector<std::pair<byte, byte>> movedVec;
	movedVec.reserve(2);
	movedVec.emplace_back(move.from(), move.to());

	// Animate Castling
	if (flags.kSideCastle())
	{
		switch (move.to())
		{
			case SQ_G1:
				movedVec.emplace_back(SQ_H1, SQ_F1);
				break;
			case SQ_G8:
				movedVec.emplace_back(SQ_H8, SQ_F8);
				break;
		}

	} else if (flags.qSideCastle())
	{
		switch (move.to())
		{
			case SQ_C1:
				movedVec.emplace_back(SQ_A1, SQ_D1);
				break;
			case SQ_C8:
				movedVec.emplace_back(SQ_A8, SQ_D8);
				break;
		}
	}

	_listener(state, shouldRedraw, movedVec);

	if (movedByPlayer &&
		(state == GameState::NONE || state == GameState::WHITE_IN_CHECK || state == GameState::BLACK_IN_CHECK))
		_workerThread = std::thread(moveComputerPlayer, _settings);
}

void BoardManager::forceMove()
{
	if (!_isWorking)
	{
		_isWorking = true;
		_workerThread = std::thread(moveComputerPlayer, _settings);
	}
}

// This function should only be called through the Worker Thread
void BoardManager::moveComputerPlayer(const Settings &settings)
{
	_isWorking = true;
	const Move bestMove = Search::findBestMove(_board, settings);
	_isWorking = false;

	makeMove(bestMove, false);

	_workerThread.detach();
}

bool BoardManager::undoLastMoves()
{
	if (isWorking() || _board.historyPly < 2) return false;

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

GameState BoardManager::getBoardState()
{
	if (_board.isDrawn())
		return GameState::DRAW;

	const bool whiteInCheck = _board.allKingAttackers<WHITE>();
	const bool blackInCheck = _board.allKingAttackers<BLACK>();

	if (whiteInCheck && blackInCheck)
		return GameState::INVALID;

	auto state = GameState::NONE;

	if (whiteInCheck)
		state = GameState::WHITE_IN_CHECK;
	else if (blackInCheck)
		state = GameState::BLACK_IN_CHECK;

	MoveList moveList(_board);
	moveList.keepLegalMoves();

	if (moveList.empty())
	{
		if (_board.colorToMove)
			state = whiteInCheck ? GameState::WINNER_BLACK : GameState::DRAW;
		else
			state = blackInCheck ? GameState::WINNER_WHITE : GameState::DRAW;
	}

	return state;
}
