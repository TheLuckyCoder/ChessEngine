#include "BoardManager.h"

#include "Stats.h"
#include "data/Board.h"
#include "algorithm/Evaluation.h"
#include "algorithm/Hash.h"
#include "algorithm/NegaMax.h"
#include "algorithm/MoveOrdering.h"

Settings BoardManager::m_Settings(4u, std::thread::hardware_concurrency() - 1u, 200, true);
BoardManager::PieceChangeListener BoardManager::m_Listener;
Board BoardManager::m_Board;
std::vector<RootMove> BoardManager::m_MovesHistory;

void BoardManager::initBoardManager(const PieceChangeListener &listener, const bool isPlayerWhite)
{
    Hash::init();
    MoveOrdering::init();

	m_Board.initDefaultBoard();
	m_Listener = listener;
	m_MovesHistory.reserve(100);

	m_MovesHistory.clear();
	Stats::resetStats();

	m_IsPlayerWhite = isPlayerWhite;

	if (!isPlayerWhite)
	    m_WorkerThread = std::thread(moveComputerPlayer, m_Settings);
}

void BoardManager::loadGame(const std::vector<PosPair> &moves)
{
	m_Board.initDefaultBoard();
	m_Board.key = Hash::compute(m_Board);

	for (const auto &move : moves)
	{
		movePieceInternal(move.first, move.second, m_Board, false);
		m_Board.updateState();
		m_Board.score = Evaluation::evaluate(m_Board);
		m_MovesHistory.emplace_back(move.first, move.second, m_Board);
	}

	m_Board.updateState();
	m_Board.score = Evaluation::evaluate(m_Board);
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
	m_Board.whiteToMove = !m_Board.whiteToMove;
	m_Board.isPromotion = m_Board.isCapture = false;

	StackVector<PosPair, 2> piecesMoved{ {selectedPos, destPos} };
	
	auto &selectedPiece = m_Board[selectedPos];
	bool shouldRedraw = false;

	if (selectedPiece.type == Type::PAWN)
		shouldRedraw = movePawn(selectedPiece, destPos);
	else if (selectedPiece.type == Type::KING)
	{
		if (selectedPiece.isWhite)
			m_Board.whiteKingSquare = destPos.toSquare();
		else
			m_Board.blackKingSquare = destPos.toSquare();

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

	m_Board.npm -= Evaluation::getPieceValue(m_Board[destPos].type);

	m_Board[destPos] = selectedPiece;
	m_Board[selectedPos] = Piece();

	m_Board.updateState();
	m_Board.key = Hash::compute(m_Board);
	m_Board.score = Evaluation::evaluate(m_Board);

	m_MovesHistory.emplace_back(selectedPos, destPos, m_Board);
	m_Listener(m_Board.state, shouldRedraw, piecesMoved);

	if (movedByPlayer && (m_Board.state == State::NONE || m_Board.state == State::WHITE_IN_CHESS || m_Board.state == State::BLACK_IN_CHESS))
		m_WorkerThread = std::thread(moveComputerPlayer, m_Settings);
}

void BoardManager::movePieceInternal(const Pos &selectedPos, const Pos &destPos, Board &board, const bool updateState)
{
	board.whiteToMove = !board.whiteToMove;
	auto &selectedPiece = board[selectedPos];
	auto &destPiece = board[destPos];
	bool hashHandled = false;
	board.isPromotion = board.isCapture = false;

	if (selectedPiece.type == Type::PAWN)
	{
		if (movePawn(selectedPiece, destPos))
		{
			Hash::promotePawn(board.key, selectedPos, destPos, selectedPiece.isWhite, Type::QUEEN);
			hashHandled = true;
			board.isPromotion = true;
		}
	}
	else if (selectedPiece.type == Type::KING)
	{
		if (selectedPiece.isWhite)
			board.whiteKingSquare = destPos.toSquare();
		else
			board.blackKingSquare = destPos.toSquare();

		if (!selectedPiece.moved)
		{
			if (const auto posPair = moveKing(selectedPiece, selectedPos, destPos, board); posPair.first.isValid())
			{
				if (selectedPiece.isWhite)
					board.whiteCastled = true;
				else
					board.blackCastled = true;

				Hash::makeMove(board.key, posPair.first, posPair.second, Piece(Type::ROOK, selectedPiece.isWhite));
			}
		}
	}

	Hash::flipSide(board.key);
	if (!hashHandled)
		Hash::makeMove(board.key, selectedPos, destPos, selectedPiece, destPiece);

	if (destPiece)
	{
		board.npm -= Evaluation::getPieceValue(destPiece.type);
		board.isCapture = true;
	}

	selectedPiece.moved = true;
	destPiece = selectedPiece;
	board[selectedPos] = Piece();

	if (updateState)
		board.updateState();
}

// This function should only be called through the Worker Thread
void BoardManager::moveComputerPlayer(const Settings &settings)
{
	assert(std::this_thread::get_id() == m_WorkerThread.get_id());

    m_IsWorking = true;
	Stats::resetStats();
	Stats::startTimer();

	const auto pair = NegaMax::getBestMove(m_Board, !m_IsPlayerWhite, settings);

	Stats::stopTimer();
	movePiece(pair.first, pair.second, false);

	m_WorkerThread.detach();
	m_IsWorking = false;
}

bool BoardManager::movePawn(Piece &selectedPiece, const Pos &destPos)
{
	if (selectedPiece.moved)
	{
		if (destPos.y == 0 || destPos.y == 7)
		{
			selectedPiece.type = Type::QUEEN;
			return true;
		}
	}

	return false;
}

PosPair BoardManager::moveKing(Piece &king, const Pos &selectedPos, const Pos &destPos, Board &board)
{
	if (destPos.x == 6)
	{
		constexpr byte startX = 7;
		const auto y = selectedPos.y;

		auto &rook = board.data[startX][y];
		if (rook.type == Type::ROOK && king.isSameColor(rook) && !rook.moved)
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
		constexpr byte startX = 0;
		const auto y = selectedPos.y;

		auto &rook = board.data[startX][y];
		if (rook.type == Type::ROOK && king.isSameColor(rook) && !rook.moved)
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
