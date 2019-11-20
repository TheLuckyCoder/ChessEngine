#include "FenParser.h"

#include "../data/Board.h"
#include "../algorithm/Hash.h"

FenParser::FenParser(Board &board)
	: board(board)
{
}

void FenParser::parseFen(const std::string &fen) noexcept
{
	std::istringstream fenStream(fen);

	// Clean board
	board.data.fill({});
	board.pieces.fill({});
	board.occupied = 0ull;

	parsePieces(fenStream);

	// Next to move
	std::string token;
	fenStream >> token;
	board.colorToMove = token == "w" ? WHITE : BLACK;

	// Castling availability
	fenStream >> token;
	board.castlingRights = CastlingRights::CASTLE_NONE;
	for (auto &currChar : token) {
		switch (currChar) {
			case 'K': board.castlingRights |= CastlingRights::CASTLE_WHITE_KING;
				break;
			case 'Q': board.castlingRights |= CastlingRights::CASTLE_WHITE_QUEEN;
				break;
			case 'k': board.castlingRights |= CastlingRights::CASTLE_BLACK_KING;
				break;
			case 'q': board.castlingRights |= CastlingRights::CASTLE_BLACK_QUEEN;
				break;
			default:
				break;
		}
	}

	// TODO: En passant target square
	fenStream >> token;
	//board.enPassant = token == "-" ? 0ull : 1 << x;

	// Halfmove clock
	fenStream >> board.halfMoveClock;

	board.updateNonPieceBitboards();
	board.zKey = Hash::compute(board);
}

std::string FenParser::exportToFen() noexcept
{
	return std::string();
}

void FenParser::parsePieces(std::istringstream &stream) noexcept
{
	std::string token;

	U64 boardPos = 56ull; // Fen string starts at a8 = index 56
	stream >> token;
	for (auto currChar : token)
	{
		switch (currChar)
		{
			case 'p':
				board.data[boardPos] = Piece(PAWN, BLACK);
				board.getType(BLACK, PAWN) |= Bitboard::shiftedBoards[boardPos++];
				break;
			case 'r':
				board.data[boardPos] = Piece(ROOK, BLACK);
				board.getType(BLACK, ROOK) |= Bitboard::shiftedBoards[boardPos++];
				break;
			case 'n':
				board.data[boardPos] = Piece(KNIGHT, BLACK);
				board.getType(BLACK, KNIGHT) |= Bitboard::shiftedBoards[boardPos++];
				break;
			case 'b':
				board.data[boardPos] = Piece(BISHOP, BLACK);
				board.getType(BLACK, BISHOP) |= Bitboard::shiftedBoards[boardPos++];
				break;
			case 'q':
				board.data[boardPos] = Piece(QUEEN, BLACK);
				board.getType(BLACK, QUEEN) |= Bitboard::shiftedBoards[boardPos++];
				break;
			case 'k':
				board.data[boardPos] = Piece(KING, BLACK);
				board.getType(BLACK, KING) |= Bitboard::shiftedBoards[boardPos++];
				break;
			case 'P':
				board.data[boardPos] = Piece(PAWN, WHITE);
				board.getType(WHITE, PAWN) |= Bitboard::shiftedBoards[boardPos++];
				break;
			case 'R':
				board.data[boardPos] = Piece(ROOK, WHITE);
				board.getType(WHITE, ROOK) |= Bitboard::shiftedBoards[boardPos++];
				break;
			case 'N':
				board.data[boardPos] = Piece(KNIGHT, WHITE);
				board.getType(WHITE, KNIGHT) |= Bitboard::shiftedBoards[boardPos++];
				break;
			case 'B':
				board.data[boardPos] = Piece(BISHOP, WHITE);
				board.getType(WHITE, BISHOP) |= Bitboard::shiftedBoards[boardPos++];
				break;
			case 'Q':
				board.data[boardPos] = Piece(QUEEN, WHITE);
				board.getType(WHITE, QUEEN) |= Bitboard::shiftedBoards[boardPos++];
				break;
			case 'K':
				board.data[boardPos] = Piece(KING, WHITE);
				board.getType(WHITE, KING) |= Bitboard::shiftedBoards[boardPos++];
				break;
			case '/': boardPos -= 16u; // Go down one rank
				break;
			default:
				boardPos += static_cast<U64>(currChar - '0');
		}
	}
}
