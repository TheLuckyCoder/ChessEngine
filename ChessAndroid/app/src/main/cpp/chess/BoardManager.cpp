#include "BoardManager.h"

//#include <ctime>

#include "Stats.h"
#include "data/Board.h"
#include "algorithm/Evaluation.h"
#include "algorithm/Hash.h"
#include "algorithm/NegaMax.h"

BoardManager::PieceChangeListener BoardManager::m_Listener;
Board BoardManager::m_Board;
std::vector<PosPair> BoardManager::m_MovesHistory;

void BoardManager::initBoardManager(const PieceChangeListener &listener)
{
    Hash::init();

	m_Board.initDefaultBoard();
	m_Listener = listener;
	m_MovesHistory.reserve(100);

	m_MovesHistory.clear();
	Stats::resetStats();

	// TODO: Support both sides again
	//srand(static_cast<unsigned int>(time(nullptr)));
	//isPlayerWhite = rand() % 2 == 0;

	if (!isPlayerWhite)
	    m_WorkerThread = std::thread(moveComputerPlayer, m_Settings);
}

void BoardManager::loadGame(std::vector<PosPair> &&moves)
{
	m_Board.initDefaultBoard();
	m_Board.key = Hash::compute(m_Board);

	for (const auto &move : moves)
		movePieceInternal(move.first, move.second, m_Board, false);

	m_Board.updateState();

	m_MovesHistory = std::move(moves);
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

	m_Board.npm -= Evaluation::getPieceValue(m_Board[destPos].type);

	m_Board[destPos] = selectedPiece;
	m_Board[selectedPos] = Piece();

	m_Board.updateState();

	m_MovesHistory.emplace_back(selectedPos, destPos);
	m_Listener(m_Board.state, shouldRedraw, piecesMoved);

	m_Board.key = Hash::compute(m_Board);

	if (movedByPlayer && (m_Board.state == State::NONE || m_Board.state == State::WHITE_IN_CHESS || m_Board.state == State::BLACK_IN_CHESS))
		m_WorkerThread = std::thread(moveComputerPlayer, m_Settings);
}

void BoardManager::movePieceInternal(const Pos &selectedPos, const Pos &destPos, Board &board, const bool checkValid)
{
	board.whiteToMove = !board.whiteToMove;
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

	//recalculateHash = true;

	if (!recalculateHash)
	{
		// Remove Selected Piece
		board.key ^= Hash::getHash(selectedPos, selectedPiece);

		if (destPiece) // Remove Destination Piece if any
			board.key ^= Hash::getHash(destPos, destPiece);

		// Add Selected Piece to Destination
		board.key ^= Hash::getHash(destPos, selectedPiece);

		// Flip the Side
		board.key ^= Hash::whiteToMove;
	}

	board.npm -= Evaluation::getPieceValue(destPiece.type);

	selectedPiece.moved = true;
	destPiece = selectedPiece;
	board[selectedPos] = Piece();

	if (checkValid)
	{
		if (recalculateHash)
			board.key = Hash::compute(board);

		board.updateState();

		switch (board.state)
		{
		case State::NONE:
		case State::WHITE_IN_CHESS:
		case State::BLACK_IN_CHESS:
			board.score = Evaluation::evaluate(board);
			break;
		case State::WINNER_WHITE:
			board.score = VALUE_WINNER_WHITE;
			break;
		case State::WINNER_BLACK:
			board.score = VALUE_WINNER_BLACK;
			break;
		case State::DRAW:
		case State::INVALID:
			board.score = 0;
			break;
		}
	}
}

void BoardManager::moveComputerPlayer(const Settings &settings)
{
    m_IsWorking = true;
	Stats::resetStats();
	Stats::startTimer();

	const bool firstMove = m_MovesHistory.empty() || m_MovesHistory.size() == 1;
	const auto pair = NegaMax::getBestMove(m_Board, !isPlayerWhite, settings, firstMove);

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
		constexpr byte startX = 7;
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
	else if (destPos.x == 2)
	{
		constexpr byte startX = 0;
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
