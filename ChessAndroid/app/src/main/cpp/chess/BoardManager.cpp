#include "BoardManager.h"

#include <cassert>

#include "Stats.h"
#include "data/Board.h"
#include "algorithm/Evaluation.h"
#include "algorithm/Hash.h"
#include "algorithm/NegaMax.h"
#include "algorithm/PieceAttacks.h"

Settings BoardManager::s_Settings(4u, std::thread::hardware_concurrency() - 1u, 100, true);
BoardManager::PieceChangeListener BoardManager::s_Listener;
Board BoardManager::s_Board;
std::vector<RootMove> BoardManager::s_MovesHistory;

void BoardManager::initBoardManager(const PieceChangeListener &listener, const bool isPlayerWhite)
{
    Hash::init();
	PieceAttacks::init();

	s_Board.initDefaultBoard();
	s_Listener = listener;

	s_MovesHistory.clear();
	s_MovesHistory.reserve(200);
	s_MovesHistory.emplace_back(Pos(), Pos(), s_Board);

	Stats::resetStats();

	s_IsPlayerWhite = isPlayerWhite;

	if (!isPlayerWhite)
	    s_WorkerThread = std::thread(moveComputerPlayer, s_Settings);
}

void BoardManager::loadGame(const std::vector<PosPair> &moves, const bool isPlayerWhite)
{
	s_IsPlayerWhite = isPlayerWhite;

	s_Board.initDefaultBoard();

	s_MovesHistory.emplace_back(Pos(), Pos(), s_Board);

	for (const PosPair &move : moves)
	{
		movePieceInternal(move.first, move.second, s_Board);
		s_Board.score = Evaluation::evaluate(s_Board);
		s_MovesHistory.emplace_back(move.first, move.second, s_Board);
	}

	s_Listener(s_Board.state, true, {});
}

Piece::MaxMovesVector BoardManager::getPossibleMoves(const Pos &selectedPos)
{
	Piece::MaxMovesVector moves;

	const Piece &piece = s_Board[selectedPos];
	const auto possibleMoves = piece.getPossibleMoves(selectedPos, s_Board);

	for (const Pos &destPos : possibleMoves)
	{
		const Piece &destPiece = s_Board[destPos];
		if (destPiece.type == Type::KING)
			continue;

		Board board = s_Board;
		BoardManager::movePieceInternal(selectedPos, destPos, board);

		if (board.state == State::INVALID)
			continue;
		if (piece.isWhite && (board.state == State::WHITE_IN_CHECK || board.state == State::WINNER_BLACK))
			continue;
		if (!piece.isWhite && (board.state == State::BLACK_IN_CHECK || board.state == State::WINNER_WHITE))
			continue;

		int count = 1;

		for (const auto &game : BoardManager::getMovesHistory()) {
			if (board.whiteToMove == game.board.whiteToMove &&
				board.state == game.board.state &&
				board.key == game.board.key)
				count++;

			if (count == 3)
			{
				board.score = 0;
				board.state = State::DRAW;
				break;
			}
		}

		moves.emplace_back(destPos);
	}

	return moves;
}

void BoardManager::movePiece(const Pos &selectedPos, const Pos &destPos, const bool movedByPlayer)
{
	assert(selectedPos.isValid() && destPos.isValid());
	s_Board.whiteToMove = !s_Board.whiteToMove;
	s_Board.isPromotion = s_Board.isCapture = false;
	bool shouldRedraw = false;

	const Pos enPassantPos = s_Board.enPassantPos;
	s_Board.enPassantPos = Pos();
	Piece &selectedPiece = s_Board[selectedPos];

	StackVector<PosPair, 2> piecesMoved{ {selectedPos, destPos} };

	const U64 selectedPosBitboard = selectedPos.toBitboard();
	const U64 destPosBitboard = destPos.toBitboard();
	const bool selectedPieceColor = selectedPiece.isWhite;

	switch (selectedPiece.type)
	{
		case PAWN:
			shouldRedraw = movePawn(s_Board, selectedPos, destPos, enPassantPos);
			s_Board.pawns[selectedPieceColor] &= ~selectedPosBitboard;
			s_Board.pawns[selectedPieceColor] |= destPosBitboard;
			break;
		case KNIGHT:
			s_Board.knights[selectedPieceColor] &= ~selectedPosBitboard;
			s_Board.knights[selectedPieceColor] |= destPosBitboard;
			break;
		case BISHOP:
			s_Board.bishops[selectedPieceColor] &= ~selectedPosBitboard;
			s_Board.bishops[selectedPieceColor] |= destPosBitboard;
			break;
		case ROOK:
			s_Board.rooks[selectedPieceColor] &= ~selectedPosBitboard;
			s_Board.rooks[selectedPieceColor] |= destPosBitboard;
			break;
		case QUEEN:
			s_Board.queens[selectedPieceColor] &= ~selectedPosBitboard;
			s_Board.queens[selectedPieceColor] |= destPosBitboard;
			break;
		case KING:
		{
			s_Board.kingSquare[selectedPiece.isWhite] = destPos.toSquare();

			if (!selectedPiece.moved)
			{
				const PosPair &posPair = piecesMoved.emplace_back(moveKing(selectedPiece, selectedPos, destPos, s_Board));
				if (posPair.first.isValid())
				{
					if (selectedPiece.isWhite)
						s_Board.whiteCastled = true;
					else
						s_Board.blackCastled = true;

					U64 &pieces = s_Board.allPieces[selectedPiece.isWhite];
					pieces &= ~posPair.first.toBitboard();
					pieces |= posPair.second.toBitboard();
				}
			}
			break;
		}
		case NONE:
			break;
	}

	s_Board.allPieces[selectedPiece.isWhite] &= ~selectedPosBitboard; // Remove selected Piece
	s_Board.allPieces[selectedPiece.isWhite] |= destPosBitboard; // Add the selected Piece to destination

	selectedPiece.moved = true;

	if (const Piece &destPiece = s_Board[destPos]; destPiece)
	{
		s_Board.allPieces[destPiece.isWhite] &= ~destPos.toBitboard(); // Remove destination Piece
		s_Board.npm -= Evaluation::getPieceValue(destPiece.type);
		s_Board.isCapture = true;
	}

	s_Board[destPos] = selectedPiece;
	s_Board[selectedPos] = Piece();

	s_Board.key = Hash::compute(s_Board);
	s_Board.updateState();
	s_Board.score = Evaluation::evaluate(s_Board);

	s_MovesHistory.emplace_back(selectedPos, destPos, s_Board);
	s_Listener(s_Board.state, shouldRedraw, piecesMoved);

	if (movedByPlayer && (s_Board.state == State::NONE || s_Board.state == State::WHITE_IN_CHECK || s_Board.state == State::BLACK_IN_CHECK))
		s_WorkerThread = std::thread(moveComputerPlayer, s_Settings);
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

	const U64 selectedPosBitboard = selectedPos.toBitboard();
	const U64 destPosBitboard = destPos.toBitboard();
	const bool selectedPieceColor = selectedPiece.isWhite;

	switch (selectedPiece.type)
	{
		case PAWN:
			hashHandled = movePawn(board, selectedPos, destPos, enPassantPos);
			board.pawns[selectedPieceColor] &= ~selectedPosBitboard;
			board.pawns[selectedPieceColor] |= destPosBitboard;
			break;
		case KNIGHT:
			board.knights[selectedPieceColor] &= ~selectedPosBitboard;
			board.knights[selectedPieceColor] |= destPosBitboard;
			break;
		case BISHOP:
			board.bishops[selectedPieceColor] &= ~selectedPosBitboard;
			board.bishops[selectedPieceColor] |= destPosBitboard;
			break;
		case ROOK:
			board.rooks[selectedPieceColor] &= ~selectedPosBitboard;
			board.rooks[selectedPieceColor] |= destPosBitboard;
			break;
		case QUEEN:
			board.queens[selectedPieceColor] &= ~selectedPosBitboard;
			board.queens[selectedPieceColor] |= destPosBitboard;
			break;
		case KING:
		{
			board.kingSquare[selectedPieceColor] = destPos.toSquare();

			if (!selectedPiece.moved)
			{
				const PosPair posPair = moveKing(selectedPiece, selectedPos, destPos, board);
				if (posPair.first.isValid()) // Castling
				{
					if (selectedPiece.isWhite)
						board.whiteCastled = true;
					else
						board.blackCastled = true;

					Hash::makeMove(board.key, posPair.first, posPair.second, Piece(Type::ROOK, selectedPieceColor));

					U64 &pieces = board.allPieces[selectedPieceColor];
					pieces &= ~posPair.first.toBitboard();
					pieces |= posPair.second.toBitboard();
				}
			}
			break;
		}
		case NONE:
			break;
	}

	board.allPieces[selectedPieceColor] &= ~selectedPosBitboard; // Remove selected Piece
	board.allPieces[selectedPieceColor] |= destPosBitboard; // Add the selected Piece to destination

	Hash::flipSide(board.key);
	if (!hashHandled)
		Hash::makeMove(board.key, selectedPos, destPos, selectedPiece, destPiece);

	if (destPiece)
	{
		board.allPieces[destPiece.isWhite] &= ~destPos.toBitboard(); // Remove destination Piece
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
    s_IsWorking = true;
	Stats::resetStats();
	Stats::startTimer();

	assert(s_Board.whiteToMove != s_IsPlayerWhite);
	const RootMove bestMove = NegaMax::getBestMove(s_Board, settings);

	Stats::stopTimer();
	s_IsWorking = false;
	movePiece(bestMove.start, bestMove.dest, false);

	s_WorkerThread.detach();
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
			board.allPieces[!pawn.isWhite] = ~capturedPos.toBitboard();
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
	if (isWorking() || s_MovesHistory.size() < 3) return;

	const auto end = s_MovesHistory.end();
	// Undo the last move, which should have been made by the engine
	const RootMove &engineMove = s_MovesHistory.back();
	const Board &engineBoard = engineMove.board;

	// Undo the move before the last move so that it is the player's turn again
	const RootMove &playerMove = *(end - 2);
	const Board &playerBoard = playerMove.board;

	// Restore the move before the last two moves
	const RootMove &previousMove = *(end - 3);
	const Board &previousBoard = previousMove.board;

	// Redraw if a Promotion or castling happened in the last three moves
	const bool shouldRedraw = engineBoard.isPromotion || engineBoard.isCapture ||
			playerBoard.isPromotion || playerBoard.isCapture ||
			engineBoard.whiteCastled != previousBoard.whiteCastled ||
			engineBoard.blackCastled != previousBoard.whiteCastled;

	s_Board = previousBoard;
	s_Listener(previousBoard.state, shouldRedraw,
			{ { engineMove.dest, engineMove.start }, { playerMove.dest, playerMove.start } });

	// Remove the last two moves from the vector
	s_MovesHistory.pop_back();
	s_MovesHistory.pop_back();
}
