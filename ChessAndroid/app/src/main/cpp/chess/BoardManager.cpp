#include "BoardManager.h"

//#include <ctime>

#include "data/pieces/Pieces.h"

BoardManager& BoardManager::m_Instance = *(new BoardManager);
std::thread *BoardManager::m_WorkerThread = nullptr;
bool BoardManager::isWhiteAtBottom = true;
bool BoardManager::whitePlayersTurn = true;

void BoardManager::initBoardManager(PieceChangeListener listener)
{
	m_Instance.m_Board.initDefaultBoard();

	m_Instance.m_Listener = listener;

	//srand((unsigned int)time(nullptr));
	//whitePlayersTurn = isWhiteAtBottom = rand() % 2 == 0; // TODO: Make MiniMax support both sides
	//moveComputerPlayer();
}

Board &BoardManager::getBoard()
{
	return m_Instance.m_Board;
}

bool BoardManager::isWorking()
{
	return m_Instance.m_WorkerThread != nullptr;
}

GameState BoardManager::movePieceInternal(const Pos &selectedPos, const Pos &destPos, Board &board)
{
	GameState state = GameState::NONE;
	Piece *destPiece = board[destPos];

	if (destPiece)
	{
		if (destPiece->type == Piece::Type::KING)
			state = destPiece->isWhite ? GameState::WINNER_BLACK : GameState::WINNER_WHITE;
		delete destPiece;
	}

	Piece *selectedPiece = board[selectedPos];

	if (selectedPiece->type == Piece::Type::PAWN)
		movePawn(std::addressof(selectedPiece), destPos);
	else if (selectedPiece->type == Piece::Type::KING)
		moveKing(selectedPiece, selectedPos, destPos, board);

	selectedPiece->hasBeenMoved = true;

	board.data[destPos.x][destPos.y] = selectedPiece;
	board.data[selectedPos.x][selectedPos.y] = nullptr;

	return state;
}

void BoardManager::movePiece(const Pos &selectedPos, const Pos &destPos)
{
	if (!selectedPos.isValid() || !destPos.isValid()) return;

	std::vector<PosPair> piecesMoved;
	piecesMoved.emplace_back(selectedPos, destPos);

	Board &board = m_Instance.m_Board;
	GameState state = GameState::NONE;
	Piece *destPiece = board[destPos];

	if (destPiece)
	{
		if (destPiece->type == Piece::Type::KING)
			state = destPiece->isWhite ? GameState::WINNER_BLACK : GameState::WINNER_WHITE;
		delete destPiece;
	}

	Piece *selectedPiece = board[selectedPos];
	bool shouldRedraw = false;

	if (selectedPiece->type == Piece::Type::PAWN)
		shouldRedraw = movePawn(std::addressof(selectedPiece), destPos);
	else if (selectedPiece->type == Piece::Type::KING && !selectedPiece->hasBeenMoved)
		piecesMoved.emplace_back(moveKing(selectedPiece, selectedPos, destPos, board));

	selectedPiece->hasBeenMoved = true;

	board.data[destPos.x][destPos.y] = selectedPiece;
	board.data[selectedPos.x][selectedPos.y] = nullptr;

	if (m_WorkerThread)
	{
		m_WorkerThread->detach();
		delete m_Instance.m_WorkerThread;
		m_Instance.m_WorkerThread = nullptr;
	}

	if (state == GameState::NONE)
	{
		if (m_Instance.m_WhitePlayer.hasOnlyTheKing(board) && m_Instance.m_BlackPlayer.hasOnlyTheKing(board))
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
	if (whitePlayersTurn) return;

	const auto pair = m_Instance.m_BlackPlayer.getNextMove();
	movePiece(pair.first, pair.second);

	whitePlayersTurn = true;
}

bool BoardManager::movePawn(Piece **selectedPiece, const Pos &destPos)
{
	auto *pawn = static_cast<PawnPiece*>(*selectedPiece);
	if (pawn->hasBeenMoved)
	{
		if (destPos.y == 0 || destPos.y == 7)
		{
			const bool isWhite = pawn->isWhite;
			delete pawn;

			auto *queen = new QueenPiece(isWhite);
			(*selectedPiece) = queen;
			return true;
		}
	}

	return false;
}

PosPair BoardManager::moveKing(Piece *king, Pos selectedPos, const Pos &destPos, Board &board)
{
	PosPair result;

	if (destPos.x == 6)
	{
		while (selectedPos.x < 7)
		{
			selectedPos.x++;
			auto *other = board[selectedPos];

			if (selectedPos.x < 7)
			{
				if (other)
					break;
			}
			else if (other && king->hasSameColor(*other) && other->type == Piece::Type::ROOK)
			{
				auto *rook = static_cast<RookPiece*>(other);
				if (!rook->hasBeenMoved)
				{
					// Move the Rook
					const short startX = 7;
					const short destX = 5;
					const short rookY = selectedPos.y;

					board.data[startX][rookY] = nullptr;
					board.data[destX][rookY] = rook;

					rook->hasBeenMoved = true;
					result = std::make_pair(Pos(startX, rookY), Pos(destX, rookY));
				}
			}
		}
	}
	else
	{
		while (selectedPos.x > 0)
		{
			selectedPos.x--;
			auto *other = board[selectedPos];

			if (selectedPos.x > 0)
			{
				if (other)
					break;
			}
			else if (other && king->hasSameColor(*other) && other->type == Piece::Type::ROOK)
			{
				auto *rook = static_cast<RookPiece*>(other);
				if (!rook->hasBeenMoved)
				{
					// Move the Rook
					const short startX = 0;
					const short destX = 3;
					const short rookY = selectedPos.y;

					board.data[startX][rookY] = nullptr;
					board.data[destX][rookY] = rook;

					rook->hasBeenMoved = true;
					result = std::make_pair(Pos(startX, rookY), Pos(destX, rookY));
				}
			}
		}
	}

	return result;
}
