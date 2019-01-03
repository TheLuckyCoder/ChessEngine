#pragma once

#include "../GamePhase.h"

class Piece;
class Board;
class Pos;

class Evaluation
{
public:
	Evaluation() = delete;

	static int evaluate(const Board &board);

private:
	static int evaluatePiece(const Piece &piece, const Pos &pos, const Board &board, GamePhase phase);
	static int evaluatePawn(const Piece &piece, const Pos &pos, const Board &board);
	static int evaluateKnight(const Piece &piece, const Pos &pos, const Board &board);
	static int evaluateBishop(const Piece &piece, const Pos &pos, const Board &board);
	static int evaluateRook(const Piece &piece, const Pos &pos, const Board &board);
	static int evaluateQueen(const Piece &piece, const Pos &pos);
	static int evaluateKing(const Piece &piece, const Pos &pos, const Board &board, GamePhase phase);

	static GamePhase determineGamePhase(const Board &board);

};
