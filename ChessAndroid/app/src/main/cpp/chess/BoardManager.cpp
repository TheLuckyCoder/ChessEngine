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
	movesHistory.reserve(100);

	boardsEvaluated = 0;
	movesHistory.clear();

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

	boardsEvaluated = 0;
	movesHistory.clear();
}

void BoardManager::loadGame(const std::vector<PosPair> &moves)
{
	m_Board.initDefaultBoard();
	m_Board.hash = Hash::compute(m_Board);


	m_Listener(GameState::NONE, true, {});

	for (const auto &move : moves)
		movePiece(move.first, move.second, false);
}

Piece::MovesReturnType BoardManager::getPossibleMoves(const Pos &selectedPos)
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

	StackVector<PosPair, 2> piecesMoved;
	piecesMoved.emplace_back(selectedPos, destPos);

	auto state = GameState::NONE;
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

	m_Board[destPos] = selectedPiece;
	m_Board[selectedPos] = Piece();

	if (state == GameState::NONE)
	{
		if (Player::onlyKingsLeft(m_Board))
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
	m_Listener(state, shouldRedraw, piecesMoved);

	if (movedByPlayer && (state == GameState::NONE || state == GameState::WHITE_IN_CHESS || state == GameState::BLACK_IN_CHESS))
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

			const byte destX = 5;
			board.data[destX][y] = rook;
			board.data[startX][y] = Piece();

			return std::make_pair(Pos(startX, y), Pos(destX, y));
		}
	}

	return std::make_pair(Pos(), Pos());
}
