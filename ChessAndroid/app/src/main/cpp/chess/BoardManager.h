#pragma once

#include <atomic>
#include <functional>
#include <mutex>
#include <thread>
#include <vector>

#include "SearchOptions.h"
#include "Board.h"
#include "algorithm/MoveGen.h"
#include "persistence/UndoRedoMoves.h"

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
	using BoardChangedCallback = std::function<void(GameState state)>;

private:
	static BoardChangedCallback _callback;

//	inline static std::mutex lock;
	inline static std::atomic_bool _isWorking{ false };
	inline static bool _isPlayerWhite{ true };
	static SearchOptions _searchOptions;
	static Board _currentBoard;
	static UndoRedoHistory _undoRedoHistory;
	
public:
	static void initBoardManager(const BoardChangedCallback &callback, bool isPlayerWhite = true);
	static bool loadGame(const std::string &fen, bool isPlayerWhite);
	static void loadGame(const std::vector<Move> &moves, bool isPlayerWhite);

	/// Actions
	static void makeMove(Move move);
	static void makeEngineMove();
	static void undoLastMoves();
	static void redoLastMoves();

	/// Getters and Setters
	static bool isWorking() noexcept { return _isWorking; }
	static bool isPlayerWhite() noexcept { return _isPlayerWhite; }
	static void setSearchOptions(const SearchOptions &searchOptions) { _searchOptions = searchOptions; }
	static SearchOptions getSearchOptions() noexcept { return _searchOptions; }
	static const auto &getBoard() noexcept { return _currentBoard; }
	static IndexedPieces getIndexedPieces() noexcept { return _undoRedoHistory.peek().getIndexedPieces(); }
	static std::vector<Move> getMovesHistory();
	static std::vector<Move> getPossibleMoves(Square from);

private:
	static GameState getBoardState();
};
