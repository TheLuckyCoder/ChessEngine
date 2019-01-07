#include "Evaluation.h"

#include "MoveGen.h"
#include "../Board.h"
#include "../Game.h"

constexpr int PAWN = 100;
constexpr int KNIGHT = 300;
constexpr int BISHOP = 325;
constexpr int ROOK = 500;
constexpr int QUEEN = 975;
constexpr int KING = 32767;

using EvalArray = std::array<std::array<short, 8>, 8>;

constexpr EvalArray reverseCopy(const EvalArray &source)
{
	const auto first = source.begin();
	auto last = source.end();
	auto dest = source;
	auto destFirst = dest.begin();

	while (first != last)
	{
		*(destFirst++) = *(--last);
	}

	return dest;
}

constexpr EvalArray PAWN_WHITE
{ {
	{ 0,   0,   0,   0,   0,   0,   0,  0},
	{50,  50,  50,  50,  50,  50,  50, 50},
	{10,  10,  20,  30,  30,  20,  10, 10},
	{ 5,   5,  10,  30,  30,  10,   5,  5},
	{ 0,   0,   0,  25,  25,   0,   0,  0},
	{ 5,  -5, -10,   0,   0, -10,  -5,  5},
	{ 5,  10,  10, -20, -20,  10,  10,  5},
	{ 0,   0,   0,   0,   0,   0,   0,  0}
} };

constexpr EvalArray KNIGHT_WHITE
{ {
	{-50, -40, -30, -30, -30, -30, -40, -50},
	{-40, -20,   0,   0,   0,   0, -20, -40},
	{-30,   0,  10,  15,  15,  10,   0, -30},
	{-30,   5,  15,  20,  20,  15,   5, -30},
	{-30,   0,  15,  20,  20,  15,   0, -30},
	{-30,   5,  10,  15,  15,  10,   5, -30},
	{-40, -20,   0,   5,   5,   0, -20, -40},
	{-50, -40, -30, -30, -30, -30, -40, -50}
} };

constexpr EvalArray BISHOP_WHITE
{ {
	{-20, -10, -10, -10, -10, -10, -10, -20},
	{-10,   0,   0,   0,   0,   0,   0, -10},
	{-10,   0,   5,  10,  10,   5,   0, -10},
	{-10,   5,   5,  10,  10,   5,   5, -10},
	{-10,   0,  10,  10,  10,  10,   0, -10},
	{-10,  10,  10,  10,  10,  10,  10, -10},
	{-10,   5,   0,   0,   0,   0,   5, -10},
	{-20, -10, -10, -10, -10, -10, -10, -20}
} };

constexpr EvalArray ROOK_WHITE
{ {
	{ 0,  0,  0,  0,  0,  0,  0,  0},
	{ 5, 10, 10, 10, 10, 10, 10,  5},
	{-5,  0,  0,  0,  0,  0,  0, -5},
	{-5,  0,  0,  0,  0,  0,  0, -5},
	{-5,  0,  0,  0,  0,  0,  0, -5},
	{-5,  0,  0,  0,  0,  0,  0, -5},
	{-5,  0,  0,  0,  0,  0,  0, -5},
	{ 0,  0,  0,  5,  5,  0,  0,  0}
} };

constexpr EvalArray QUEEN_WHITE
{ {
	{-20, -10, -10, -5, -5, -10, -10, -20},
	{-10,   0,   0,  0,  0,   0,   0, -10},
	{-10,   0,   5,  5,  5,   5,   0, -10},
	{ -5,   0,   5,  5,  5,   5,   0,  -5},
	{  0,   0,   5,  5,  5,   5,   0,  -5},
	{-10,   5,   5,  5,  5,   5,   0, -10},
	{-10,   0,   5,  0,  0,   0,   0, -10},
	{-20, -10, -10, -5, -5, -10, -10, -20}
} };

constexpr EvalArray KING_WHITE =
{ {
	{-30, -40, -40, -50, -50, -40, -40, -30},
	{-30, -40, -40, -50, -50, -40, -40, -30},
	{-30, -40, -40, -50, -50, -40, -40, -30},
	{-30, -40, -40, -50, -50, -40, -40, -30},
	{-20, -30, -30, -40, -40, -30, -30, -20},
	{-10, -20, -20, -20, -20, -20, -20, -10},
	{ 20,  20,   0,   0,   0,   0,  20,  20},
	{ 20,  30,  10,   0,   0,  10,  30,  20}
} };

constexpr EvalArray KING_WHITE_ENDING =
{ {
	{-50, -40, -30, -20, -20, -30, -40, -50},
	{-30, -20, -10,   0,   0, -10, -20, -30},
	{-30, -10,  20,  30,  30,  20, -10, -30},
	{-30, -10,  20,  30,  30,  20, -10, -30},
	{-30, -10,  30,  40,  40,  30, -10, -30},
	{-30, -10,  20,  30,  30,  20, -10, -30},
	{-30, -30,   0,   0,   0,   0, -30, -30},
	{-50, -30, -30, -30, -30, -30, -30, -50}
} };

constexpr EvalArray PAWN_BLACK = reverseCopy(PAWN_WHITE);
constexpr EvalArray KNIGHT_BLACK = reverseCopy(KNIGHT_WHITE);
constexpr EvalArray BISHOP_BLACK = reverseCopy(BISHOP_WHITE);
constexpr EvalArray ROOK_BLACK = reverseCopy(ROOK_WHITE);
constexpr EvalArray QUEEN_BLACK = reverseCopy(QUEEN_WHITE);
constexpr EvalArray KING_BLACK = reverseCopy(KING_WHITE);
constexpr EvalArray KING_BLACK_ENDING = reverseCopy(KING_WHITE_ENDING);

int Evaluation::evaluate(const Board &board)
{
	int value = 0;
	byte remainingPieces = 0;
	std::pair<byte, byte> bishopCount;

	auto whiteMoves = MoveGen::getAllMovesPerColor(true, board);
	auto blackMoves = MoveGen::getAllMovesPerColor(false, board);

	for (byte x = 0; x < 8; x++)
		for (byte y = 0; y < 8; y++)
			if (board.data[x][y])
			{
				remainingPieces++;
				const Pos pos(x, y);
				const auto &piece = board[pos];

				{
					const auto defendedValue = piece.isWhite ? whiteMoves[pos] : blackMoves[pos];
					const auto attackedValue = piece.isWhite ? blackMoves[pos] : whiteMoves[pos];

					if (defendedValue < attackedValue)
						value -= (attackedValue - defendedValue) * 10;
				}
				
				if (piece.type == Piece::Type::BISHOP)
				{
					if (piece.isWhite) bishopCount.first++; else bishopCount.second++;
				}
			}

	const auto phase = remainingPieces < 10 ? GamePhase::ENDING : GamePhase::MIDDLE;
	short count = 0;

	for (byte x = 0; x < 8; x++)
		for (byte y = 0; y < 8; y++)
			if (const auto &piece = board.data[x][y]; piece)
			{
				const Pos pos(x, y);
				int points;
				switch (piece.type)
				{
				case Piece::Type::PAWN:
					points = evaluatePawn(piece, pos, board);
					break;
				case Piece::Type::KNIGHT:
					points = evaluateKnight(piece, pos, board, phase);
					break;
				case Piece::Type::BISHOP:
					points = evaluateBishop(piece, pos, board, phase, bishopCount);
					break;
				case Piece::Type::ROOK:
					points = evaluateRook(piece, pos, board, phase);
					break;
				case Piece::Type::QUEEN:
					points = evaluateQueen(piece, pos, phase);
					break;
				case Piece::Type::KING:
					if (piece.isWhite)
						points = evaluateKing(piece, pos, board, blackMoves, phase);
					else
						points = evaluateKing(piece, pos, board, whiteMoves, phase);
					break;
				default:
					points = 0;
					break;
				}

				if (piece.isWhite)
				{
					value += points;
					count++;
				}
				else
				{
					value -= points;
					count--;
				}
			}

	value += count * 40;

	if (board.state == GameState::BLACK_IN_CHESS)
	{
		value += 60;
		if (phase == GamePhase::ENDING)
			value += 10;
	}
	else if (board.state == GameState::WHITE_IN_CHESS)
	{
		value -= 60;
		if (phase == GamePhase::ENDING)
			value -= 10;
	}

	return value;
}

int Evaluation::evaluatePawn(const Piece &piece, const Pos &pos, const Board &board)
{
	int value = PAWN + (piece.isWhite ? PAWN_WHITE[pos.y][pos.x] : PAWN_BLACK[pos.y][pos.x]);

	// Rook Pawns are worth 15% less because they can only attack one way
	if (pos.x == 0 || pos.x == 7)
		value -= 15;

	bool isolated = true;

	if (const auto &pieceAbove = board.data[pos.x][pos.y + 1];
		piece.hasSameColor(pieceAbove) && pieceAbove.type == Piece::Type::PAWN)
	{
		value -= 16;
		isolated = false;
	} else if (const auto &pieceBelow = board.data[pos.x][pos.y - 1];
		piece.hasSameColor(pieceBelow) && pieceBelow.type == Piece::Type::PAWN)
	{
		value -= 16;
		isolated = false;
	}

	const std::array<Pos, 6> nearby {
		pos + Pos(0, 1),
		pos + Pos(1, 1),
		pos + Pos(1, -1),
		pos + Pos(-1, -1),
		pos + Pos(0, -1),
		pos + Pos(-1, 1),
	};

	if (isolated)
	{
		for (const auto &p : nearby)
		{
			if (p.isValid() && board[p].type == Piece::Type::PAWN)
			{
				isolated = false;
				break;
			}
		}
	}

	if (isolated)
		value -= 12;

	value += MoveGen::generatePawnMoves(piece, pos, board).size();

	if ((piece.isWhite && pos.y == 6) ||
		(!piece.isWhite && pos.y == 1))
		value += 100;

	return value;
}

int Evaluation::evaluateKnight(const Piece &piece, const Pos &pos, const Board &board, const GamePhase phase)
{
	int value = KNIGHT + (piece.isWhite ? KNIGHT_WHITE[pos.y][pos.x] : KNIGHT_BLACK[pos.y][pos.x]);

	value += MoveGen::generateKnightMoves(piece, pos, board).size();

	// In the end game remove a few points for Knights since they are worth less
	if (phase == GamePhase::ENDING)
		value -= 10;

	return value;
}

int Evaluation::evaluateBishop(const Piece &piece, const Pos &pos, const Board &board, const GamePhase phase, std::pair<byte, byte> bishopCount)
{
	int value = BISHOP + (piece.isWhite ? BISHOP_WHITE[pos.y][pos.x] : BISHOP_BLACK[pos.y][pos.x]);

	value += MoveGen::generateBishopMoves(piece, pos, board).size();

	// 2 Bishops receive a bonus
	if ((piece.isWhite && bishopCount.first >= 2) ||
		(!piece.isWhite && bishopCount.second >= 2))
		value += 10;

	// In the end game Bishops are worth more
	if (phase == GamePhase::ENDING)
		value += 10;

	return value;
}

int Evaluation::evaluateRook(const Piece &piece, const Pos &pos, const Board &board, const GamePhase phase)
{
	int value = ROOK + (piece.isWhite ? ROOK_WHITE[pos.y][pos.x] : ROOK_BLACK[pos.y][pos.x]);

	value += MoveGen::generateRookMoves(piece, pos, board).size();

	if (piece.moved && phase != GamePhase::ENDING)
		if ((piece.isWhite && !board.whiteCastled) ||
			(!piece.isWhite && !board.blackCastled))
			value -= 10;

	return value;
}

int Evaluation::evaluateQueen(const Piece &piece, const Pos &pos, const GamePhase phase)
{
	int value = QUEEN + (piece.isWhite ? QUEEN_WHITE[pos.y][pos.x] : QUEEN_BLACK[pos.y][pos.x]);

	if (piece.moved && phase != GamePhase::ENDING)
		value -= 15;

	return value;
}

int Evaluation::evaluateKing(const Piece &piece, const Pos &pos, const Board &board, const std::unordered_map<Pos, short> &opponentsMoves, const GamePhase phase)
{
	int value = KING;

	if (phase == GamePhase::ENDING)
		value += piece.isWhite ? KING_WHITE_ENDING[pos.y][pos.x] : KING_BLACK_ENDING[pos.y][pos.x];
	else
	{
		value += piece.isWhite ? KING_WHITE[pos.y][pos.x] : KING_BLACK[pos.y][pos.x];
		
		if (piece.moved)
			if ((piece.isWhite && !board.whiteCastled) ||
				(!piece.isWhite && !board.blackCastled))
				value -= 30;
	}

	auto moves = MoveGen::generateKingInitialMoves(pos);
	const auto iterator = std::remove_if(moves.begin(), moves.end(), [&](const Pos &destPos) {
		return opponentsMoves.find(destPos) == opponentsMoves.end();
	});
	moves.erase(iterator, moves.end());

	if (moves.size() < 2)
		value -= 5;

	return value;
}
