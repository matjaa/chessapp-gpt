#pragma once

#include "chess/Board.h"

namespace chess
{
    class MoveValidator
    {
    public:
        static bool isValidMove(
            const Board& board,
            int fromRow,
            int fromColumn,
            int toRow,
            int toColumn
        );

        static bool isSquareAttacked(
            const Board& board,
            int row,
            int column,
            PieceColor attackingColor
        );

        static bool isKingInCheck(
            const Board& board,
            PieceColor kingColor
        );

        static bool hasLegalMoves(
            const Board& board,
            PieceColor color
        );

        static bool isCheckmate(
            const Board& board,
            PieceColor color
        );

        static bool isStalemate(
            const Board& board,
            PieceColor color
        );

        static bool isValidCastlingMove(
            const Board& board,
            int fromRow,
            int fromColumn,
            int toRow,
            int toColumn
        );

    private:
        static bool isValidPawnMove(
            const Board& board,
            int fromRow,
            int fromColumn,
            int toRow,
            int toColumn
        );

        static bool isValidKnightMove(
            const Board& board,
            int fromRow,
            int fromColumn,
            int toRow,
            int toColumn
        );

        static bool isValidBishopMove(
            const Board& board,
            int fromRow,
            int fromColumn,
            int toRow,
            int toColumn
        );

        static bool isValidRookMove(
            const Board& board,
            int fromRow,
            int fromColumn,
            int toRow,
            int toColumn
        );

        static bool isValidQueenMove(
            const Board& board,
            int fromRow,
            int fromColumn,
            int toRow,
            int toColumn
        );

        static bool isValidKingMove(
            const Board& board,
            int fromRow,
            int fromColumn,
            int toRow,
            int toColumn
        );
    };
}