#pragma once

namespace chess
{
    enum class PieceType
    {
        None,
        Pawn,
        Knight,
        Bishop,
        Rook,
        Queen,
        King
    };

    enum class PieceColor
    {
        None,
        White,
        Black
    };

    struct Piece
    {
        PieceType type = PieceType::None;
        PieceColor color = PieceColor::None;

        bool isEmpty() const
        {
            return type == PieceType::None;
        }
    };
}