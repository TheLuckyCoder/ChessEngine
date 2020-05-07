#include "MoveGen.h"

#include "PieceAttacks.h"

namespace
{
	template <Color Us>
	Move *generatePawnMoves(const Board &board, Move *moveList, const U64 targets)
	{
		using namespace Bits;

		constexpr Piece Piece{ PAWN, Us };
		constexpr Color Them = ~Us;
		constexpr Dir Forward = Us == WHITE ? NORTH : SOUTH;
		constexpr U64 StartRank = Us == WHITE ? RANK_2 : RANK_7;
		constexpr U64 LastRank = Us == WHITE ? RANK_7 : RANK_2;

		const auto addQuietMove = [&](const byte from, const byte to, const U64 pos)
		{
			Move move{ from, to, PAWN };

			if (LastRank & pos)
			{
				move.setFlags(Move::PROMOTION);
				for (byte promotionType = KNIGHT; promotionType <= QUEEN; ++promotionType)
				{
					move.setPromotedPiece(PieceType(promotionType));
					*moveList++ = move;
				}
			} else
				*moveList++ = move;
		};

		const auto addCaptureMove = [&](const byte from, const byte to, const U64 pos)
		{
			Move move{ from, to, PAWN, Move::CAPTURE };
			move.setCapturedPiece(board.getPiece(to).type());

			if (LastRank & pos)
			{
				move.setFlags(Move::CAPTURE | Move::PROMOTION);
				for (byte promotionType = KNIGHT; promotionType <= QUEEN; ++promotionType)
				{
					move.setPromotedPiece(PieceType(promotionType));
					*moveList++ = move;
				}
			} else
				*moveList++ = move;
		};

		for (byte pieceNumber{}; pieceNumber < board.pieceCount[Piece]; ++pieceNumber)
		{
			const byte from = board.pieceList[Piece][pieceNumber];
			const U64 bb = getSquare64(from);
			U64 attacks = PieceAttacks::pawnAttacks<Us>(bb);

			if (board.enPassantSq < SQ_NONE)
			{
				constexpr Dir EnPassantDirection = Us == WHITE ? SOUTH : NORTH;
				const U64 enPassantCapture = getSquare64(board.enPassantSq);
				const U64 capturedPawn = shift<EnPassantDirection>(enPassantCapture);

				if (board.getType(PAWN, Them) & capturedPawn && (attacks & enPassantCapture))
				{
					attacks &= ~enPassantCapture;
					*moveList++ = { from, board.enPassantSq, PAWN, Move::EN_PASSANT };
				}
			}

			attacks &= targets & board.occupied;

			while (attacks)
			{
				const byte to = findNextSquare(attacks);
				addCaptureMove(from, to, bb);
			}

			const U64 moveBB = shift<Forward>(bb);

			if (!(board.occupied & moveBB))
			{
				addQuietMove(from, bitScanForward(moveBB), bb);

				if (StartRank & bb)
				{
					const U64 doublePushBb = shift<Forward>(moveBB);

					if (!(board.occupied & doublePushBb))
						*moveList++ = { from, bitScanForward(doublePushBb), PAWN,
										Move::DOUBLE_PAWN_PUSH };
				}
			}
		}

		return moveList;
	}

	template <Color Us, PieceType P>
	Move *generatePieceMoves(const Board &board, Move *moveList, const U64 targets)
	{
		static_assert(P != KING && P != PAWN);
		constexpr Color Them = ~Us;
		constexpr Piece Piece{ P, Us };

		for (byte pieceNumber{}; pieceNumber < board.pieceCount[Piece]; ++pieceNumber)
		{
			const byte from = board.pieceList[Piece][pieceNumber];

			U64 attacks{};

			if constexpr (P == KNIGHT)
				attacks = PieceAttacks::knightAttacks(from);
			else if constexpr (P == BISHOP)
				attacks = PieceAttacks::bishopAttacks(from, board.occupied);
			else if constexpr (P == ROOK)
				attacks = PieceAttacks::rookAttacks(from, board.occupied);
			else if constexpr (P == QUEEN)
				attacks = PieceAttacks::queenAttacks(from, board.occupied);

			attacks &= targets;

			while (attacks)
			{
				const byte to = Bits::findNextSquare(attacks);
				const U64 bb = Bits::getSquare64(to);

				Move move{ from, to, P };
				if (bb & board.allPieces[Them])
				{
					move.setFlags(Move::CAPTURE);
					move.setCapturedPiece(board.getPiece(to).type());
				}

				*moveList++ = move;
			}
		}

		return moveList;
	}

	template <Color Us>
	Move *generateAllMoves(const Board &board, Move *moveList, const U64 targets)
	{
		constexpr Color Them = ~Us;

		moveList = generatePawnMoves<Us>(board, moveList, targets);
		moveList = generatePieceMoves<Us, KNIGHT>(board, moveList, targets);
		moveList = generatePieceMoves<Us, BISHOP>(board, moveList, targets);
		moveList = generatePieceMoves<Us, ROOK>(board, moveList, targets);
		moveList = generatePieceMoves<Us, QUEEN>(board, moveList, targets);

		// King Moves
		constexpr Piece KingPiece{ KING, Us };
		const byte kingSquare = board.pieceList[KingPiece][0];
		assert(kingSquare < SQUARE_NB);

		{
			U64 attacks = PieceAttacks::kingAttacks(kingSquare) & targets;
			while (attacks)
			{
				const byte to = Bits::findNextSquare(attacks);
				const U64 bb = Bits::getSquare64(to);

				Move move{ kingSquare, to, KING };
				if (bb & board.allPieces[Them])
				{
					move.setFlags(Move::CAPTURE);
					move.setCapturedPiece(board.getPiece(to).type());
				}
				*moveList++ = move;
			}
		}

		if (board.canCastle<Us>() && !board.isInCheck<Us>())
		{
			const byte y = row(kingSquare);
			const auto isEmptyAndCheckFree = [&board, y](const byte x)
			{
				const byte sq = toSquare(x, y);
				return !(board.occupied & Bits::getSquare64(sq))
					   && !board.isAttackedByAny(Them, sq);
			};

			// King Side
			if (board.canCastleKs<Us>()
				&& isEmptyAndCheckFree(5)
				&& isEmptyAndCheckFree(6))
			{
				*moveList++ = { kingSquare, toSquare(6, y), KING, Move::KSIDE_CASTLE };
			}

			// Queen Side
			if (board.canCastleQs<Us>()
				&& isEmptyAndCheckFree(3)
				&& isEmptyAndCheckFree(2)
				&& !(board.occupied & Bits::getSquare64(toSquare(1, y))))
			{
				*moveList++ = { kingSquare, toSquare(2, y), KING, Move::QSIDE_CASTLE };
			}
		}

		return moveList;
	}
}

Move *generateMoves(const Board &board, Move *moveList) noexcept
{
	const Color color = board.colorToMove;
	const U64 targets = ~board.allPieces[color]; // Remove our pieces

	return color == WHITE
		   ? generateAllMoves<WHITE>(board, moveList, targets)
		   : generateAllMoves<BLACK>(board, moveList, targets);
}
