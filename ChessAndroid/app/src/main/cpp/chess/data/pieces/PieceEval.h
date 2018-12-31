#pragma once

class Board;
class Pos;

namespace PieceEval
{

	int evaluatePawn(bool isMaximizing, const Pos &pos, const Board &board);
	int evaluateKnight(bool isMaximizing, const Pos &pos, const Board &board);
	int evaluateBishop(bool isMaximizing, const Pos &pos, const Board &board);
	int evaluateRook(bool isMaximizing, const Pos &pos, const Board &board);
	int evaluateQueen(bool isMaximizing, const Pos &pos);
	int evaluateKing(bool isMaximizing, const Pos &pos, const Board &board);

};
