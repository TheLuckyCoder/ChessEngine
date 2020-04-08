#include "MoveGen.h"

#include "PieceAttacks.h"
#include "../data/Pos.h"

namespace
{
	template <Color Us, GenType Type>
	Move *generatePawnMoves(const Board &board, Move *moveList, const U64 targets)
	{
		using namespace Bitboard;

		constexpr Piece piece(PAWN, Us);
		constexpr Color Them = ~Us;
		constexpr Dir direction = (Us == WHITE ? NORTH : SOUTH);
		constexpr U64 startingRank = (Us == WHITE ? RANK_2 : RANK_7);
		constexpr U64 lastRank = (Us == WHITE ? RANK_7 : RANK_2);

		const auto addQuietMove = [&](const byte from, const byte to, const U64 pos)
		{
			Move move(from, to, PAWN);
			if (lastRank & pos)
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

			if (lastRank & pos)
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
			const U64 pos = getSquare64(from);

			U64 attacks = PieceAttacks::getPawnAttacks(Us, from);

			if constexpr (Type == ALL || Type == CAPTURES)
			{
				if (board.enPassantSq < 64u)
				{
					constexpr Dir EN_PASSANT_DIRECTION = (Us == WHITE ? SOUTH : NORTH);
					const U64 enPassantCapture = getSquare64(board.enPassantSq);
					const U64 capturedPawn = shift<EN_PASSANT_DIRECTION>(enPassantCapture);

					if (board.getType(Them, PAWN) & capturedPawn && (attacks & enPassantCapture))
					{
						attacks &= ~enPassantCapture;
						*moveList++ = { from, board.enPassantSq, PAWN, Move::EN_PASSANT };
					}
				}
			}

			attacks &= targets & board.occupied;

			while (attacks)
			{
				const byte to = findNextSquare(attacks);
				addCaptureMove(from, to, pos);
			}

			if constexpr (Type == ALL)
			{
				const U64 moveBB = shift<direction>(pos);

				if (!(board.occupied & moveBB))
				{
					addQuietMove(from, bitScanForward(moveBB), pos);

					if (startingRank & pos)
					{
						const U64 doubleMoveBB = shift<direction>(moveBB);

						if (!(board.occupied & doubleMoveBB))
							*moveList++ = { from, bitScanForward(doubleMoveBB), PAWN, Move::DOUBLE_PAWN_PUSH };
					}
				}
			}
		}

		return moveList;
	}

	template <PieceType P>
	Move *generatePieceMoves(const Board &board, const Color color, Move *moveList, const U64 targets)
	{
		static_assert(P != KING && P != PAWN);
		const Piece piece(P, color);

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
				const byte to = Bitboard::findNextSquare(attacks);
				const PieceType capturedPiece = board.getPiece(to).type();

				Move move(from, to, P);
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

	template <Color Us, GenType Type>
	Move *generateAllMoves(const Board &board, Move *moveList, const U64 targets)
	{
		constexpr Color Them = ~Us;
		constexpr Piece kingPiece(KING, Us);

		moveList = generatePawnMoves<Us, Type>(board, moveList, targets);
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
				const byte to = Bitboard::findNextSquare(attacks);
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

		if constexpr (Type == ALL)
		{
			if (board.canCastle(Us) && !board.isInCheck(Us))
			{
				const byte y = row(kingSquare);
				const auto isEmptyAndCheckFree = [&, y](const byte x)
				{
					return !board.getPiece(x, y) && !board.attackCount(Them, toSquare(x, y));
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
					&& !board.getPiece(1, y))
				{
					*moveList++ = { kingSquare, Pos(2, y).toSquare(), KING, Move::Flag::QSIDE_CASTLE };
				}
			}
		}

		return moveList;
	}
}

template <GenType Type>
Move *generateMoves(const Board &board, Move *moveList) noexcept
{
	const Color color = board.colorToMove;
	U64 targets{};

	if constexpr (Type == ALL)
		targets = ~board.allPieces[color]; // Remove our pieces
	else if constexpr (Type == CAPTURES)
		targets = board.allPieces[~color]; // Keep only their pieces
	else if constexpr (Type == ATTACKS_DEFENSES)
		targets = board.occupied; // Keep only the pieces

	return color == WHITE
		       ? generateAllMoves<WHITE, Type>(board, moveList, targets)
		       : generateAllMoves<BLACK, Type>(board, moveList, targets);
}

// Explicit template instantiations
template Move *generateMoves<ALL>(const Board &, Move *) noexcept;
template Move *generateMoves<CAPTURES>(const Board &, Move *) noexcept;
template Move *generateMoves<ATTACKS_DEFENSES>(const Board &, Move *) noexcept;
