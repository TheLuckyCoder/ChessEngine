#include "MoveGen.h"

#include "PieceAttacks.h"
#include "../data/Pos.h"

namespace
{
	template <Color Us>
	Move *generatePawnMoves(const Board &board, Move *moveList, const U64 targets)
	{
		using namespace Bits;

		constexpr Piece piece{ PAWN, Us };
		constexpr Color Them = ~Us;
		constexpr Dir Forward = Us == WHITE ? NORTH : SOUTH;
		constexpr U64 StartRank = Us == WHITE ? RANK_2 : RANK_7;
		constexpr U64 LastRank = Us == WHITE ? RANK_7 : RANK_2;

		const auto addQuietMove = [&](const byte from, const byte to, const U64 pos)
		{
			Move move(from, to, PAWN);
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
			Move move(from, to, PAWN);
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

		for (byte pieceNumber{}; pieceNumber < board.pieceCount[piece]; ++pieceNumber)
		{
			const byte from = board.pieceList[piece][pieceNumber];
			const U64 bb = getSquare64(from);
			U64 attacks = PieceAttacks::getPawnAttacks(Us, from);

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
					const U64 doubleMoveBB = shift<Forward>(moveBB);

					if (!(board.occupied & doubleMoveBB))
						*moveList++ = { from, bitScanForward(doubleMoveBB), PAWN, Move::DOUBLE_PAWN_PUSH };
				}
			}
		}

		return moveList;
	}

	template <PieceType P>
	Move *generatePieceMoves(const Board &board, const Color color, Move *moveList, const U64 targets)
	{
		static_assert(P != KING && P != PAWN);
		const Piece piece{ P, color };

		for (byte pieceNumber{}; pieceNumber < board.pieceCount[piece]; ++pieceNumber)
		{
			const byte from = board.pieceList[piece][pieceNumber];

			U64 attacks{};

			if constexpr (P == KNIGHT)
				attacks = PieceAttacks::getKnightAttacks(from);
			else if constexpr (P == BISHOP)
				attacks = PieceAttacks::getBishopAttacks(from, board.occupied);
			else if constexpr (P == ROOK)
				attacks = PieceAttacks::getRookAttacks(from, board.occupied);
			else if constexpr (P == QUEEN)
				attacks = PieceAttacks::getQueenAttacks(from, board.occupied);

			attacks &= targets;

			while (attacks)
			{
				const byte to = Bits::findNextSquare(attacks);
				const PieceType capturedPiece = board.getPiece(to).type();

				Move move{ from, to, P };
				if (capturedPiece)
				{
					move.setFlags(Move::CAPTURE);
					move.setCapturedPiece(capturedPiece);
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
		constexpr Piece kingPiece{ KING, Us };

		moveList = generatePawnMoves<Us>(board, moveList, targets);
		moveList = generatePieceMoves<KNIGHT>(board, Us, moveList, targets);
		moveList = generatePieceMoves<BISHOP>(board, Us, moveList, targets);
		moveList = generatePieceMoves<ROOK>(board, Us, moveList, targets);
		moveList = generatePieceMoves<QUEEN>(board, Us, moveList, targets);

		// King Moves
		const byte kingSquare = board.pieceList[kingPiece][0];
		assert(kingSquare < SQUARE_NB);

		{
			U64 attacks = PieceAttacks::getKingAttacks(kingSquare) & targets;
			while (attacks)
			{
				const byte to = Bits::findNextSquare(attacks);
				const PieceType captured = board.getPiece(to).type();

				Move move(kingSquare, to, KING);
				if (captured != NO_PIECE_TYPE)
				{
					move.setFlags(Move::CAPTURE);
					move.setCapturedPiece(captured);
				}
				*moveList++ = move;
			}
		}


		if (board.canCastle(Us) && !board.isInCheck(Us))
		{
			const byte y = row(kingSquare);
			const auto isEmptyAndCheckFree = [&, y](const byte x)
			{
				const byte sq = toSquare(x, y);
				return !board.getPiece(sq) && !board.isAttackedByAny(Them, sq);
			};

			// King Side
			if (board.canCastleKs(Us)
				&& isEmptyAndCheckFree(5)
				&& isEmptyAndCheckFree(6))
			{
				*moveList++ = { kingSquare, Pos(6, y).toSquare(), KING, Move::Flag::KSIDE_CASTLE };
			}

			// Queen Side
			if (board.canCastleQs(Us)
				&& isEmptyAndCheckFree(3)
				&& isEmptyAndCheckFree(2)
				&& !board.getPiece(toSquare(1, y)))
			{
				*moveList++ = { kingSquare, Pos(2, y).toSquare(), KING, Move::Flag::QSIDE_CASTLE };
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
