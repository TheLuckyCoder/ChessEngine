#include "BoardManager.h"

#include <iostream>

#include "algorithm/Evaluation.h"
#include "MoveGen.h"
#include "algorithm/Search.h"

SearchOptions BoardManager::_searchOptions;
BoardManager::BoardChangedCallback BoardManager::_callback;
Board BoardManager::_currentBoard;
UndoRedo::MovesStack BoardManager::_movesStack;

void BoardManager::initBoardManager(const BoardChangedCallback &callback, const bool isPlayerWhite)
{
	Attacks::init();

	_isPlayerWhite = isPlayerWhite;
	_currentBoard.setToStartPos();
	_movesStack = { _currentBoard };
	_callback = callback;

	_callback(GameState::NONE);

	if (!isPlayerWhite)
		makeEngineMove();
}

bool BoardManager::loadGame(bool isPlayerWhite, const std::string &fen)
{
	Search::clearAll();
	Board tempBoard;

	if (tempBoard.setToFen(fen))
	{
		_isPlayerWhite = isPlayerWhite;
		_currentBoard = tempBoard;
		_movesStack = { _currentBoard };

		_callback(getBoardState());

		if (isPlayerWhite != _currentBoard.colorToMove)
			makeEngineMove();

		return true;
	}

	return false;
}

void BoardManager::loadGame(bool isPlayerWhite, const std::vector<Move> &moves)
{
	Search::clearAll();
	assert(moves.size() < MAX_MOVES);

	_isPlayerWhite = isPlayerWhite;
	_currentBoard.setToStartPos();
	_movesStack = { _currentBoard };

	for (const Move &move : moves)
	{
		if (move.empty() || !moveExists(_currentBoard, move) || !_currentBoard.isMoveLegal(move))
			break;
		_currentBoard.makeMove(move);
		_movesStack.push(_currentBoard, move);
	}

	_callback(getBoardState());

	if (isPlayerWhite != _currentBoard.colorToMove)
		makeEngineMove();
}

void BoardManager::makeMove(const Move move)
{
	_currentBoard.makeMove(move);
	_movesStack.push(_currentBoard, move);
	_callback(getBoardState());
}

void BoardManager::makeEngineMove()
{
	if (const auto state = getBoardState();
		!(state == GameState::NONE || state == GameState::WHITE_IN_CHECK || state == GameState::BLACK_IN_CHECK))
		return;

	if (_isWorking) return;
	_isWorking = true;

	const auto tempBoard = _currentBoard;
	const auto settings = _searchOptions;

	std::thread([tempBoard, settings]
				{
					const Move bestMove = Search::findBestMove(tempBoard, settings);
					_isWorking = false;

					makeMove(bestMove);
				}).detach();
}

void BoardManager::undoLastMoves()
{
	if (!_movesStack.undo().empty())
		_currentBoard.undoMove();
	if (_movesStack.peek().colorToMove() != isPlayerWhite() && !_movesStack.undo().empty())
		_currentBoard.undoMove();

	_callback(getBoardState());
}

void BoardManager::redoLastMoves()
{
	if (const Move move = _movesStack.redo(); !move.empty())
		_currentBoard.makeMove(move);
	if (_movesStack.peek().colorToMove() != isPlayerWhite())
	{
		const Move move = _movesStack.redo();
		if (!move.empty())
			_currentBoard.makeMove(move);
		else
			makeEngineMove(); // If there was no move to redo, try to make one
	}

	_callback(getBoardState());
}

std::vector<Move> BoardManager::getPossibleMoves(const Square from)
{
	std::vector<Move> moves;
	moves.reserve(27);

	Board tempBoard = _currentBoard;
	const MoveList allMoves(tempBoard);

	for (const Move &move : allMoves)
	{
		if (move.from() == from && tempBoard.isMoveLegal(move))
			moves.push_back(move);
	}

	return moves;
}

GameState BoardManager::getBoardState()
{
	if (_currentBoard.isDrawn())
		return GameState::DRAW;

	const Color colorToMove = _currentBoard.colorToMove;

	const bool otherInCheck =
		(_currentBoard.generateAttackers(_currentBoard.getKingSq(colorToMove)) &
		 _currentBoard.getPieces(~colorToMove)).notEmpty();

	if (_currentBoard.isSideInCheck() && otherInCheck)
		return GameState::INVALID;

	MoveList moveList(_currentBoard);
	moveList.keepLegalMoves();

	if (moveList.empty())
	{
		const auto winner = (colorToMove == WHITE ? GameState::WINNER_BLACK : GameState::WINNER_WHITE);
		return (_currentBoard.isSideInCheck() ? winner : GameState::DRAW);
	}

	if (_currentBoard.isSideInCheck())
		return colorToMove == WHITE ? GameState::WHITE_IN_CHECK : GameState::BLACK_IN_CHECK;

	return GameState::NONE;
}
