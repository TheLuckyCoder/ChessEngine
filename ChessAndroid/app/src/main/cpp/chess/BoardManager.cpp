#include "BoardManager.h"

#include <iostream>

#include "algorithm/Evaluation.h"
#include "algorithm/MoveGen.h"
#include "algorithm/Search.h"

SearchOptions BoardManager::_searchOptions;
BoardManager::BoardChangedCallback BoardManager::_callback;
Board BoardManager::_board;
std::vector<BoardManager::IndexedPiece> BoardManager::_indexedPieces;

void BoardManager::initBoardManager(const BoardChangedCallback &callback, const bool isPlayerWhite)
{
	Attacks::init();

	_isPlayerWhite = isPlayerWhite;
	_board.setToStartPos();
	generatedIndexedPieces();
	_callback = callback;

	if (!isPlayerWhite)
		makeEngineMove();
}

bool BoardManager::loadGame(const std::string &fen, bool isPlayerWhite)
{
	Board tempBoard;

	if (tempBoard.setToFen(fen))
	{
		_isPlayerWhite = isPlayerWhite;
		_board = tempBoard;
		generatedIndexedPieces();

		_callback(getBoardState());

		if (isPlayerWhite != _board.colorToMove)
			makeEngineMove();

		return true;
	}

	return false;
}

void BoardManager::loadGame(const std::vector<Move> &moves, const bool isPlayerWhite)
{
	assert(moves.size() < MAX_MOVES);

	_isPlayerWhite = isPlayerWhite;
	_board.setToStartPos();
	generatedIndexedPieces();

	for (const Move &move : moves)
	{
		if (move.empty() || !moveExists(_board, move) || !_board.makeMove(move))
			break;
		updateIndexedPieces(move);
	}

	_callback(getBoardState());

	if (isPlayerWhite != _board.colorToMove)
		makeEngineMove();
}

void BoardManager::makeMove(const Move move)
{
	_board.makeMove(move);
	updateIndexedPieces(move);
	_callback(getBoardState());
}

void BoardManager::makeEngineMove()
{
	if (const auto state = getBoardState();
		!(state == GameState::NONE || state == GameState::WHITE_IN_CHECK || state == GameState::BLACK_IN_CHECK))
		return;

	if (_isWorking) return;
	_isWorking = true;

	const auto tempBoard = _board;
	const auto settings = _searchOptions;

	std::thread([tempBoard, settings]
				{
					const Move bestMove = Search::findBestMove(tempBoard, settings);
					_isWorking = false;

					makeMove(bestMove);
				}).detach();
}

bool BoardManager::undoLastMoves()
{
	return false;
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

	_callback(getBoardState());

	return true;
}

bool BoardManager::redoLastMoves()
{
	return false;
}

std::vector<Move> BoardManager::getMovesHistory()
{
	std::vector<Move> moves(static_cast<size_t>(_board.historyPly));

	for (size_t i{}; i < moves.size(); ++i)
		moves[i] = _board.history[i].getMove();

	return moves;
}

std::vector<Move> BoardManager::getPossibleMoves(const Square from)
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

GameState BoardManager::getBoardState()
{
	if (_board.isDrawn())
		return GameState::DRAW;

	const bool whiteInCheck = bool(_board.allKingAttackers<WHITE>());
	const bool blackInCheck = bool(_board.allKingAttackers<BLACK>());

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

void BoardManager::generatedIndexedPieces()
{
	std::vector<IndexedPiece> indexedPieces;
	indexedPieces.reserve(32);

	for (u8 square{}; square < SQUARE_NB; ++square)
	{
		auto &&piece = _board.data[square];
		if (piece.isValid())
			indexedPieces.emplace_back(i32(square), toSquare(square), piece.color(), piece.type());
	}

	_indexedPieces = std::move(indexedPieces);
}

void BoardManager::updateIndexedPieces(const Move move)
{
	const auto findPiece = [&](const Square square)
	{
		const auto it = std::find_if(_indexedPieces.begin(), _indexedPieces.end(), [&square](const IndexedPiece &piece)
		{
			return piece.square == square;
		});
		assert(it != _indexedPieces.end());
		return it;
	};

	const auto movePiece = [&](const Square from, const Square to) { findPiece(from)->square = to; };
	const auto promotePawn = [&](const Square square, const PieceType to) { findPiece(square)->pieceType = to; };
	const auto removePiece = [&](const Square square) { _indexedPieces.erase(findPiece(square)); };

	const Square from = move.from();
	const Square to = move.to();
	const Color side = _board.colorToMove;
	const auto flags = move.flags();

	// Handle en passant capture and castling
	if (flags.enPassant())
	{
		const Square capturedSq = toSquare(u8(to) + static_cast<u8>(side ? -8 : 8));

		removePiece(capturedSq);
	} else if (flags.kSideCastle())
	{
		switch (to)
		{
			case SQ_G1:
				movePiece(SQ_H1, SQ_F1);
				break;
			case SQ_G8:
				movePiece(SQ_H8, SQ_F8);
				break;
			default:
				break;
		}
	} else if (flags.qSideCastle())
	{
		switch (to)
		{
			case SQ_C1:
				movePiece(SQ_A1, SQ_D1);
				break;
			case SQ_C8:
				movePiece(SQ_A8, SQ_D8);
				break;
			default:
				break;
		}
	}

	if (const PieceType capturedType = move.capturedPiece();
		capturedType != PieceType::NO_PIECE_TYPE)
		removePiece(to);

	movePiece(from, to);

	if (move.flags().promotion())
		promotePawn(to, move.promotedPiece());
}
