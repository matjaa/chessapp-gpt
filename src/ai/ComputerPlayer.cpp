#include "ai/ComputerPlayer.h"

#include <algorithm>
#include <cstdlib>
#include <limits>

#include "chess/MoveValidator.h"

namespace chess
{
    namespace
    {
        constexpr int CheckmateScore = 100000;
    }

    Move ComputerPlayer::chooseMove(
        const Board& board,
        PieceColor color,
        AIDifficulty difficulty
    )
    {
        const std::vector<Move> moves =
            generateLegalMoves(board, color);

        if (moves.empty())
        {
            return Move{};
        }

        const int depth = getSearchDepth(difficulty);

        Move bestMove = moves.front();

        int bestScore =
            std::numeric_limits<int>::min();

        int alpha =
            std::numeric_limits<int>::min();

        const int beta =
            std::numeric_limits<int>::max();

        for (const Move& move : moves)
        {
            Board testBoard = board;

            applyMove(testBoard, move);

            const int score = minimax(
                testBoard,
                depth - 1,
                alpha,
                beta,
                getOpponent(color),
                color
            );

            if (score > bestScore)
            {
                bestScore = score;
                bestMove = move;
            }

            alpha = std::max(alpha, bestScore);
        }

        return bestMove;
    }

    void ComputerPlayer::applyMove(
        Board& board,
        const Move& move
    )
    {
        const Piece movingPiece =
            board.getPiece(
                move.fromRow,
                move.fromColumn
            );

        const bool isCastling =
            movingPiece.type == PieceType::King &&
            std::abs(
                move.toColumn -
                move.fromColumn
            ) == 2;

        if (isCastling)
        {
            board.castle(
                move.fromRow,
                move.fromColumn,
                move.toColumn
            );
        }
        else
        {
            board.movePiece(
                move.fromRow,
                move.fromColumn,
                move.toRow,
                move.toColumn
            );
        }

        if (move.promotion != PieceType::None)
        {
            board.promotePawn(
                move.toRow,
                move.toColumn,
                move.promotion
            );
        }
    }

    std::vector<Move> ComputerPlayer::generateLegalMoves(
        const Board& board,
        PieceColor color
    )
    {
        std::vector<Move> moves;

        for (int fromRow = 0;
            fromRow < Board::Size;
            ++fromRow)
        {
            for (int fromColumn = 0;
                fromColumn < Board::Size;
                ++fromColumn)
            {
                const Piece& piece =
                    board.getPiece(
                        fromRow,
                        fromColumn
                    );

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
                        if (!MoveValidator::isValidMove(
                                board,
                                fromRow,
                                fromColumn,
                                toRow,
                                toColumn))
                        {
                            continue;
                        }

                        const bool isPromotion =
                            piece.type == PieceType::Pawn &&
                            (toRow == 0 || toRow == 7);

                        if (isPromotion)
                        {
                            const PieceType promotionTypes[] = {
                                PieceType::Queen,
                                PieceType::Rook,
                                PieceType::Bishop,
                                PieceType::Knight
                            };

                            for (PieceType promotion :
                                promotionTypes)
                            {
                                moves.push_back({
                                    fromRow,
                                    fromColumn,
                                    toRow,
                                    toColumn,
                                    promotion
                                });
                            }
                        }
                        else
                        {
                            moves.push_back({
                                fromRow,
                                fromColumn,
                                toRow,
                                toColumn
                            });
                        }
                    }
                }
            }
        }

        std::sort(
            moves.begin(),
            moves.end(),
            [&board](const Move& first,
                     const Move& second)
            {
                return scoreMove(board, first) >
                    scoreMove(board, second);
            }
        );

        return moves;
    }

    int ComputerPlayer::minimax(
        const Board& board,
        int depth,
        int alpha,
        int beta,
        PieceColor sideToMove,
        PieceColor aiColor
    )
    {
        if (MoveValidator::isCheckmate(
                board,
                sideToMove))
        {
            return sideToMove == aiColor
                ? -CheckmateScore - depth
                : CheckmateScore + depth;
        }

        if (MoveValidator::isStalemate(
                board,
                sideToMove))
        {
            return 0;
        }

        if (board.getHalfmoveClock() >= 100)
        {
            return 0;
        }

        if (depth == 0)
        {
            return evaluateBoard(board, aiColor);
        }

        const std::vector<Move> moves =
            generateLegalMoves(board, sideToMove);

        if (moves.empty())
        {
            return 0;
        }

        if (sideToMove == aiColor)
        {
            int bestScore =
                std::numeric_limits<int>::min();

            for (const Move& move : moves)
            {
                Board testBoard = board;

                applyMove(testBoard, move);

                const int score = minimax(
                    testBoard,
                    depth - 1,
                    alpha,
                    beta,
                    getOpponent(sideToMove),
                    aiColor
                );

                bestScore = std::max(
                    bestScore,
                    score
                );

                alpha = std::max(alpha, bestScore);

                if (beta <= alpha)
                {
                    break;
                }
            }

            return bestScore;
        }

        int bestScore =
            std::numeric_limits<int>::max();

        for (const Move& move : moves)
        {
            Board testBoard = board;

            applyMove(testBoard, move);

            const int score = minimax(
                testBoard,
                depth - 1,
                alpha,
                beta,
                getOpponent(sideToMove),
                aiColor
            );

            bestScore = std::min(
                bestScore,
                score
            );

            beta = std::min(beta, bestScore);

            if (beta <= alpha)
            {
                break;
            }
        }

        return bestScore;
    }

    int ComputerPlayer::evaluateBoard(
        const Board& board,
        PieceColor aiColor
    )
    {
        int score = 0;

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

                if (piece.isEmpty())
                {
                    continue;
                }

                int pieceScore =
                    getPieceValue(piece.type);

                // Reward pawns for advancing.
                if (piece.type == PieceType::Pawn)
                {
                    const int advancement =
                        piece.color == PieceColor::White
                            ? 6 - row
                            : row - 1;

                    pieceScore += advancement * 5;
                }

                // Reward occupying the four central squares.
                if (row >= 3 && row <= 4 &&
                    column >= 3 && column <= 4)
                {
                    pieceScore += 10;
                }

                if (piece.color == aiColor)
                {
                    score += pieceScore;
                }
                else
                {
                    score -= pieceScore;
                }
            }
        }

        return score;
    }

    int ComputerPlayer::getSearchDepth(
        AIDifficulty difficulty
    )
    {
        switch (difficulty)
        {
        case AIDifficulty::Easy:
            return 2;

        case AIDifficulty::Medium:
            return 3;

        case AIDifficulty::Hard:
            return 4;

        default:
            return 2;
        }
    }

    int ComputerPlayer::getPieceValue(
        PieceType type
    )
    {
        switch (type)
        {
        case PieceType::Pawn:
            return 100;

        case PieceType::Knight:
            return 320;

        case PieceType::Bishop:
            return 330;

        case PieceType::Rook:
            return 500;

        case PieceType::Queen:
            return 900;

        case PieceType::King:
            return 20000;

        default:
            return 0;
        }
    }

    PieceColor ComputerPlayer::getOpponent(
        PieceColor color
    )
    {
        return color == PieceColor::White
            ? PieceColor::Black
            : PieceColor::White;
    }

    int ComputerPlayer::scoreMove(
        const Board& board,
        const Move& move
    )
    {
        const Piece& target =
            board.getPiece(
                move.toRow,
                move.toColumn
            );

        int score = getPieceValue(target.type);

        if (move.promotion != PieceType::None)
        {
            score += getPieceValue(move.promotion);
        }

        return score;
    }
}