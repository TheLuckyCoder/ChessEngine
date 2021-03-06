#pragma once

#include <atomic>
#include <functional>
#include <thread>
#include <vector>

#include "SearchOptions.h"
#include "Board.h"
#include "algorithm/MoveGen.h"

enum class GameState : u8
{
	NONE,
	WINNER_WHITE,
	WINNER_BLACK,
	DRAW,
	WHITE_IN_CHECK,
	BLACK_IN_CHECK,
	INVALID = 10
};

/**
 * This is a wrapper for the actual Chess Engine in order to make it easier to be used with custom UIs
 */
class BoardManager final
{
public:
	struct IndexedPiece
	{
		IndexedPiece(const i32 id, const Square square, const Color pieceColor, const PieceType pieceType)
			: id(id), square(square), pieceColor(pieceColor), pieceType(pieceType)
		{
		}

		/**
		 * An unique ID assigned to this piece
		 */
		i32 id;
		/**
		 * The square on which the Piece is located
		 */
		Square square;
		Color pieceColor;
		PieceType pieceType;
	};

	using BoardChangedCallback = std::function<void(GameState state)>;

private:
	static BoardChangedCallback _callback;

	inline static std::atomic_bool _isWorking{ false };
	inline static bool _isPlayerWhite{ true };
	static SearchOptions _searchOptions;
	static Board _board;
	static std::vector<IndexedPiece> _indexedPieces;
	
public:
	static void initBoardManager(const BoardChangedCallback &callback, bool isPlayerWhite = true);
	static bool loadGame(const std::string &fen, bool isPlayerWhite);
	static void loadGame(const std::vector<Move> &moves, bool isPlayerWhite);

	/// Actions
	static void makeMove(Move move);
	static void makeEngineMove();
	static bool undoLastMoves();
	static bool redoLastMoves();

	/// Getters and Setters
	static bool isWorking() { return _isWorking; }
	static bool isPlayerWhite() { return _isPlayerWhite; }
	static void setSearchOptions(const SearchOptions &searchOptions) { _searchOptions = searchOptions; }
	static SearchOptions getSearchOptions() { return _searchOptions; }
	static const auto &getBoard() { return _board; }
	static const auto &getIndexedPieces() { return _indexedPieces; }
	static std::vector<Move> getMovesHistory();
	static std::vector<Move> getPossibleMoves(Square from);

private:
	static GameState getBoardState();
	static void generatedIndexedPieces();
	static void updateIndexedPieces(Move move);
};
