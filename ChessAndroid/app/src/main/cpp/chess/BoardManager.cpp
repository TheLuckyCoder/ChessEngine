#include "BoardManager.h"

//#include <ctime>

#include "data/pieces/Piece.h"
#include "data/minimax/MiniMax.h"

BoardManager& BoardManager::m_Instance = *new BoardManager;
std::thread *BoardManager::m_WorkerThread = nullptr;
bool BoardManager::isWhiteAtBottom = true;
bool BoardManager::whitePlayersTurn = true;

void BoardManager::initBoardManager(const PieceChangeListener &listener)
{
	m_Instance.m_Board.initDefaultBoard();
	m_Instance.m_Listener = listener;

	//srand((unsigned int)time(nullptr));
	//whitePlayersTurn = isWhiteAtBottom = rand() % 2 == 0; // TODO: Support both sides again
	//moveComputerPlayer();
}

void BoardManager::loadJsonGame(Board &&board)
{
	m_Instance.m_Board = std::move(board);
}

Board &BoardManager::getBoard()
{
	return m_Instance.m_Board;
}

bool BoardManager::isWorking()
{
	return m_WorkerThread != nullptr;
}

GameState BoardManager::movePieceInternal(const Pos &selectedPos, const Pos &destPos, Board &board)
{
	GameState state = GameState::NONE;
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

void BoardManager::movePiece(const Pos &selectedPos, const Pos &destPos)
{
	if (!selectedPos.isValid() || !destPos.isValid()) return;

	std::vector<PosPair> piecesMoved;
	piecesMoved.emplace_back(selectedPos, destPos);

	auto &board = m_Instance.m_Board;
	auto state = GameState::NONE;
	auto &destPiece = board[destPos];

	if (destPiece && destPiece.type == Piece::Type::KING)
		state = destPiece.isWhite ? GameState::WINNER_BLACK : GameState::WINNER_WHITE;

	auto &selectedPiece = board[selectedPos];
	bool shouldRedraw = false;

	if (selectedPiece.type == Piece::Type::PAWN)
		shouldRedraw = movePawn(selectedPiece, destPos);
	else if (selectedPiece.type == Piece::Type::KING && !selectedPiece.hasBeenMoved)
		piecesMoved.emplace_back(moveKing(selectedPiece, selectedPos, destPos, board));

	selectedPiece.hasBeenMoved = true;

	board[destPos] = selectedPiece;
	board[selectedPos] = Piece();

	if (m_WorkerThread)
	{
		m_WorkerThread->detach();
		delete m_WorkerThread;
		m_WorkerThread = nullptr;
	}

	if (state == GameState::NONE)
	{
		if (Player::hasOnlyTheKing(true, board) && Player::hasOnlyTheKing(false, board))
			state = GameState::DRAW;
		else {
			whitePlayersTurn = !whitePlayersTurn;
			if (!whitePlayersTurn)
				m_WorkerThread = new std::thread(moveComputerPlayer);
		}
	}

	m_Instance.m_Listener(state, shouldRedraw, piecesMoved);
}

void BoardManager::moveComputerPlayer()
{
	if (whitePlayersTurn == isWhiteAtBottom) return;

	constexpr int depth = 5;
	const auto pair = isWhiteAtBottom ?	MiniMax::MinMove(getBoard(), depth) : MiniMax::MaxMove(getBoard(), depth);
	movePiece(pair.first, pair.second);

	whitePlayersTurn = true;
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
