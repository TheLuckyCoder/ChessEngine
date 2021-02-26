#pragma once

#include <atomic>
#include <functional>
#include <thread>
#include <vector>

#include "Settings.h"
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

	using BoardChangedListener = std::function<void(GameState state)>;

private:
	static Settings _settings;
	inline static std::thread _workerThread;
	inline static std::atomic_bool _isWorking{ false };
	inline static bool _isPlayerWhite{ true };
	static BoardChangedListener _listener;
	static Board _board;
	static std::vector<IndexedPiece> _indexedPieces;
	
public:
	static void initBoardManager(const BoardChangedListener &listener, bool isPlayerWhite = true);
	static bool loadGame(bool isPlayerWhite, const std::string &fen);
	static void loadGame(const std::vector<Move> &moves, bool isPlayerWhite);
	static bool undoLastMoves();

	static const Board &getBoard() { return _board; }
	static const std::vector<IndexedPiece> &getIndexedPieces() { return _indexedPieces; }
	static std::vector<Move> getMovesHistory();
	static bool isWorking() { return _isWorking; }
	static bool isPlayerWhite() { return _isPlayerWhite; }
	static std::vector<Move> getPossibleMoves(Square from);
	static void makeMove(Move move, bool movedByPlayer = true);
	static void setSettings(const Settings &settings) { _settings = settings; }
	static void forceMove();

private:
	static void moveComputerPlayer(const Settings &settings);
	static GameState getBoardState();
	static void generatedIndexedPieces();
	static void updateIndexedPieces(Move move);
};
