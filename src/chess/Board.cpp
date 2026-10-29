#include "chess/Board.h"

#include <cstdlib>

namespace chess
{
    Board::Board()
    {
        setupStartingPosition();
    }

    int Board::getSize() const
    {
        return Size;
    }

    const Piece& Board::getPiece(int row, int column) const
    {
        return squares[row][column];
    }

    void Board::setPiece(int row, int column, Piece piece)
    {
        squares[row][column] = piece;
    }

    void Board::setupStartingPosition()
    {
        for (auto& row : squares)
        {
            for (auto& square : row)
            {
                square = Piece{};
            }
        }

        // Black pieces
        squares[0][0] = {PieceType::Rook, PieceColor::Black};
        squares[0][1] = {PieceType::Knight, PieceColor::Black};
        squares[0][2] = {PieceType::Bishop, PieceColor::Black};
        squares[0][3] = {PieceType::Queen, PieceColor::Black};
        squares[0][4] = {PieceType::King, PieceColor::Black};
        squares[0][5] = {PieceType::Bishop, PieceColor::Black};
        squares[0][6] = {PieceType::Knight, PieceColor::Black};
        squares[0][7] = {PieceType::Rook, PieceColor::Black};

        for (int column = 0; column < Size; ++column)
        {
            squares[1][column] = {
                PieceType::Pawn,
                PieceColor::Black
            };
        }

        // White pieces
        squares[7][0] = {PieceType::Rook, PieceColor::White};
        squares[7][1] = {PieceType::Knight, PieceColor::White};
        squares[7][2] = {PieceType::Bishop, PieceColor::White};
        squares[7][3] = {PieceType::Queen, PieceColor::White};
        squares[7][4] = {PieceType::King, PieceColor::White};
        squares[7][5] = {PieceType::Bishop, PieceColor::White};
        squares[7][6] = {PieceType::Knight, PieceColor::White};
        squares[7][7] = {PieceType::Rook, PieceColor::White};

        for (int column = 0; column < Size; ++column)
        {
            squares[6][column] = {
                PieceType::Pawn,
                PieceColor::White
            };
        }
    }

    void Board::movePiece(
        int fromRow,
        int fromColumn,
        int toRow,
        int toColumn
    )
    {
        const Piece piece =
            squares[fromRow][fromColumn];

        const bool isEnPassant =
            piece.type == PieceType::Pawn &&
            fromColumn != toColumn &&
            squares[toRow][toColumn].isEmpty() &&
            hasEnPassantTarget() &&
            toRow == enPassantRow &&
            toColumn == enPassantColumn;

        clearEnPassantTarget();

        const bool isCapture =
            !squares[toRow][toColumn].isEmpty() ||
            isEnPassant;

        if (piece.type == PieceType::Pawn || isCapture)
        {
            halfmoveClock = 0;
        }
        else
        {
            ++halfmoveClock;
        }

        if (isEnPassant)
        {
            squares[fromRow][toColumn] = Piece{};
        }

        if (piece.type == PieceType::Pawn &&
            std::abs(toRow - fromRow) == 2)
        {
            const int targetRow =
                (fromRow + toRow) / 2;

            setEnPassantTarget(
                targetRow,
                fromColumn
            );
        }

        if (piece.type == PieceType::King)
        {
            if (piece.color == PieceColor::White)
            {
                whiteKingMoved = true;
            }
            else if (piece.color == PieceColor::Black)
            {
                blackKingMoved = true;
            }
        }

        if (piece.type == PieceType::Rook)
        {
            if (piece.color == PieceColor::White)
            {
                if (fromRow == 7 && fromColumn == 0)
                {
                    whiteQueensideRookMoved = true;
                }
                else if (fromRow == 7 && fromColumn == 7)
                {
                    whiteKingsideRookMoved = true;
                }
            }
            else if (piece.color == PieceColor::Black)
            {
                if (fromRow == 0 && fromColumn == 0)
                {
                    blackQueensideRookMoved = true;
                }
                else if (fromRow == 0 && fromColumn == 7)
                {
                    blackKingsideRookMoved = true;
                }
            }
        }
        
        squares[toRow][toColumn] =
            squares[fromRow][fromColumn];

        squares[fromRow][fromColumn] = Piece{};
    }

    void Board::promotePawn(
        int row,
        int column,
        PieceType newType
    )
    {
        Piece& piece = squares[row][column];

        if (piece.type != PieceType::Pawn)
        {
            return;
        }

        if (newType == PieceType::Pawn ||
            newType == PieceType::King ||
            newType == PieceType::None)
        {
            return;
        }

        piece.type = newType;
    }

    bool Board::hasWhiteKingMoved() const
    {
        return whiteKingMoved;
    }

    bool Board::hasBlackKingMoved() const
    {
        return blackKingMoved;
    }

    bool Board::hasWhiteKingsideRookMoved() const
    {
        return whiteKingsideRookMoved;
    }

    bool Board::hasWhiteQueensideRookMoved() const
    {
        return whiteQueensideRookMoved;
    }

    bool Board::hasBlackKingsideRookMoved() const
    {
        return blackKingsideRookMoved;
    }

    bool Board::hasBlackQueensideRookMoved() const
    {
        return blackQueensideRookMoved;
    }

    void Board::castle(
        int row,
        int kingFromColumn,
        int kingToColumn
    )
    {
        clearEnPassantTarget();

        ++halfmoveClock;
        
        const Piece& king =
            squares[row][kingFromColumn];

        if (king.color == PieceColor::White)
        {
            whiteKingMoved = true;

            if (kingToColumn > kingFromColumn)
            {
                whiteKingsideRookMoved = true;
            }
            else
            {
                whiteQueensideRookMoved = true;
            }
        }
        else if (king.color == PieceColor::Black)
        {
            blackKingMoved = true;

            if (kingToColumn > kingFromColumn)
            {
                blackKingsideRookMoved = true;
            }
            else
            {
                blackQueensideRookMoved = true;
            }
        }

        const int direction =
            kingToColumn > kingFromColumn
                ? 1
                : -1;

        const int rookFromColumn =
            direction == 1
                ? 7
                : 0;

        const int rookToColumn =
            kingToColumn - direction;

        squares[row][kingToColumn] =
            squares[row][kingFromColumn];

        squares[row][kingFromColumn] = Piece{};

        squares[row][rookToColumn] =
            squares[row][rookFromColumn];

        squares[row][rookFromColumn] = Piece{};
    }

    bool Board::hasEnPassantTarget() const
    {
        return enPassantRow != -1 &&
            enPassantColumn != -1;
    }

    int Board::getEnPassantRow() const
    {
        return enPassantRow;
    }

    int Board::getEnPassantColumn() const
    {
        return enPassantColumn;
    }

    void Board::setEnPassantTarget(int row, int column)
    {
        enPassantRow = row;
        enPassantColumn = column;
    }

    void Board::clearEnPassantTarget()
    {
        enPassantRow = -1;
        enPassantColumn = -1;
    }

    int Board::getHalfmoveClock() const
    {
        return halfmoveClock;
    }

}