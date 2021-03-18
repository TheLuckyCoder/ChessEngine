#include "BoardManager.h"

#include <iostream>

#include "algorithm/Evaluation.h"
#include "algorithm/MoveGen.h"
#include "algorithm/Search.h"

SearchOptions BoardManager::_searchOptions;
BoardManager::BoardChangedCallback BoardManager::_callback;
Board BoardManager::_currentBoard;
UndoRedo::HistoryStack BoardManager::_undoRedoStack;

void BoardManager::initBoardManager(const BoardChangedCallback &callback, const bool isPlayerWhite)
{
	Attacks::init();

	_isPlayerWhite = isPlayerWhite;
	_currentBoard.setToStartPos();
	_undoRedoStack = { _currentBoard };
	_callback = callback;

	_callback(GameState::NONE);

	if (!isPlayerWhite)
		makeEngineMove();
}

bool BoardManager::loadGame(const std::string &fen, bool isPlayerWhite)
{
	Search::clearAll();
	Board tempBoard;

	if (tempBoard.setToFen(fen))
	{
		_isPlayerWhite = isPlayerWhite;
		_currentBoard = tempBoard;
		_undoRedoStack = { _currentBoard };

		_callback(getBoardState());

		if (isPlayerWhite != _currentBoard.colorToMove)
			makeEngineMove();

		return true;
	}

	return false;
}

void BoardManager::loadGame(const std::vector<Move> &moves, const bool isPlayerWhite)
{
	Search::clearAll();
	assert(moves.size() < MAX_MOVES);

	_isPlayerWhite = isPlayerWhite;
	_currentBoard.setToStartPos();
	_undoRedoStack = { _currentBoard };

	for (const Move &move : moves)
	{
		if (move.empty() || !moveExists(_currentBoard, move) || !_currentBoard.makeMove(move))
			break;
		_undoRedoStack.push(_currentBoard, move);
	}

	_callback(getBoardState());

	if (isPlayerWhite != _currentBoard.colorToMove)
		makeEngineMove();
}

void BoardManager::makeMove(const Move move)
{
	_currentBoard.makeMove(move);
	_undoRedoStack.push(_currentBoard, move);
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
	if (!_undoRedoStack.undo().empty())
		_currentBoard.undoMove();
	if (_undoRedoStack.peek().colorToMove() != isPlayerWhite() && !_undoRedoStack.undo().empty())
		_currentBoard.undoMove();

	_callback(getBoardState());
}

void BoardManager::redoLastMoves()
{
	if (const Move move = _undoRedoStack.redo(); !move.empty())
		_currentBoard.makeMove(move);
	if (_undoRedoStack.peek().colorToMove() != isPlayerWhite())
	{
		const Move move = _undoRedoStack.redo();
		if (!move.empty())
			_currentBoard.makeMove(move);
		else
			makeEngineMove(); // If there was no move to redo, try to make one
	}

	_callback(getBoardState());
}

std::vector<Move> BoardManager::getMovesHistory()
{
	std::vector<Move> moves;
	moves.reserve(_undoRedoStack.size());

	std::transform(_undoRedoStack.begin(), _undoRedoStack.end(), std::back_inserter(moves),
				   [](const auto &history) { return history.getMove(); });

	return moves;
}

std::vector<Move> BoardManager::getPossibleMoves(const Square from)
{
	std::vector<Move> moves;
	moves.reserve(27);

	Board tempBoard = _currentBoard;
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

GameState BoardManager::getBoardState()
{
	if (_currentBoard.isDrawn())
		return GameState::DRAW;

	const bool whiteInCheck = bool(_currentBoard.allKingAttackers<WHITE>());
	const bool blackInCheck = bool(_currentBoard.allKingAttackers<BLACK>());

	if (whiteInCheck && blackInCheck)
		return GameState::INVALID;

	auto state = GameState::NONE;

	if (whiteInCheck)
		state = GameState::WHITE_IN_CHECK;
	else if (blackInCheck)
		state = GameState::BLACK_IN_CHECK;

	MoveList moveList(_currentBoard);
	moveList.keepLegalMoves();

	if (moveList.empty())
	{
		if (_currentBoard.colorToMove)
			state = whiteInCheck ? GameState::WINNER_BLACK : GameState::DRAW;
		else
			state = blackInCheck ? GameState::WINNER_WHITE : GameState::DRAW;
	}

	return state;
}
