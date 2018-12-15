#include "QueenPiece.h"

#include "PieceEval.h"

void QueenPiece::calculateMoves(Pos &pos, std::vector <Pos> &moves, const Board &board) const
{
    Pos posCopy = pos;

    // Vertical and Horizontal
    while (posCopy.x > 0)
    {
        posCopy.x--;

        auto *other = board[posCopy];

        if (other)
        {
            if (!hasSameColor(*other))
                moves.push_back(posCopy);
            break;
        }
        else
            moves.push_back(posCopy);
    }

    posCopy = pos;
    while (posCopy.x < 7)
    {
        posCopy.x++;

        auto *other = board[posCopy];

        if (other)
        {
            if (!hasSameColor(*other))
                moves.push_back(posCopy);
            break;
        }
        else
            moves.push_back(posCopy);
    }

    posCopy = pos;
    while (posCopy.y > 0)
    {
        posCopy.y--;

        auto *other = board[posCopy];

        if (other)
        {
            if (!hasSameColor(*other))
                moves.push_back(posCopy);
            break;
        }
        else
            moves.push_back(posCopy);
    }

    posCopy = pos;
    while (posCopy.y < 7)
    {
        posCopy.y++;

        auto *other = board[posCopy];

        if (other)
        {
            if (!hasSameColor(*other))
                moves.push_back(posCopy);
            break;
        }
        else
            moves.push_back(posCopy);
    }

    // Diagonal
    posCopy = pos;
    while (posCopy.x < 7 && posCopy.y > 0)
    {
        posCopy.x++;
        posCopy.y--;

        auto *other = board[posCopy];

        if (other)
        {
            if (!hasSameColor(*other))
                moves.push_back(posCopy);
            break;
        }
        else
            moves.push_back(posCopy);
    }

    posCopy = pos;
    while (posCopy.x < 7 && posCopy.y < 7)
    {
        posCopy.x++;
        posCopy.y++;

        auto *other = board[posCopy];

        if (other)
        {
            if (!hasSameColor(*other))
                moves.push_back(posCopy);
            break;
        }
        else
            moves.push_back(posCopy);
    }

    posCopy = pos;
    while (posCopy.x > 0 && posCopy.y > 0)
    {
        posCopy.x--;
        posCopy.y--;

        auto *other = board[posCopy];

        if (other)
        {
            if (!hasSameColor(*other))
                moves.push_back(posCopy);
            break;
        }
        else
            moves.push_back(posCopy);
    }

    posCopy = pos;
    while (posCopy.x > 0 && posCopy.y < 7)
    {
        posCopy.x--;
        posCopy.y++;

        auto *other = board[posCopy];

        if (other)
        {
            if (!hasSameColor(*other))
                moves.push_back(posCopy);
            break;
        }
        else
            moves.push_back(posCopy);
    }
}

int QueenPiece::evaluatePiece(const int x, const int y) const
{
    return PieceEval::QUEEN + (isMaximising() ? PieceEval::QUEEN_WHITE[x][y] : PieceEval::QUEEN_BLACK[x][y]);
}
