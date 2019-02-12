#include "BoardManager.h"

//#include <ctime>
#include <chrono>

#include "DebugFile.h"
#include "data/Board.h"
#include "minimax/Evaluation.h"
#include "minimax/Hash.h"
#include "minimax/NegaMax.h"

BoardManager::PieceChangeListener BoardManager::m_Listener;
Board BoardManager::m_Board;
HashTable<Cache> BoardManager::cacheTable(1); // 30000000
std::vector<PosPair> BoardManager::movesHistory;
std::atomic_size_t BoardManager::boardsEvaluated = 0;

void BoardManager::initBoardManager(const PieceChangeListener &listener)
{
	m_Board.initDefaultBoard();
	m_Listener = listener;
	movesHistory.reserve(100);

	movesHistory.clear();
	boardsEvaluated = 0;

	// TODO: Support both sides again
	//srand(static_cast<unsigned int>(time(nullptr)));
	//isPlayerWhite = rand() % 2 == 0;

	if (!isPlayerWhite)
		m_WorkerThread = new std::thread(moveComputerPlayer);
}

void BoardManager::loadGame(std::vector<PosPair> &&moves)
{
	m_Board.initDefaultBoard();
	m_Board.hash = Hash::compute(m_Board);

	for (const auto &move : moves)
		movePieceInternal(move.first, move.second, m_Board, false);

	m_Board.updateState();

	movesHistory = std::move(moves);
	m_Listener(m_Board.state, true, {});
}

Piece::MaxMovesVector BoardManager::getPossibleMoves(const Pos &selectedPos)
{
	const auto &piece = m_Board[selectedPos];
	auto moves = piece.getPossibleMoves(selectedPos, m_Board);

	const auto iterator = std::remove_if(moves.begin(), moves.end(), [&](const Pos &destPos) {
		Board newBoard = m_Board;
		movePieceInternal(selectedPos, destPos, newBoard, false);
		return Player::isInChess(piece.isWhite, newBoard);
	});
	moves.erase(iterator, moves.end());

	return moves;
}

void BoardManager::movePiece(const Pos &selectedPos, const Pos &destPos, const bool movedByPlayer)
{
	if (!selectedPos.isValid() || !destPos.isValid()) return;

	StackVector<PosPair, 2> piecesMoved{ {selectedPos, destPos} };
	
	auto &selectedPiece = m_Board[selectedPos];
	bool shouldRedraw = false;

	if (selectedPiece.type == Piece::Type::PAWN)
		shouldRedraw = movePawn(selectedPiece, destPos);
	else if (selectedPiece.type == Piece::Type::KING)
	{
		if (selectedPiece.isWhite)
			m_Board.whiteKingPos = destPos.toBitboard();
		else
			m_Board.blackKingPos = destPos.toBitboard();

		if (!selectedPiece.moved)
		{
			const auto &pair = piecesMoved.emplace_back(moveKing(selectedPiece, selectedPos, destPos, m_Board));
			if (pair.first.isValid())
			{
				if (selectedPiece.isWhite)
					m_Board.whiteCastled = true;
				else
					m_Board.blackCastled = true;
			}
		}
	}

	selectedPiece.moved = true;

	m_Board[destPos] = selectedPiece;
	m_Board[selectedPos] = Piece();

	if (m_Board.state == State::NONE)
		m_Board.updateState();

	movesHistory.emplace_back(selectedPos, destPos);
	m_Listener(m_Board.state, shouldRedraw, piecesMoved);

	m_Board.hash = Hash::compute(m_Board);

	if (movedByPlayer && (m_Board.state == State::NONE || m_Board.state == State::WHITE_IN_CHESS || m_Board.state == State::BLACK_IN_CHESS))
		m_WorkerThread = new std::thread(moveComputerPlayer);
}

void BoardManager::movePieceInternal(const Pos &selectedPos, const Pos &destPos, Board &board, const bool checkValid)
{
	auto &selectedPiece = board[selectedPos];
	auto &destPiece = board[destPos];
	bool recalculateHash = false;

	if (selectedPiece.type == Piece::Type::PAWN)
		recalculateHash = movePawn(selectedPiece, destPos);
	else if (selectedPiece.type == Piece::Type::KING)
	{
		if (selectedPiece.isWhite)
			board.whiteKingPos = destPos.toBitboard();
		else
			board.blackKingPos = destPos.toBitboard();

		if (!selectedPiece.moved && moveKing(selectedPiece, selectedPos, destPos, board).first.isValid())
		{
			recalculateHash = true;
			if (selectedPiece.isWhite)
				board.whiteCastled = true;
			else
				board.blackCastled = true;
		}
	}

	/*if (!recalculateHash)
	{
		// Remove Selected Piece
		board.hash ^= Hash::getHash(selectedPos, selectedPiece);

		if (destPiece) // Remove Dest Piece
			board.hash ^= Hash::getHash(destPos, destPiece);

		// Add Selected Piece to Destination
		selectedPiece.moved = true;
		board.hash ^= Hash::getHash(destPos, selectedPiece);
	} else
		selectedPiece.moved = true;*/

	destPiece = selectedPiece;
	board[selectedPos] = Piece();

	if (checkValid)
	{
		/*Cache cache;
		if (recalculateHash)
			board.hash = Hash::compute(board);*/

		board.state = State::NONE;

		board.updateState();

		/*if (cacheTable.get(board.hash, cache))
		{
			board.state = cache.state;
			board.value = cache.value;
			return;
		}*/

		switch (board.state)
		{
		case State::NONE:
		case State::WHITE_IN_CHESS:
		case State::BLACK_IN_CHESS:
			board.value = Evaluation::evaluate(board);
			break;
		case State::WINNER_WHITE:
			board.value = VALUE_WINNER_WHITE;
			break;
		case State::WINNER_BLACK:
			board.value = VALUE_WINNER_BLACK;
			break;
		case State::DRAW:
			board.value = 0;
			break;
		}

		/*cache.state = board.state;
		cache.value = board.value;
		cacheTable.insert(board.hash, cache);*/
	}
}

void BoardManager::moveComputerPlayer()
{
	boardsEvaluated = 0;
#ifdef CR_PLATFORM_WINDOWS
	const auto startTime = std::chrono::high_resolution_clock::now();
	allocations = 0;
	allocatedMemory = 0;
#endif

	const auto pair = NegaMax::negaMax(m_Board, !isPlayerWhite);
	movePiece(pair.first, pair.second, false);

	if (m_WorkerThread)
	{
		m_WorkerThread->detach();
		delete m_WorkerThread;
		m_WorkerThread = nullptr;

#ifdef CR_PLATFORM_WINDOWS
		const auto endTime = std::chrono::high_resolution_clock::now();
		writeTime("Board Evaluated: ", static_cast<size_t>(boardsEvaluated),
			"\tTime Needed: ", std::chrono::duration<double, std::milli>(endTime - startTime).count(),
			"\tAllocations: ", static_cast<size_t>(allocations),
			"\tAllocated Memory: ", static_cast<size_t>(allocatedMemory));
#endif
	}
}

bool BoardManager::movePawn(Piece &selectedPiece, const Pos &destPos)
{
	if (selectedPiece.moved)
	{
		if (destPos.y == 0 || destPos.y == 7)
		{
			selectedPiece.type = Piece::Type::QUEEN;
			return true;
		}
	}

	return false;
}

PosPair BoardManager::moveKing(Piece &king, const Pos &selectedPos, const Pos &destPos, Board &board)
{
	if (destPos.x == 6)
	{
		const auto y = selectedPos.y;
		const byte startX = 7;

		auto &rook = board.data[startX][y];
		if (rook.type == Piece::Type::ROOK && king.hasSameColor(rook) && !rook.moved)
		{
			rook.moved = true;

			const byte destX = 5;
			board.data[destX][y] = rook;
			board.data[startX][y] = Piece();

			return std::make_pair(Pos(startX, y), Pos(destX, y));
		}
	}
	else if (destPos.x == 2)
	{
		const byte startX = 0;
		const auto y = selectedPos.y;

		auto &rook = board.data[startX][y];
		if (rook.type == Piece::Type::ROOK && king.hasSameColor(rook) && !rook.moved)
		{
			rook.moved = true;

			const byte destX = 3;
			board.data[destX][y] = rook;
			board.data[startX][y] = Piece();

			return std::make_pair(Pos(startX, y), Pos(destX, y));
		}
	}

	return std::make_pair(Pos(), Pos());
}
