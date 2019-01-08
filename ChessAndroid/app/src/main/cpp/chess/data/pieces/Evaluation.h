#pragma once

#include <unordered_map>

#include "../Game.h"
#include "../Pos.h"

class Piece;
class Board;

class Evaluation
{
public:
	Evaluation() = delete;

	static int evaluate(const Board &board);

private:
	static int evaluatePawn(const Piece &piece, const Pos &pos, const Board &board);
	static int evaluateKnight(const Piece &piece, const Pos &pos, const Board &board, GamePhase phase);
	static int evaluateBishop(const Piece &piece, const Pos &pos, const Board &board, GamePhase phase, std::pair<byte, byte> bishopCount);
	static int evaluateRook(const Piece &piece, const Pos &pos, const Board &board, GamePhase phase);
	static int evaluateQueen(const Piece &piece, const Pos &pos, GamePhase phase);
	static int evaluateKing(const Piece &piece, const Pos &pos, const Board &board, const std::unordered_map<Pos, short> &opponentsMoves, GamePhase phase);
};
