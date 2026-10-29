#pragma once

#include <vector>

#include "chess/Board.h"

namespace chess
{
    enum class AIDifficulty
    {
        Easy,
        Medium,
        Hard
    };

    struct Move
    {
        int fromRow = -1;
        int fromColumn = -1;

        int toRow = -1;
        int toColumn = -1;

        PieceType promotion = PieceType::None;
    };

    class ComputerPlayer
    {
    public:
        static Move chooseMove(
            const Board& board,
            PieceColor color,
            AIDifficulty difficulty
        );

        static void applyMove(
            Board& board,
            const Move& move
        );

    private:
        static std::vector<Move> generateLegalMoves(
            const Board& board,
            PieceColor color
        );

        static int minimax(
            const Board& board,
            int depth,
            int alpha,
            int beta,
            PieceColor sideToMove,
            PieceColor aiColor
        );

        static int evaluateBoard(
            const Board& board,
            PieceColor aiColor
        );

        static int getSearchDepth(
            AIDifficulty difficulty
        );

        static int getPieceValue(
            PieceType type
        );

        static PieceColor getOpponent(
            PieceColor color
        );

        static int scoreMove(
            const Board& board,
            const Move& move
        );
    };
}