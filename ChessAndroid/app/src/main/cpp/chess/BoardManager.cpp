#include "BoardManager.h"

//#include <ctime>

#include "data/Board.h"
#include "minimax/MiniMax.h"
#include "minimax/Hash.h"

std::thread *BoardManager::m_WorkerThread = nullptr;
BoardManager::PieceChangeListener BoardManager::m_Listener;
Board BoardManager::m_Board;
HashTable<int> BoardManager::evaluationsCache(1);
std::vector<PosPair> BoardManager::movesHistory;
bool BoardManager::isPlayerWhite = true;
std::atomic_size_t BoardManager::boardsEvaluated = 0;

void BoardManager::initBoardManager(const PieceChangeListener &listener)
{
	m_Board.initDefaultBoard();
	m_Listener = listener;
	movesHistory.reserve(50);
	boardsEvaluated = 0;

	// TODO: Support both sides again
	//srand(static_cast<unsigned int>(time(nullptr)));
	//isPlayerWhite = rand() % 2 == 0;

	if (!isPlayerWhite)
		m_WorkerThread = new std::thread(moveComputerPlayer);
}

void BoardManager::loadGame(const Board &board)
{
	m_Board = board;

	if (m_Board.hash == 0)
		m_Board.hash = Hash::compute(m_Board);

	evaluationsCache.clear();
	movesHistory.clear();
}

void BoardManager::loadGame(const std::vector<PosPair> &moves)
{
	m_Board.initDefaultBoard();
	m_Board.hash = Hash::compute(m_Board);

	evaluationsCache.clear();
	m_Listener(GameState::NONE, true, {});

	for (const auto &move : moves)
		movePiece(move.first, move.second, false);
}

Piece::MovesReturnType BoardManager::getPossibleMoves(const Pos &selectedPos)
{
	return m_Board[selectedPos].getValidMoves(selectedPos, m_Board);
}

void BoardManager::movePiece(const Pos &selectedPos, const Pos &destPos, const bool movedByPlayer)
{
	if (!selectedPos.isValid() || !destPos.isValid()) return;

	StackVector<PosPair, 2> piecesMoved;
	piecesMoved.emplace_back(selectedPos, destPos);

	auto state = GameState::NONE;
	auto &destPiece = m_Board[destPos];

	if (destPiece && destPiece.type == Piece::Type::KING)
		state = destPiece.isWhite ? GameState::WINNER_BLACK : GameState::WINNER_WHITE;

	auto &selectedPiece = m_Board[selectedPos];
	bool shouldRedraw = false;

	if (selectedPiece.type == Piece::Type::PAWN)
		shouldRedraw = movePawn(selectedPiece, destPos);
	else if (selectedPiece.type == Piece::Type::KING && !selectedPiece.moved)
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

	selectedPiece.moved = true;

	destPiece = selectedPiece;
	m_Board[selectedPos] = Piece();

	if (state == GameState::NONE)
	{
		if (Player::hasOnlyTheKing(true, m_Board) && Player::hasOnlyTheKing(false, m_Board))
			state = GameState::DRAW;
		else if (Player::hasNoValidMoves(true, m_Board))
		{
			state = Player::isInChess(true, m_Board) ? GameState::WINNER_BLACK : GameState::DRAW;
		}
		else if (Player::hasNoValidMoves(false, m_Board))
		{
			state = Player::isInChess(false, m_Board) ? GameState::WINNER_WHITE : GameState::DRAW;
		}
	}

	movesHistory.emplace_back(selectedPos, destPos);
	if (!(state == GameState::WINNER_WHITE || state == GameState::WINNER_BLACK) && movesHistory.size() == 50)
		state = GameState::DRAW;
	m_Listener(state, shouldRedraw, piecesMoved);

	if (movedByPlayer && state == GameState::NONE)
		m_WorkerThread = new std::thread(moveComputerPlayer);
}

void BoardManager::movePieceInternal(const Pos &selectedPos, const Pos &destPos, Board &board, const bool checkValid)
{
	board.state = GameState::NONE;
	auto &selectedPiece = board[selectedPos];
	auto &destPiece = board[destPos];

	if (selectedPiece.type == Piece::Type::PAWN)
		movePawn(selectedPiece, destPos);
	else if (selectedPiece.type == Piece::Type::KING)
	{
		if (moveKing(selectedPiece, selectedPos, destPos, board).first.isValid())
		{
			if (selectedPiece.isWhite)
				board.whiteCastled = true;
			else
				board.blackCastled = true;
		}
	}

	selectedPiece.moved = true;

	destPiece = selectedPiece;
	board[selectedPos] = Piece();

	if (checkValid)
	{
		const bool whiteInChess = Player::isInChess(true, board);
		if (whiteInChess)
			board.state = GameState::WHITE_IN_CHESS;

		if (Player::hasNoMoves(true, board))
			board.state = whiteInChess ? GameState::WINNER_BLACK : GameState::DRAW;
		else
		{
			const bool blackInChess = Player::isInChess(false, board);
			if (blackInChess)
				board.state = GameState::BLACK_IN_CHESS;
			if (Player::hasNoMoves(false, board))
				board.state = blackInChess ? GameState::WINNER_WHITE : GameState::DRAW;
		}
	}
}

void BoardManager::moveComputerPlayer()
{
	boardsEvaluated = 0;

	const auto pair = isPlayerWhite ? MiniMax::MinMove(getBoard()) : MiniMax::MaxMove(getBoard());
	movePiece(pair.first, pair.second, false);

	if (m_WorkerThread)
	{
		m_WorkerThread->detach();
		delete m_WorkerThread;
		m_WorkerThread = nullptr;
	}
}

bool BoardManager::movePawn(Piece &selectedPiece, const Pos &destPos)
{
	if (selectedPiece.moved)
	{
		if (destPos.y == 0 || destPos.y == 7)
		{
			selectedPiece = Piece(Piece::Type::QUEEN, selectedPiece.isWhite);
			return true;
		}
	}

	return false;
}

PosPair BoardManager::moveKing(Piece &king, Pos selectedPos, const Pos &destPos, Board &board)
{
	if (destPos.x == 6)
	{
		while (selectedPos.x < 7)
		{
			selectedPos.x++;
			auto &other = board[selectedPos];

			if (selectedPos.x < 7)
			{
				if (other)
					break;
			}
			else if (other.type == Piece::Type::ROOK && king.hasSameColor(other) && !other.moved)
			{
				// Move the Rook
				const byte startX = 7;
				const byte destX = 5;
				const byte y = selectedPos.y;

				other.moved = true;
				board.data[destX][y] = other;
				board.data[startX][y] = Piece();

				return std::make_pair(Pos(startX, y), Pos(destX, y));
			}
		}
	}
	else if (destPos.x == 2)
	{
		while (selectedPos.x > 0)
		{
			selectedPos.x--;
			auto &other = board[selectedPos];

			if (selectedPos.x > 0)
			{
				if (other)
					break;
			}
			else if (other.type == Piece::Type::ROOK && king.hasSameColor(other) && !other.moved)
			{
				// Move the Rook
				const byte startX = 0;
				const byte destX = 3;
				const byte y = selectedPos.y;

				other.moved = true;
				board.data[destX][y] = other;
				board.data[startX][y] = Piece();

				return std::make_pair(Pos(startX, y), Pos(destX, y));
			}
		}
	}

	return std::make_pair(Pos(), Pos());
}
