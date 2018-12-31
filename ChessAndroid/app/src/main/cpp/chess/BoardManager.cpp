#include "BoardManager.h"

//#include <ctime>
#include <algorithm>

#include "data/pieces/Piece.h"
#include "data/minimax/MiniMax.h"
#include "data/minimax/Hash.h"

std::thread *BoardManager::m_WorkerThread;
BoardManager::PieceChangeListener BoardManager::m_Listener;
Board BoardManager::m_Board;
CacheTable BoardManager::cache;
std::vector<PosPair> BoardManager::movesHistory;
bool BoardManager::isWhiteAtBottom = true;

void BoardManager::initBoardManager(const PieceChangeListener &listener)
{
	m_Board.initDefaultBoard();
	m_Listener = listener;

	//srand(static_cast<unsigned int>(time(nullptr)));
	//isWhitePlayersTurn = isWhiteAtBottom = rand() % 2 == 0; // TODO: Support both sides again
	//moveComputerPlayer();
}

void BoardManager::loadGame(Board &&board)
{
	m_Board = std::move(board);

	if (m_Board.hash == 0)
		m_Board.hash = Hash::compute(m_Board);

	cache.clearAll();
	movesHistory.clear();
}

void BoardManager::loadGame(const std::vector<PosPair> &moves)
{
	m_Board.initDefaultBoard();
	m_Board.hash = Hash::compute(m_Board);

	cache.clearAll();
	m_Listener(GameState::NONE, true, {});

	for (const auto &move : moves)
		movePiece(move.first, move.second, false);
}

std::vector<Pos> BoardManager::getPossibleMoves(const Pos &selectedPos)
{
	const auto &piece = m_Board[selectedPos];
	auto moves = piece.getPossibleMoves(selectedPos, m_Board);

	const auto iterator = std::remove_if(moves.begin(), moves.end(), [&](const Pos &destPos) {
		Board board(m_Board);
		movePieceInternal(selectedPos, destPos, board);
		return Player::isInChess(piece.isWhite, board);
	});
	moves.erase(iterator, moves.end());

	return moves;
}

void BoardManager::movePiece(const Pos &selectedPos, const Pos &destPos, const bool movedByPlayer)
{
	if (!selectedPos.isValid() || !destPos.isValid()) return;

	std::vector<PosPair> piecesMoved{ {selectedPos, destPos} };

	auto state = GameState::NONE;
	auto &destPiece = m_Board[destPos];

	if (destPiece && destPiece.type == Piece::Type::KING)
		state = destPiece.isWhite ? GameState::WINNER_BLACK : GameState::WINNER_WHITE;

	auto &selectedPiece = m_Board[selectedPos];
	bool shouldRedraw = false;

	if (selectedPiece.type == Piece::Type::PAWN)
		shouldRedraw = movePawn(selectedPiece, destPos);
	else if (selectedPiece.type == Piece::Type::KING && !selectedPiece.hasBeenMoved)
		piecesMoved.emplace_back(moveKing(selectedPiece, selectedPos, destPos, m_Board));

	selectedPiece.hasBeenMoved = true;

	m_Board[destPos] = selectedPiece;
	m_Board[selectedPos] = Piece();

	if (m_WorkerThread)
	{
		m_WorkerThread->detach();
		delete m_WorkerThread;
		m_WorkerThread = nullptr;
	}

	if (state == GameState::NONE)
	{
		if (Player::hasOnlyTheKing(true, m_Board) && Player::hasOnlyTheKing(false, m_Board))
			state = GameState::DRAW;
		else if (Player::hasNoValidMoves(true, m_Board))
		{
			state = Player::isInChess(true, m_Board) ? GameState::WINNER_BLACK : GameState::DRAW;
		}
		else if (Player::hasNoValidMoves(false, m_Board) && Player::isInChess(false, m_Board))
		{
			state = Player::isInChess(false, m_Board) ? GameState::WINNER_WHITE : GameState::DRAW;
		}
	}

	movesHistory.emplace_back(selectedPos, destPos);
	m_Listener(state, shouldRedraw, piecesMoved);

	if (state == GameState::NONE && movedByPlayer)
		m_WorkerThread = new std::thread(moveComputerPlayer);
}

void BoardManager::moveComputerPlayer()
{
	constexpr int depth = 4;
	const auto pair = isWhiteAtBottom ? MiniMax::MinMove(getBoard(), depth) : MiniMax::MaxMove(getBoard(), depth);
	movePiece(pair.first, pair.second, false);
}

GameState BoardManager::movePieceInternal(const Pos &selectedPos, const Pos &destPos, Board &board)
{
	auto state = GameState::NONE;
	auto &destPiece = board[destPos];

	if (destPiece && destPiece.type == Piece::Type::KING)
		state = destPiece.isWhite ? GameState::WINNER_BLACK : GameState::WINNER_WHITE;

	auto &selectedPiece = board[selectedPos];

	if (selectedPiece.type == Piece::Type::PAWN)
		movePawn(selectedPiece, destPos);
	else if (selectedPiece.type == Piece::Type::KING)
		moveKing(selectedPiece, selectedPos, destPos, board);

	selectedPiece.hasBeenMoved = true;

	board[destPos] = selectedPiece;
	board[selectedPos] = Piece();

	return state;
}

bool BoardManager::movePawn(Piece &selectedPiece, const Pos &destPos)
{
	if (selectedPiece.hasBeenMoved)
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
			else if (other.type == Piece::Type::ROOK && king.hasSameColor(other) && !other.hasBeenMoved)
			{
				// Move the Rook
				const short startX = 7;
				const short destX = 5;
				const short y = selectedPos.y;

				other.hasBeenMoved = true;
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
			else if (other.type == Piece::Type::ROOK && king.hasSameColor(other) && !other.hasBeenMoved)
			{
				// Move the Rook
				const short startX = 0;
				const short destX = 3;
				const short y = selectedPos.y;

				other.hasBeenMoved = true;
				board.data[destX][y] = other;
				board.data[startX][y] = Piece();

				return std::make_pair(Pos(startX, y), Pos(destX, y));
			}
		}
	}

	return std::make_pair(Pos(), Pos());
}
