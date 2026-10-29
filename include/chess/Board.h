#pragma once

#include <array>

#include "chess/Piece.h"

namespace chess
{
    class Board
    {
    public:
        static constexpr int Size = 8;

        Board();

        int getSize() const;

        const Piece& getPiece(int row, int column) const;
        void setPiece(int row, int column, Piece piece);

        void movePiece(int fromRow, int fromColumn, int toRow, int toColumn);

        void promotePawn(
            int row,
            int column,
            PieceType newType
        );

        bool hasWhiteKingMoved() const;
        bool hasBlackKingMoved() const;

        bool hasWhiteKingsideRookMoved() const;
        bool hasWhiteQueensideRookMoved() const;

        bool hasBlackKingsideRookMoved() const;
        bool hasBlackQueensideRookMoved() const;

        void castle(
            int row,
            int kingFromColumn,
            int kingToColumn
        );

        bool hasEnPassantTarget() const;

        int getEnPassantRow() const;
        int getEnPassantColumn() const;

        void setEnPassantTarget(int row, int column);

        void clearEnPassantTarget();

        int getHalfmoveClock() const;

    private:
        std::array<std::array<Piece, Size>, Size> squares;

        void setupStartingPosition();

        bool whiteKingMoved = false;
        bool blackKingMoved = false;

        bool whiteKingsideRookMoved = false;
        bool whiteQueensideRookMoved = false;

        bool blackKingsideRookMoved = false;
        bool blackQueensideRookMoved = false;

        int enPassantRow = -1;
        int enPassantColumn = -1;
        
        int halfmoveClock = 0;
        
    };
}