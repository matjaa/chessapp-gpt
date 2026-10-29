#include "chess/MoveValidator.h"

#include <cstdlib>

namespace chess
{
    bool MoveValidator::isValidMove(
        const Board& board,
        int fromRow,
        int fromColumn,
        int toRow,
        int toColumn
    )
    {
        // Make sure both positions are inside the board.
        if (fromRow < 0 || fromRow >= Board::Size ||
            fromColumn < 0 || fromColumn >= Board::Size ||
            toRow < 0 || toRow >= Board::Size ||
            toColumn < 0 || toColumn >= Board::Size)
        {
            return false;
        }

        const Piece& piece =
            board.getPiece(fromRow, fromColumn);

        // There must be a piece at the starting position.
        if (piece.isEmpty())
        {
            return false;
        }

        // A piece cannot move to its current square.
        if (fromRow == toRow &&
            fromColumn == toColumn)
        {
            return false;
        }

        const Piece& target =
            board.getPiece(toRow, toColumn);

        // Cannot capture your own piece.
        if (!target.isEmpty() &&
            target.color == piece.color)
        {
            return false;
        }

        // Kings are never captured directly.
        if (!target.isEmpty() &&
            target.type == PieceType::King)
        {
            return false;
        }

        // Check for castling
        if (piece.type == PieceType::King &&
            std::abs(toColumn - fromColumn) == 2)
        {
            return isValidCastlingMove(
                board,
                fromRow,
                fromColumn,
                toRow,
                toColumn
            );
        }

        bool movementIsValid = false;

        switch (piece.type)
        {
        case PieceType::Pawn:
            movementIsValid = isValidPawnMove(
                board,
                fromRow,
                fromColumn,
                toRow,
                toColumn
            );
            break;

        case PieceType::Knight:
            movementIsValid = isValidKnightMove(
                board,
                fromRow,
                fromColumn,
                toRow,
                toColumn
            );
            break;

        case PieceType::Bishop:
            movementIsValid = isValidBishopMove(
                board,
                fromRow,
                fromColumn,
                toRow,
                toColumn
            );
            break;

        case PieceType::Rook:
            movementIsValid = isValidRookMove(
                board,
                fromRow,
                fromColumn,
                toRow,
                toColumn
            );
            break;

        case PieceType::Queen:
            movementIsValid = isValidQueenMove(
                board,
                fromRow,
                fromColumn,
                toRow,
                toColumn
            );
            break;

        case PieceType::King:
            movementIsValid = isValidKingMove(
                board,
                fromRow,
                fromColumn,
                toRow,
                toColumn
            );
            break;

        default:
            return false;
        }

        if (!movementIsValid)
        {
            return false;
        }


        Board testBoard = board;

        const bool isEnPassant =
            piece.type == PieceType::Pawn &&
            fromColumn != toColumn &&
            board.getPiece(toRow, toColumn).isEmpty() &&
            board.hasEnPassantTarget() &&
            toRow == board.getEnPassantRow() &&
            toColumn == board.getEnPassantColumn();

        if (isEnPassant)
        {
            testBoard.setPiece(
                fromRow,
                toColumn,
                Piece{}
            );
        }

        testBoard.movePiece(
            fromRow,
            fromColumn,
            toRow,
            toColumn
        );

        if (isKingInCheck(testBoard, piece.color))
        {
            return false;
        }

        return true;
    }

    bool MoveValidator::isValidPawnMove(
        const Board& board,
        int fromRow,
        int fromColumn,
        int toRow,
        int toColumn
    )
    {
        const Piece& pawn =
            board.getPiece(fromRow, fromColumn);

        const Piece& target =
            board.getPiece(toRow, toColumn);

        const int direction =
            pawn.color == PieceColor::White ? -1 : 1;

        const int rowDifference =
            toRow - fromRow;

        const int columnDifference =
            toColumn - fromColumn;

        // Move one square forward.
        if (columnDifference == 0 &&
            rowDifference == direction &&
            target.isEmpty())
        {
            return true;
        }

        // Move two squares forward from starting position.
        const int startingRow =
            pawn.color == PieceColor::White ? 6 : 1;

        if (columnDifference == 0 &&
            fromRow == startingRow &&
            rowDifference == 2 * direction &&
            target.isEmpty())
        {
            const int middleRow =
                fromRow + direction;

            if (board.getPiece(
                    middleRow,
                    fromColumn
                ).isEmpty())
            {
                return true;
            }
        }

        // Diagonal capture.
        if (std::abs(columnDifference) == 1 &&
            rowDifference == direction &&
            !target.isEmpty() &&
            target.color != pawn.color)
        {
            return true;
        }

        // En passant capture.
        if (std::abs(columnDifference) == 1 &&
            rowDifference == direction &&
            target.isEmpty() &&
            board.hasEnPassantTarget() &&
            toRow == board.getEnPassantRow() &&
            toColumn == board.getEnPassantColumn())
        {
            const Piece& capturedPawn =
                board.getPiece(fromRow, toColumn);

            if (capturedPawn.type == PieceType::Pawn &&
                capturedPawn.color != pawn.color)
            {
                return true;
            }
        }

        return false;
    }

    bool MoveValidator::isValidKnightMove(
        const Board& board,
        int fromRow,
        int fromColumn,
        int toRow,
        int toColumn
    )
    {
        const int rowDifference =
            std::abs(toRow - fromRow);

        const int columnDifference =
            std::abs(toColumn - fromColumn);

        return
            (rowDifference == 2 &&
            columnDifference == 1) ||
            (rowDifference == 1 &&
            columnDifference == 2);
    }

    bool MoveValidator::isValidBishopMove(
        const Board& board,
        int fromRow,
        int fromColumn,
        int toRow,
        int toColumn
    )
    {
        const int rowDifference =
            std::abs(toRow - fromRow);

        const int columnDifference =
            std::abs(toColumn - fromColumn);

        // Bishops must move diagonally.
        if (rowDifference != columnDifference)
        {
            return false;
        }

        const int rowDirection =
            toRow > fromRow ? 1 : -1;

        const int columnDirection =
            toColumn > fromColumn ? 1 : -1;

        int row = fromRow + rowDirection;
        int column = fromColumn + columnDirection;

        // Every square between the two positions
        // must be empty.
        while (row != toRow &&
            column != toColumn)
        {
            if (!board.getPiece(row, column).isEmpty())
            {
                return false;
            }

            row += rowDirection;
            column += columnDirection;
        }

        return true;
    }

    bool MoveValidator::isValidRookMove(
        const Board& board,
        int fromRow,
        int fromColumn,
        int toRow,
        int toColumn
    )
    {
        // Rooks must move horizontally or vertically.
        if (fromRow != toRow &&
            fromColumn != toColumn)
        {
            return false;
        }

        const int rowDirection =
            toRow > fromRow ? 1 :
            toRow < fromRow ? -1 : 0;

        const int columnDirection =
            toColumn > fromColumn ? 1 :
            toColumn < fromColumn ? -1 : 0;

        int row = fromRow + rowDirection;
        int column = fromColumn + columnDirection;

        // Every square between the two positions
        // must be empty.
        while (row != toRow ||
            column != toColumn)
        {
            if (!board.getPiece(row, column).isEmpty())
            {
                return false;
            }

            row += rowDirection;
            column += columnDirection;
        }

        return true;
    }

    bool MoveValidator::isValidQueenMove(
        const Board& board,
        int fromRow,
        int fromColumn,
        int toRow,
        int toColumn
    )
    {
        const int rowDifference =
            std::abs(toRow - fromRow);

        const int columnDifference =
            std::abs(toColumn - fromColumn);

        // Queen moves like a bishop.
        if (rowDifference == columnDifference)
        {
            const int rowDirection =
                toRow > fromRow ? 1 : -1;

            const int columnDirection =
                toColumn > fromColumn ? 1 : -1;

            int row = fromRow + rowDirection;
            int column = fromColumn + columnDirection;

            while (row != toRow &&
                column != toColumn)
            {
                if (!board.getPiece(row, column).isEmpty())
                {
                    return false;
                }

                row += rowDirection;
                column += columnDirection;
            }

            return true;
        }

        // Queen moves like a rook.
        if (fromRow == toRow ||
            fromColumn == toColumn)
        {
            const int rowDirection =
                toRow > fromRow ? 1 :
                toRow < fromRow ? -1 : 0;

            const int columnDirection =
                toColumn > fromColumn ? 1 :
                toColumn < fromColumn ? -1 : 0;

            int row = fromRow + rowDirection;
            int column = fromColumn + columnDirection;

            while (row != toRow ||
                column != toColumn)
            {
                if (!board.getPiece(row, column).isEmpty())
                {
                    return false;
                }

                row += rowDirection;
                column += columnDirection;
            }

            return true;
        }

        return false;
    }

    bool MoveValidator::isValidKingMove(
        const Board& board,
        int fromRow,
        int fromColumn,
        int toRow,
        int toColumn
    )
    {
        const int rowDifference =
            std::abs(toRow - fromRow);

        const int columnDifference =
            std::abs(toColumn - fromColumn);
        
        return rowDifference <= 1 &&
            columnDifference <= 1;
    }

    bool MoveValidator::isSquareAttacked(
        const Board& board,
        int row,
        int column,
        PieceColor attackingColor
    )
    {
        for (int fromRow = 0;
            fromRow < Board::Size;
            ++fromRow)
        {
            for (int fromColumn = 0;
                fromColumn < Board::Size;
                ++fromColumn)
            {
                const Piece& piece =
                    board.getPiece(fromRow, fromColumn);

                if (piece.isEmpty() ||
                    piece.color != attackingColor)
                {
                    continue;
                }

                const int rowDifference =
                    std::abs(row - fromRow);

                const int columnDifference =
                    std::abs(column - fromColumn);

                switch (piece.type)
                {
                case PieceType::Pawn:
                {
                    const int direction =
                        attackingColor == PieceColor::White
                            ? -1
                            : 1;

                    if (row - fromRow == direction &&
                        columnDifference == 1)
                    {
                        return true;
                    }

                    break;
                }

                case PieceType::Knight:
                {
                    if ((rowDifference == 2 &&
                        columnDifference == 1) ||
                        (rowDifference == 1 &&
                        columnDifference == 2))
                    {
                        return true;
                    }

                    break;
                }

                case PieceType::Bishop:
                {
                    if (rowDifference != columnDifference)
                    {
                        break;
                    }

                    const int rowDirection =
                        row > fromRow ? 1 : -1;

                    const int columnDirection =
                        column > fromColumn ? 1 : -1;

                    int checkRow =
                        fromRow + rowDirection;

                    int checkColumn =
                        fromColumn + columnDirection;

                    bool blocked = false;

                    while (checkRow != row &&
                        checkColumn != column)
                    {
                        if (!board.getPiece(
                                checkRow,
                                checkColumn
                            ).isEmpty())
                        {
                            blocked = true;
                            break;
                        }

                        checkRow += rowDirection;
                        checkColumn += columnDirection;
                    }

                    if (!blocked)
                    {
                        return true;
                    }

                    break;
                }

                case PieceType::Rook:
                {
                    if (fromRow != row &&
                        fromColumn != column)
                    {
                        break;
                    }

                    const int rowDirection =
                        row > fromRow ? 1 :
                        row < fromRow ? -1 : 0;

                    const int columnDirection =
                        column > fromColumn ? 1 :
                        column < fromColumn ? -1 : 0;

                    int checkRow =
                        fromRow + rowDirection;

                    int checkColumn =
                        fromColumn + columnDirection;

                    bool blocked = false;

                    while (checkRow != row ||
                        checkColumn != column)
                    {
                        if (!board.getPiece(
                                checkRow,
                                checkColumn
                            ).isEmpty())
                        {
                            blocked = true;
                            break;
                        }

                        checkRow += rowDirection;
                        checkColumn += columnDirection;
                    }

                    if (!blocked)
                    {
                        return true;
                    }

                    break;
                }

                case PieceType::Queen:
                {
                    const bool diagonal =
                        rowDifference == columnDifference;

                    const bool straight =
                        fromRow == row ||
                        fromColumn == column;

                    if (!diagonal && !straight)
                    {
                        break;
                    }

                    const int rowDirection =
                        row > fromRow ? 1 :
                        row < fromRow ? -1 : 0;

                    const int columnDirection =
                        column > fromColumn ? 1 :
                        column < fromColumn ? -1 : 0;

                    int checkRow =
                        fromRow + rowDirection;

                    int checkColumn =
                        fromColumn + columnDirection;

                    bool blocked = false;

                    while (checkRow != row ||
                        checkColumn != column)
                    {
                        if (!board.getPiece(
                                checkRow,
                                checkColumn
                            ).isEmpty())
                        {
                            blocked = true;
                            break;
                        }

                        checkRow += rowDirection;
                        checkColumn += columnDirection;
                    }

                    if (!blocked)
                    {
                        return true;
                    }

                    break;
                }

                case PieceType::King:
                {
                    if (rowDifference <= 1 &&
                        columnDifference <= 1)
                    {
                        return true;
                    }

                    break;
                }

                default:
                    break;
                }
            }
        }

        return false;
    }

    bool MoveValidator::isKingInCheck(
        const Board& board,
        PieceColor kingColor
    )
    {
        for (int row = 0;
            row < Board::Size;
            ++row)
        {
            for (int column = 0;
                column < Board::Size;
                ++column)
            {
                const Piece& piece =
                    board.getPiece(row, column);

                if (piece.type == PieceType::King &&
                    piece.color == kingColor)
                {
                    const PieceColor attackingColor =
                        kingColor == PieceColor::White
                            ? PieceColor::Black
                            : PieceColor::White;

                    return isSquareAttacked(
                        board,
                        row,
                        column,
                        attackingColor
                    );
                }
            }
        }

        // A valid chess position should always
        // contain the king.
        return false;
    }

    bool MoveValidator::hasLegalMoves(
        const Board& board,
        PieceColor color
    )
    {
        for (int fromRow = 0;
            fromRow < Board::Size;
            ++fromRow)
        {
            for (int fromColumn = 0;
                fromColumn < Board::Size;
                ++fromColumn)
            {
                const Piece& piece =
                    board.getPiece(fromRow, fromColumn);

                if (piece.isEmpty() ||
                    piece.color != color)
                {
                    continue;
                }

                for (int toRow = 0;
                    toRow < Board::Size;
                    ++toRow)
                {
                    for (int toColumn = 0;
                        toColumn < Board::Size;
                        ++toColumn)
                    {
                        if (isValidMove(
                                board,
                                fromRow,
                                fromColumn,
                                toRow,
                                toColumn))
                        {
                            return true;
                        }
                    }
                }
            }
        }

        return false;
    }

    bool MoveValidator::isCheckmate(
        const Board& board,
        PieceColor color
    )
    {
        return isKingInCheck(board, color) &&
            !hasLegalMoves(board, color);
    }

    bool MoveValidator::isStalemate(
        const Board& board,
        PieceColor color
    )
    {
        return !isKingInCheck(board, color) &&
            !hasLegalMoves(board, color);
    }

    bool MoveValidator::isValidCastlingMove(
        const Board& board,
        int fromRow,
        int fromColumn,
        int toRow,
        int toColumn
    )
    {
        const Piece& king =
            board.getPiece(fromRow, fromColumn);

        // The moving piece must be a king.
        if (king.type != PieceType::King)
        {
            return false;
        }

        // The king must move exactly two columns.
        if (fromRow != toRow ||
            std::abs(toColumn - fromColumn) != 2)
        {
            return false;
        }

        // The king must be on its original square.
        if (king.color == PieceColor::White)
        {
            if (fromRow != 7 ||
                fromColumn != 4)
            {
                return false;
            }

            if (board.hasWhiteKingMoved())
            {
                return false;
            }
        }
        else if (king.color == PieceColor::Black)
        {
            if (fromRow != 0 ||
                fromColumn != 4)
            {
                return false;
            }

            if (board.hasBlackKingMoved())
            {
                return false;
            }
        }
        else
        {
            return false;
        }

        const bool kingside =
            toColumn > fromColumn;

        const int rookColumn =
            kingside ? 7 : 0;

        const Piece& rook =
            board.getPiece(fromRow, rookColumn);

        // There must be a rook of the same color.
        if (rook.type != PieceType::Rook ||
            rook.color != king.color)
        {
            return false;
        }

        // The appropriate rook must not have moved.
        if (king.color == PieceColor::White)
        {
            if (kingside &&
                board.hasWhiteKingsideRookMoved())
            {
                return false;
            }

            if (!kingside &&
                board.hasWhiteQueensideRookMoved())
            {
                return false;
            }
        }
        else
        {
            if (kingside &&
                board.hasBlackKingsideRookMoved())
            {
                return false;
            }

            if (!kingside &&
                board.hasBlackQueensideRookMoved())
            {
                return false;
            }
        }

        const int direction =
            kingside ? 1 : -1;

        for (int column = fromColumn + direction;
            column != rookColumn;
            column += direction)
        {
            if (!board.getPiece(fromRow, column).isEmpty())
            {
                return false;
            }
        }

        const int middleColumn =
            fromColumn + direction;

        const PieceColor enemyColor =
            king.color == PieceColor::White
                ? PieceColor::Black
                : PieceColor::White;

        if (isSquareAttacked(
                board,
                fromRow,
                middleColumn,
                enemyColor))
        {
            return false;
        }

        if (isSquareAttacked(
                board,
                toRow,
                toColumn,
                enemyColor))
        {
            return false;
        }

        return true;
    }
}