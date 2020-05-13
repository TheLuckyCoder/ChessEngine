#include "FenParser.h"

#include "../algorithm/Evaluation.h"
#include "../data/Bits.h"
#include "../data/Board.h"
#include "../algorithm/Hash.h"

FenParser::FenParser(Board &board)
	: _board(board)
{
}

bool FenParser::parseFen(const std::string &fen)
{
	std::istringstream stream(fen);

	if (!stream) return false;

	// Clean board
	_board.data.fill({});
	_board.pieces.fill({});
	_board.occupied = 0ull;
	_board.npm = 0;

	parsePieces(stream);

	// Next to move
	std::string side;
	stream >> side;
	_board.colorToMove = side == "w" ? WHITE : BLACK;

	// Castling availability
	std::string castling = "-";
	stream >> castling;
	_board.castlingRights = CastlingRights::CASTLE_NONE;
	for (char currChar : castling) {
		switch (currChar) {
			case 'K': _board.castlingRights |= CastlingRights::CASTLE_WHITE_KING;
				break;
			case 'Q': _board.castlingRights |= CastlingRights::CASTLE_WHITE_QUEEN;
				break;
			case 'k': _board.castlingRights |= CastlingRights::CASTLE_BLACK_KING;
				break;
			case 'q': _board.castlingRights |= CastlingRights::CASTLE_BLACK_QUEEN;
				break;
			default:
				break;
		}
	}

	std::string ep = "-";
	stream >> ep;
	_board.enPassantSq = SQUARE_NB;
	if (ep != "-")
	{
		if (ep.length() != 2)
			return false;
		if (!(ep[0] >= 'a' && ep[0] <= 'h'))
			return false;
		if (!((side == "w" && ep[1] == '6') || (side == "b" && ep[1] == '3')))
			return false;

		const byte sq = ::toSquare(static_cast<byte>(ep[0] - 'a'), static_cast<byte>(ep[1] - '1'));
		_board.enPassantSq = sq < SQUARE_NB ? sq : SQUARE_NB;
	}

	// HalfMove Clock
	int halfMove{};
	stream >> halfMove;
	_board.fiftyMoveRule = static_cast<byte>(halfMove);

	_board.updatePieceList();
	_board.updateNonPieceBitboards();
	_board.zKey = Hash::compute(_board);

	_board.kingAttackers = _board.colorToMove ? _board.allKingAttackers<WHITE>() : _board.allKingAttackers<BLACK>();
	return true;
}

std::string FenParser::exportToFen()
{
	return {};
}

void FenParser::parsePieces(std::istringstream &stream)
{
	std::string token;

	U64 boardPos = 56ull; // Fen string starts at a8 = index 56
	stream >> token;
	for (auto currChar : token)
	{
		switch (currChar)
		{
			case 'p':
				_board.data[boardPos++] = { PAWN, BLACK };
				break;
			case 'r':
				_board.data[boardPos++] = { ROOK, BLACK };
				break;
			case 'n':
				_board.data[boardPos++] = { KNIGHT, BLACK };
				break;
			case 'b':
				_board.data[boardPos++] = { BISHOP, BLACK };
				break;
			case 'q':
				_board.data[boardPos++] = { QUEEN, BLACK };
				break;
			case 'k':
				_board.data[boardPos++] = { KING, BLACK };
				break;
			case 'P':
				_board.data[boardPos++] = { PAWN, WHITE };
				break;
			case 'R':
				_board.data[boardPos++] = { ROOK, WHITE };
				break;
			case 'N':
				_board.data[boardPos++] = { KNIGHT, WHITE };
				break;
			case 'B':
				_board.data[boardPos++] = { BISHOP, WHITE };
				break;
			case 'Q':
				_board.data[boardPos++] = { QUEEN, WHITE };
				break;
			case 'K':
				_board.data[boardPos++] = { KING, WHITE };
				break;
			case '/': boardPos -= 16u; // Go down one rank
				break;
			default:
				boardPos += static_cast<U64>(currChar - '0');
		}
	}

	for (byte square = 0u; square < SQUARE_NB; ++square)
	{
		const Piece piece = _board.data[square];
		const U64 bb = Bits::getSquare64(square);

		_board.getType(piece) |= bb;

		if (piece.type() != PAWN)
			_board.npm += Evaluation::getPieceValue(piece.type());
	}
}
