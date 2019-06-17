#include "BoardManager.h"

#include <cassert>

#include "Stats.h"
#include "data/Board.h"
#include "algorithm/Evaluation.h"
#include "algorithm/Hash.h"
#include "algorithm/NegaMax.h"
#include "algorithm/PieceAttacks.h"

Settings BoardManager::m_Settings(4u, std::thread::hardware_concurrency() - 1u, 200, true);
BoardManager::PieceChangeListener BoardManager::m_Listener;
Board BoardManager::m_Board;
std::vector<RootMove> BoardManager::m_MovesHistory;

void BoardManager::initBoardManager(const PieceChangeListener &listener, const bool isPlayerWhite)
{
    Hash::init();
    PieceAttacks::init();

	m_Board.initDefaultBoard();
	m_Listener = listener;

	m_MovesHistory.clear();
	m_MovesHistory.reserve(200);
	m_MovesHistory.emplace_back(Pos(), Pos(), m_Board);

	Stats::resetStats();

	m_IsPlayerWhite = isPlayerWhite;

	if (!isPlayerWhite)
	    m_WorkerThread = std::thread(moveComputerPlayer, m_Settings);
}

void BoardManager::loadGame(const std::vector<PosPair> &moves, const bool isPlayerWhite)
{
	m_IsPlayerWhite = isPlayerWhite;

	m_Board.initDefaultBoard();
	m_Board.key = Hash::compute(m_Board);

	m_MovesHistory.emplace_back(Pos(), Pos(), m_Board);

	for (const PosPair &move : moves)
	{
		movePieceInternal(move.first, move.second, m_Board, false);
		m_Board.updateState();
		m_Board.score = Evaluation::evaluate(m_Board);
		m_MovesHistory.emplace_back(move.first, move.second, m_Board);
	}

	m_Listener(m_Board.state, true, {});
}

Piece::MaxMovesVector BoardManager::getPossibleMoves(const Pos &selectedPos)
{
	const Piece &piece = m_Board[selectedPos];
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
	assert(selectedPos.isValid() && destPos.isValid());
	m_Board.whiteToMove = !m_Board.whiteToMove;
	m_Board.isPromotion = m_Board.isCapture = false;
	bool shouldRedraw = false;

	const Pos enPassantPos = m_Board.enPassantPos;
	m_Board.enPassantPos = Pos();

	StackVector<PosPair, 2> piecesMoved{ {selectedPos, destPos} };

	Piece &selectedPiece = m_Board[selectedPos];

	if (selectedPiece.type == Type::PAWN)
		shouldRedraw = movePawn(m_Board, selectedPos, destPos, enPassantPos);
	else if (selectedPiece.type == Type::KING)
	{
		m_Board.kingSquare[selectedPiece.isWhite] = destPos.toSquare();

		if (!selectedPiece.moved)
		{
			const PosPair &posPair = piecesMoved.emplace_back(moveKing(selectedPiece, selectedPos, destPos, m_Board));
			if (posPair.first.isValid())
			{
				if (selectedPiece.isWhite)
					m_Board.whiteCastled = true;
				else
					m_Board.blackCastled = true;

				U64 &pieces = m_Board.pieces[selectedPiece.isWhite];
				pieces &= ~posPair.first.toBitboard();
				pieces |= posPair.second.toBitboard();
			}
		}
	}

	m_Board.pieces[selectedPiece.isWhite] &= ~selectedPos.toBitboard(); // Remove selected Piece
	m_Board.pieces[selectedPiece.isWhite] |= destPos.toBitboard(); // Add the selected Piece to destination

	selectedPiece.moved = true;

	if (const Piece &destPiece = m_Board[destPos]; destPiece)
	{
		m_Board.pieces[destPiece.isWhite] &= ~destPos.toBitboard(); // Remove destination Piece
		m_Board.npm -= Evaluation::getPieceValue(destPiece.type);
		m_Board.isCapture = true;
	}

	m_Board[destPos] = selectedPiece;
	m_Board[selectedPos] = Piece();

	m_Board.key = Hash::compute(m_Board);
	m_Board.updateState();
	m_Board.score = Evaluation::evaluate(m_Board);

	m_MovesHistory.emplace_back(selectedPos, destPos, m_Board);
	m_Listener(m_Board.state, shouldRedraw, piecesMoved);

	if (movedByPlayer && (m_Board.state == State::NONE || m_Board.state == State::WHITE_IN_CHESS || m_Board.state == State::BLACK_IN_CHESS))
		m_WorkerThread = std::thread(moveComputerPlayer, m_Settings);
}

void BoardManager::movePieceInternal(const Pos &selectedPos, const Pos &destPos, Board &board, const bool updateState)
{
	board.whiteToMove = !board.whiteToMove;
	Piece &selectedPiece = board[selectedPos];
	Piece &destPiece = board[destPos];
	bool hashHandled = false;
	board.isPromotion = board.isCapture = false;

	const Pos enPassantPos = board.enPassantPos;
	board.enPassantPos = Pos();

	if (selectedPiece.type == Type::PAWN)
		hashHandled = movePawn(board, selectedPos, destPos, enPassantPos);
	else if (selectedPiece.type == Type::KING)
	{
		board.kingSquare[selectedPiece.isWhite] = destPos.toSquare();

		if (!selectedPiece.moved)
		{
			const PosPair posPair = moveKing(selectedPiece, selectedPos, destPos, board);
			if (posPair.first.isValid())
			{
				if (selectedPiece.isWhite)
					board.whiteCastled = true;
				else
					board.blackCastled = true;

				Hash::makeMove(board.key, posPair.first, posPair.second, Piece(Type::ROOK, selectedPiece.isWhite));

				U64 &pieces = board.pieces[selectedPiece.isWhite];
				pieces &= ~posPair.first.toBitboard();
				pieces |= posPair.second.toBitboard();
			}
		}
	}

	board.pieces[selectedPiece.isWhite] &= ~selectedPos.toBitboard(); // Remove selected Piece
	board.pieces[selectedPiece.isWhite] |= destPos.toBitboard(); // Add the selected Piece to destination

	Hash::flipSide(board.key);
	if (!hashHandled)
		Hash::makeMove(board.key, selectedPos, destPos, selectedPiece, destPiece);

	if (destPiece)
	{
		board.pieces[destPiece.isWhite] &= ~destPos.toBitboard(); // Remove destination Piece
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
    m_IsWorking = true;
	Stats::resetStats();
	Stats::startTimer();

	const RootMove bestMove = NegaMax::getBestMove(m_Board, !m_IsPlayerWhite, settings);

	Stats::stopTimer();
	m_IsWorking = false;
	movePiece(bestMove.start, bestMove.dest, false);

	m_WorkerThread.detach();
}

bool BoardManager::movePawn(Board &board, const Pos &startPos, const Pos &destPos, const Pos &enPassantPos)
{
	Piece &pawn = board[startPos];

	if (pawn.moved) {
		if (destPos.y == 0 || destPos.y == 7)
		{
			pawn.type = Type::QUEEN;
			board.isPromotion = true;

			Hash::promotePawn(board.key, startPos, destPos, pawn.isWhite, Type::QUEEN);
			return true;
		} else if (destPos == enPassantPos) {
			board.isCapture = true;

			const Pos capturedPos(enPassantPos.x, enPassantPos.y + static_cast<byte>(pawn.isWhite ? -1 : 1));
			Piece &capturedPiece = board[capturedPos];

			// Remove the captured Pawn
			Hash::xorPiece(board.key, capturedPos, capturedPiece);
			board.pieces[!pawn.isWhite] = ~capturedPos.toBitboard();
			board.npm -= Evaluation::getPieceValue(Type::PAWN);
			capturedPiece = Piece();
			return true;
		}
	} else {
		const int distance = static_cast<int>(destPos.y) - static_cast<int>(startPos.y);
		if (distance == 2 || distance == -2)
			board.enPassantPos = Pos(destPos.x, destPos.y - static_cast<byte>(distance / 2));
	}

	return false;
}

PosPair BoardManager::moveKing(Piece &king, const Pos &selectedPos, const Pos &destPos, Board &board)
{
	if (destPos.x == 6u)
	{
		constexpr byte startX = 7u;
		const byte y = selectedPos.y;

		Piece &rook = board.getPiece(startX, y);
		if (rook.type == Type::ROOK && king.isSameColor(rook) && !rook.moved)
		{
			rook.moved = true;

			constexpr byte destX = 5;
			board.getPiece(destX, y) = rook;
			board.getPiece(startX, y) = Piece::EMPTY;

			return std::make_pair(Pos(startX, y), Pos(destX, y));
		}
	}
	else if (destPos.x == 2u)
	{
		constexpr byte startX = 0u;
		const byte y = selectedPos.y;

		Piece &rook = board.getPiece(startX, y);
		if (rook.type == Type::ROOK && king.isSameColor(rook) && !rook.moved)
		{
			rook.moved = true;

			constexpr byte destX = 3u;
			board.getPiece(destX, y) = rook;
			board.getPiece(startX, y) = Piece::EMPTY;

			return std::make_pair(Pos(startX, y), Pos(destX, y));
		}
	}

	return std::make_pair(Pos(), Pos());
}

void BoardManager::undoLastMoves()
{
	if (isWorking() || m_MovesHistory.size() < 3) return;

	const auto end = m_MovesHistory.end();
	// Undo the last move, which should have been made by the engine
	const RootMove &engineMove = m_MovesHistory.back();
	const Board &engineBoard = engineMove.board;

	// Undo the move before the last move so that it is the player's turn again
	const RootMove &playerMove = *(end - 2);
	const Board &playerBoard = playerMove.board;

	// Restore the move before the last two moves
	const RootMove &previousMove = *(end - 3);
	const Board &previousBoard = previousMove.board;

	const bool shouldRedraw = engineBoard.isPromotion || engineBoard.isCapture ||
			playerBoard.isPromotion || playerBoard.isCapture ||
			engineBoard.whiteCastled != previousBoard.whiteCastled ||
			engineBoard.blackCastled != previousBoard.whiteCastled;

	m_Board = previousBoard;
	m_Listener(previousBoard.state, shouldRedraw,
			{ { engineMove.dest, engineMove.start }, { playerMove.dest, playerMove.start } });

	// Remove the last two moves from the vector
	m_MovesHistory.pop_back();
	m_MovesHistory.pop_back();
}
