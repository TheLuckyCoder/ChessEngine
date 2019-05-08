#include "BoardManager.h"

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
	m_MovesHistory.reserve(100);
	m_MovesHistory.emplace_back(Pos(), Pos(), m_Board);

	Stats::resetStats();

	m_IsPlayerWhite = isPlayerWhite;

	if (!isPlayerWhite)
	    m_WorkerThread = std::thread(moveComputerPlayer, m_Settings);
}

void BoardManager::loadGame(const std::vector<PosPair> &moves)
{
	m_Board.initDefaultBoard();
	m_Board.key = Hash::compute(m_Board);

	m_MovesHistory.emplace_back(Pos(), Pos(), m_Board);

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

	if (selectedPiece.type == Type::PAWN)
		m_Board.isPromotion = movePawn(selectedPiece, destPos);
	else if (selectedPiece.type == Type::KING)
	{
		m_Board.kingSquare[selectedPiece.isWhite] = destPos.toSquare();

		if (!selectedPiece.moved)
		{
			const auto &posPair = piecesMoved.emplace_back(moveKing(selectedPiece, selectedPos, destPos, m_Board));
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

	if (const auto &destPiece = m_Board[destPos]; destPiece)
	{
		m_Board.pieces[destPiece.isWhite] &= ~destPos.toBitboard(); // Remove destination Piece
		m_Board.npm -= Evaluation::getPieceValue(destPiece.type);
		m_Board.isCapture = true;
	}

	m_Board[destPos] = selectedPiece;
	m_Board[selectedPos] = Piece();

	m_Board.updateState();
	m_Board.key = Hash::compute(m_Board);
	m_Board.score = Evaluation::evaluate(m_Board);

	m_MovesHistory.emplace_back(selectedPos, destPos, m_Board);
	m_Listener(m_Board.state, m_Board.isPromotion, piecesMoved); // Redraw the pieces if there is a promotion

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
		board.kingSquare[selectedPiece.isWhite] = destPos.toSquare();

		if (!selectedPiece.moved)
		{
			if (const auto posPair = moveKing(selectedPiece, selectedPos, destPos, board); posPair.first.isValid())
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

	const auto pair = NegaMax::getBestMove(m_Board, !m_IsPlayerWhite, settings);

	Stats::stopTimer();
	m_IsWorking = false;
	movePiece(pair.first, pair.second, false);

	m_WorkerThread.detach();
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

void BoardManager::undoLastMoves()
{
	if (isWorking() || m_MovesHistory.size() < 3) return;

	const auto end = m_MovesHistory.end();
	// Undo the last move, which should have been made by the engine
	const RootMove &engineMove = m_MovesHistory.back();
	// Undo the move before the last move so that it is the player's turn again
	const RootMove &playerMove = *(end - 2);

	const RootMove &previousMove = *(end - 3);
	const bool shouldRedraw = engineMove.board.isPromotion || engineMove.board.isCapture ||
			playerMove.board.isPromotion || playerMove.board.isCapture;

	m_Board = previousMove.board;
	m_Listener(previousMove.board.state, shouldRedraw,
			{ { engineMove.dest, engineMove.start }, { playerMove.dest, playerMove.start } });

	m_MovesHistory.pop_back();
	m_MovesHistory.pop_back();
}
