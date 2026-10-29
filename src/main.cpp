#include <cmath>
#include <optional>
#include <iostream>
#include <string>
#include <vector>
#include <unordered_map>
#include <cctype>

#include <SFML/Graphics.hpp>

#include "chess/Board.h"
#include "chess/MoveValidator.h"
#include "ai/ComputerPlayer.h"
#include "graphics/Renderer.h"

int main()
{
    sf::RenderWindow window(
        sf::VideoMode({1100, 900}),
        "Chess"
    );

    chess::Board board;
    graphics::Renderer renderer(window);

    sf::Font font;
    if (!font.openFromFile("assets/fonts/arial.ttf"))
    {
        std::cerr << "Failed to load font.\n";
        return 1;
    }

    sf::Text statusText(font);

    statusText.setCharacterSize(24);
    statusText.setFillColor(sf::Color::White);
    statusText.setPosition({10.0f, 10.0f});

    bool selected = false;
    bool holdingPiece = false;
    bool draggingPiece = false;
    bool clickedSelectedPiece = false;

    chess::PieceColor currentTurn =
        chess::PieceColor::White;

    chess::PieceColor playerColor = chess::PieceColor::None;

    bool gameStarted = false;

    chess::AIDifficulty selectedDifficulty =
        chess::AIDifficulty::Medium;

    std::vector<std::string> moveHistory;

    std::string pendingMoveNotation;

    std::unordered_map<std::string, int> positionCounts;

    bool gameOver = false;
    bool promotionPending = false;

    int promotionRow = -1;
    int promotionColumn = -1;

    int selectedRow = -1;
    int selectedColumn = -1;

    int hoverRow = -1;
    int hoverColumn = -1;

    sf::Vector2i pressPosition;

    constexpr float DragThreshold = 5.0f;

    auto deselectPiece = [&]()
    {
        selected = false;
        selectedRow = -1;
        selectedColumn = -1;
    };

    auto getBoardPosition = [&](
        int screenRow,
        int screenColumn
    )
    {
        if (playerColor == chess::PieceColor::Black)
        {
            return sf::Vector2i(
                7 - screenColumn,
                7 - screenRow
            );
        }

        return sf::Vector2i(
            screenColumn,
            screenRow
        );
    };

    auto drawBoardCoordinates = [&]()
    {
        for (int displayRow = 0;
            displayRow < chess::Board::Size;
            ++displayRow)
        {
            const int boardRow =
                playerColor == chess::PieceColor::Black
                    ? 7 - displayRow
                    : displayRow;

            const char rank =
                static_cast<char>('8' - boardRow);

            sf::Text rankText(
                font,
                std::string(1, rank)
            );

            rankText.setCharacterSize(18);

            rankText.setFillColor(
                (displayRow % 2 == 0)
                    ? sf::Color(120, 85, 55)
                    : sf::Color(245, 225, 190)
            );

            rankText.setPosition({
                5.0f,
                displayRow * 100.0f + 3.0f
            });

            window.draw(rankText);
        }

        for (int displayColumn = 0;
            displayColumn < chess::Board::Size;
            ++displayColumn)
        {
            const int boardColumn =
                playerColor == chess::PieceColor::Black
                    ? 7 - displayColumn
                    : displayColumn;

            const char file =
                static_cast<char>('a' + boardColumn);

            sf::Text fileText(
                font,
                std::string(1, file)
            );

            fileText.setCharacterSize(18);

            fileText.setFillColor(
                (displayColumn % 2 == 0)
                    ? sf::Color(245, 225, 190)
                    : sf::Color(120, 85, 55)
            );

            fileText.setPosition({
                displayColumn * 100.0f + 82.0f,
                774.0f
            });

            window.draw(fileText);
        }
    };

    auto createPositionKey = [&]()
    {
        std::string key;

        for (int row = 0;
            row < chess::Board::Size;
            ++row)
        {
            for (int column = 0;
                column < chess::Board::Size;
                ++column)
            {
                const chess::Piece& piece =
                    board.getPiece(row, column);

                if (piece.isEmpty())
                {
                    key += '.';
                    continue;
                }

                char symbol = '?';

                switch (piece.type)
                {
                case chess::PieceType::Pawn:
                    symbol = 'P';
                    break;

                case chess::PieceType::Knight:
                    symbol = 'N';
                    break;

                case chess::PieceType::Bishop:
                    symbol = 'B';
                    break;

                case chess::PieceType::Rook:
                    symbol = 'R';
                    break;

                case chess::PieceType::Queen:
                    symbol = 'Q';
                    break;

                case chess::PieceType::King:
                    symbol = 'K';
                    break;

                default:
                    break;
                }

                if (piece.color == chess::PieceColor::Black)
                {
                    symbol = static_cast<char>(
                        std::tolower(symbol)
                    );
                }

                key += symbol;
            }
        }

        key += "|";

        key += currentTurn == chess::PieceColor::White
            ? "w"
            : "b";

        key += "|";

        bool hasCastlingRights = false;

        const chess::Piece& whiteKing =
            board.getPiece(7, 4);

        const chess::Piece& blackKing =
            board.getPiece(0, 4);

        const chess::Piece& whiteKingsideRook =
            board.getPiece(7, 7);

        const chess::Piece& whiteQueensideRook =
            board.getPiece(7, 0);

        const chess::Piece& blackKingsideRook =
            board.getPiece(0, 7);

        const chess::Piece& blackQueensideRook =
            board.getPiece(0, 0);

        if (!board.hasWhiteKingMoved() &&
            !board.hasWhiteKingsideRookMoved() &&
            whiteKing.type == chess::PieceType::King &&
            whiteKing.color == chess::PieceColor::White &&
            whiteKingsideRook.type == chess::PieceType::Rook &&
            whiteKingsideRook.color == chess::PieceColor::White)
        {
            key += 'K';
            hasCastlingRights = true;
        }

        if (!board.hasWhiteKingMoved() &&
            !board.hasWhiteQueensideRookMoved() &&
            whiteKing.type == chess::PieceType::King &&
            whiteKing.color == chess::PieceColor::White &&
            whiteQueensideRook.type == chess::PieceType::Rook &&
            whiteQueensideRook.color == chess::PieceColor::White)
        {
            key += 'Q';
            hasCastlingRights = true;
        }

        if (!board.hasBlackKingMoved() &&
            !board.hasBlackKingsideRookMoved() &&
            blackKing.type == chess::PieceType::King &&
            blackKing.color == chess::PieceColor::Black &&
            blackKingsideRook.type == chess::PieceType::Rook &&
            blackKingsideRook.color == chess::PieceColor::Black)
        {
            key += 'k';
            hasCastlingRights = true;
        }

        if (!board.hasBlackKingMoved() &&
            !board.hasBlackQueensideRookMoved() &&
            blackKing.type == chess::PieceType::King &&
            blackKing.color == chess::PieceColor::Black &&
            blackQueensideRook.type == chess::PieceType::Rook &&
            blackQueensideRook.color == chess::PieceColor::Black)
        {
            key += 'q';
            hasCastlingRights = true;
        }

        if (!hasCastlingRights)
        {
            key += '-';
        }

        key += "|";

        bool enPassantCapturePossible = false;

        if (board.hasEnPassantTarget())
        {
            const int targetRow =
                board.getEnPassantRow();

            const int targetColumn =
                board.getEnPassantColumn();

            for (int row = 0;
                row < chess::Board::Size &&
                !enPassantCapturePossible;
                ++row)
            {
                for (int column = 0;
                    column < chess::Board::Size;
                    ++column)
                {
                    const chess::Piece& piece =
                        board.getPiece(row, column);

                    if (piece.type != chess::PieceType::Pawn ||
                        piece.color != currentTurn)
                    {
                        continue;
                    }

                    if (chess::MoveValidator::isValidMove(
                            board,
                            row,
                            column,
                            targetRow,
                            targetColumn))
                    {
                        enPassantCapturePossible = true;
                        break;
                    }
                }
            }

            if (enPassantCapturePossible)
            {
                key += static_cast<char>(
                    'a' + targetColumn
                );

                key += static_cast<char>(
                    '8' - targetRow
                );
            }
        }

        if (!enPassantCapturePossible)
        {
            key += '-';
        }

        return key;
    };

    auto resetPositionHistory = [&]()
    {
        positionCounts.clear();

        positionCounts[createPositionKey()] = 1;
    };

    auto recordPosition = [&]()
    {
        const std::string positionKey =
            createPositionKey();

        const int occurrences =
            ++positionCounts[positionKey];

        if (occurrences < 3)
        {
            return false;
        }

        gameOver = true;

        statusText.setString(
            "Draw by threefold repetition."
        );

        return true;
    };

    auto updateGameState = [&]()
    {
        currentTurn =
            currentTurn == chess::PieceColor::White
                ? chess::PieceColor::Black
                : chess::PieceColor::White;

        if (chess::MoveValidator::isCheckmate(
                board,
                currentTurn))
        {
            gameOver = true;

            statusText.setString(
                currentTurn == chess::PieceColor::White
                    ? "Checkmate! Black wins."
                    : "Checkmate! White wins."
            );
        }
        else if (chess::MoveValidator::isStalemate(
                    board,
                    currentTurn))
        {
            gameOver = true;

            statusText.setString(
                "Stalemate! Draw."
            );
        }
        else if (board.getHalfmoveClock() >= 100)
        {
            gameOver = true;

            statusText.setString(
                "Draw by fifty-move rule."
            );
        }
        else if (chess::MoveValidator::isKingInCheck(
                    board,
                    currentTurn))
        {
            statusText.setString(
                currentTurn == chess::PieceColor::White
                    ? "White is in check."
                    : "Black is in check."
            );
        }
        else
        {
            statusText.setString(
                currentTurn == chess::PieceColor::White
                    ? "White's turn"
                    : "Black's turn"
            );
        }

        if (!gameOver)
        {
            recordPosition();
        }

        if (!pendingMoveNotation.empty())
        {
            if (chess::MoveValidator::isCheckmate(
                    board,
                    currentTurn))
            {
                pendingMoveNotation += "#";
            }
            else if (chess::MoveValidator::isKingInCheck(
                        board,
                        currentTurn))
            {
                pendingMoveNotation += "+";
            }

            moveHistory.push_back(pendingMoveNotation);

            pendingMoveNotation.clear();
        }
    };

    auto restartGame = [&]()
    {
        board = chess::Board{};

        moveHistory.clear();
        pendingMoveNotation.clear();

        currentTurn = chess::PieceColor::White;

        resetPositionHistory();

        gameOver = false;
        promotionPending = false;

        promotionRow = -1;
        promotionColumn = -1;

        holdingPiece = false;
        draggingPiece = false;
        clickedSelectedPiece = false;

        hoverRow = -1;
        hoverColumn = -1;

        deselectPiece();

        statusText.setString("White's turn");
    };

    auto returnToSideSelection = [&]()
    {
        restartGame();

        playerColor = chess::PieceColor::None;
        gameStarted = false;

        statusText.setString("");
    };

    auto createMoveNotation = [&](
        int fromRow,
        int fromColumn,
        int toRow,
        int toColumn
    )
    {
        const chess::Piece& movingPiece =
            board.getPiece(fromRow, fromColumn);

        const chess::Piece& targetPiece =
            board.getPiece(toRow, toColumn);

        const bool isCastling =
            movingPiece.type == chess::PieceType::King &&
            std::abs(toColumn - fromColumn) == 2;

        if (isCastling)
        {
            return toColumn > fromColumn
                ? std::string("O-O")
                : std::string("O-O-O");
        }

        const bool isEnPassant =
            movingPiece.type == chess::PieceType::Pawn &&
            fromColumn != toColumn &&
            targetPiece.isEmpty() &&
            board.hasEnPassantTarget() &&
            toRow == board.getEnPassantRow() &&
            toColumn == board.getEnPassantColumn();

        const bool isCapture =
            !targetPiece.isEmpty() || isEnPassant;

        std::string notation;

        switch (movingPiece.type)
        {
        case chess::PieceType::Knight:
            notation = "N";
            break;

        case chess::PieceType::Bishop:
            notation = "B";
            break;

        case chess::PieceType::Rook:
            notation = "R";
            break;

        case chess::PieceType::Queen:
            notation = "Q";
            break;

        case chess::PieceType::King:
            notation = "K";
            break;

        default:
            break;
        }

        // Pawns show their original file only when capturing.
        if (movingPiece.type == chess::PieceType::Pawn &&
            isCapture)
        {
            notation +=
                static_cast<char>('a' + fromColumn);
        }

        // Distinguish identical pieces that can legally
        // move to the same destination.
        if (movingPiece.type != chess::PieceType::Pawn &&
            movingPiece.type != chess::PieceType::King)
        {
            bool anotherPieceCanMove = false;
            bool sameFileExists = false;
            bool sameRankExists = false;

            for (int row = 0; row < chess::Board::Size; ++row)
            {
                for (int column = 0;
                    column < chess::Board::Size;
                    ++column)
                {
                    if (row == fromRow &&
                        column == fromColumn)
                    {
                        continue;
                    }

                    const chess::Piece& otherPiece =
                        board.getPiece(row, column);

                    if (otherPiece.type != movingPiece.type ||
                        otherPiece.color != movingPiece.color)
                    {
                        continue;
                    }

                    if (!chess::MoveValidator::isValidMove(
                            board,
                            row,
                            column,
                            toRow,
                            toColumn))
                    {
                        continue;
                    }

                    anotherPieceCanMove = true;

                    if (column == fromColumn)
                    {
                        sameFileExists = true;
                    }

                    if (row == fromRow)
                    {
                        sameRankExists = true;
                    }
                }
            }

            if (anotherPieceCanMove)
            {
                if (!sameFileExists)
                {
                    notation +=
                        static_cast<char>('a' + fromColumn);
                }
                else if (!sameRankExists)
                {
                    notation +=
                        static_cast<char>('8' - fromRow);
                }
                else
                {
                    notation +=
                        static_cast<char>('a' + fromColumn);

                    notation +=
                        static_cast<char>('8' - fromRow);
                }
            }
        }

        if (isCapture)
        {
            notation += "x";
        }

        notation += static_cast<char>('a' + toColumn);
        notation += static_cast<char>('8' - toRow);

        return notation;
    };

    auto completeMove = [&](int toRow, int toColumn)
    {
        pendingMoveNotation = createMoveNotation(
            selectedRow,
            selectedColumn,
            toRow,
            toColumn
        );

        const chess::Piece& movingPiece =
            board.getPiece(
                selectedRow,
                selectedColumn
            );

        const bool isCastling =
            movingPiece.type == chess::PieceType::King &&
            std::abs(toColumn - selectedColumn) == 2;

        if (isCastling)
        {
            board.castle(
                selectedRow,
                selectedColumn,
                toColumn
            );
        }
        else
        {
            board.movePiece(
                selectedRow,
                selectedColumn,
                toRow,
                toColumn
            );
        }

        selectedRow = toRow;
        selectedColumn = toColumn;

        const chess::Piece& movedPiece =
            board.getPiece(toRow, toColumn);

        const bool reachedPromotionRank =
            movedPiece.type == chess::PieceType::Pawn &&
            (toRow == 0 || toRow == 7);

        deselectPiece();

        if (reachedPromotionRank)
        {
            promotionPending = true;

            promotionRow = toRow;
            promotionColumn = toColumn;
        }
        else
        {
            updateGameState();
        }
    };

    auto handlePromotion = [&](int option)
    {
        const chess::PieceType promotionTypes[] = {
            chess::PieceType::Queen,
            chess::PieceType::Rook,
            chess::PieceType::Bishop,
            chess::PieceType::Knight
        };

        board.promotePawn(
            promotionRow,
            promotionColumn,
            promotionTypes[option]
        );

        switch (promotionTypes[option])
        {
        case chess::PieceType::Queen:
            pendingMoveNotation += "=Q";
            break;

        case chess::PieceType::Rook:
            pendingMoveNotation += "=R";
            break;

        case chess::PieceType::Bishop:
            pendingMoveNotation += "=B";
            break;

        case chess::PieceType::Knight:
            pendingMoveNotation += "=N";
            break;

        default:
            break;
        }

        promotionPending = false;

        promotionRow = -1;
        promotionColumn = -1;

        updateGameState();
    };

    auto makeComputerMove = [&]()
    {
        const chess::PieceColor computerColor =
            playerColor == chess::PieceColor::White
                ? chess::PieceColor::Black
                : chess::PieceColor::White;

        const chess::Move move =
            chess::ComputerPlayer::chooseMove(
                board,
                computerColor,
                selectedDifficulty
            );

        // No move means checkmate or stalemate.
        if (move.fromRow == -1)
        {
            return;
        }

        pendingMoveNotation = createMoveNotation(
            move.fromRow,
            move.fromColumn,
            move.toRow,
            move.toColumn
        );

        switch (move.promotion)
        {
        case chess::PieceType::Queen:
            pendingMoveNotation += "=Q";
            break;

        case chess::PieceType::Rook:
            pendingMoveNotation += "=R";
            break;

        case chess::PieceType::Bishop:
            pendingMoveNotation += "=B";
            break;

        case chess::PieceType::Knight:
            pendingMoveNotation += "=N";
            break;

        default:
            break;
        }

        chess::ComputerPlayer::applyMove(
            board,
            move
        );

        updateGameState();
    };

    while (window.isOpen())
    {   
        if (gameStarted &&
            !gameOver &&
            !promotionPending &&
            currentTurn != playerColor)
        {
            makeComputerMove();
        }
        while (const std::optional event = window.pollEvent())
        {
            if (event->is<sf::Event::Closed>())
            {
                window.close();
            }

            // --------------------------------
            // Mouse pressed
            // --------------------------------

            if (const auto* mouseButton =
                    event->getIf<sf::Event::MouseButtonPressed>())
            {
                if (!gameStarted)
                {
                    const int mouseX = mouseButton->position.x;
                    const int mouseY = mouseButton->position.y;

                    if (mouseY >= 610 && mouseY < 675)
                    {
                        if (mouseX >= 230 && mouseX < 430)
                        {
                            selectedDifficulty = chess::AIDifficulty::Easy;
                        }
                        else if (mouseX >= 450 && mouseX < 650)
                        {
                            selectedDifficulty = chess::AIDifficulty::Medium;
                        }
                        else if (mouseX >= 670 && mouseX < 870)
                        {
                            selectedDifficulty = chess::AIDifficulty::Hard;
                        }
                    }

                    if (mouseY >= 430 && mouseY < 530)
                    {
                        if (mouseX >= 320 && mouseX < 540)
                        {
                            playerColor = chess::PieceColor::White;
                            gameStarted = true;
                        }
                        else if (mouseX >= 560 && mouseX < 780)
                        {
                            playerColor = chess::PieceColor::Black;
                            gameStarted = true;
                        }

                        if (gameStarted)
                        {
                            resetPositionHistory();
                            statusText.setString("White's turn");
                        }
                    }

                    continue;
                }

                if (mouseButton->button ==
                    sf::Mouse::Button::Left)
                {
                    const int mouseX = mouseButton->position.x;
                    const int mouseY = mouseButton->position.y;

                    // Center button shown after checkmate or stalemate.
                    if (gameOver)
                    {
                        if (mouseX >= 250 && mouseX < 550 &&
                            mouseY >= 360 && mouseY < 440)
                        {
                            returnToSideSelection();
                        }

                        continue;
                    }

                    if (gameStarted &&
                        mouseButton->position.x >= 620 &&
                        mouseButton->position.x < 780 &&
                        mouseButton->position.y >= 820 &&
                        mouseButton->position.y < 880)
                    {
                        restartGame();
                        continue;
                    }

                    if (gameStarted &&
                        mouseX >= 440 &&
                        mouseX < 600 &&
                        mouseY >= 820 &&
                        mouseY < 880)
                    {
                        returnToSideSelection();
                        continue;
                    }

                    if (currentTurn != playerColor)
                    {
                        continue;
                    }

                    const int screenColumn =
                        mouseButton->position.x / 100;

                    const int screenRow =
                        mouseButton->position.y / 100;

                    const sf::Vector2i boardPosition =
                        getBoardPosition(screenRow, screenColumn);

                    const int column = boardPosition.x;
                    const int row = boardPosition.y;

                    if (row < 0 || row >= 8 ||
                        column < 0 || column >= 8)
                    {
                        continue;
                    }

                    // Nothing selected yet
                    if (!selected)
                    {
                        const chess::Piece& piece =
                            board.getPiece(row, column);

                        if (!piece.isEmpty() &&
                            piece.color == currentTurn)
                        {
                            selected = true;

                            selectedRow = row;
                            selectedColumn = column;

                            holdingPiece = true;
                            draggingPiece = false;

                            pressPosition =
                                mouseButton->position;
                        }
                    }
                    else
                    {
                        // A piece is already selected.

                        clickedSelectedPiece =
                            row == selectedRow &&
                            column == selectedColumn;

                        holdingPiece = true;
                        draggingPiece = false;

                        pressPosition =
                            mouseButton->position;
                    }
                }
            }

            // --------------------------------
            // Mouse moved
            // --------------------------------

            if (const auto* mouseMoved =
                    event->getIf<sf::Event::MouseMoved>())
            {
                // Update hovered square
                const int screenColumn =
                    mouseMoved->position.x / 100;

                const int screenRow =
                    mouseMoved->position.y / 100;

                if (screenRow >= 0 && screenRow < 8 &&
                    screenColumn >= 0 && screenColumn < 8)
                {
                    const sf::Vector2i boardPosition =
                        getBoardPosition(screenRow, screenColumn);

                    hoverRow = boardPosition.y;
                    hoverColumn = boardPosition.x;
                }
                else
                {
                    hoverRow = -1;
                    hoverColumn = -1;
                }

                // Existing drag detection
                if (holdingPiece && !draggingPiece)
                {
                    const float dx =
                        static_cast<float>(
                            mouseMoved->position.x -
                            pressPosition.x
                        );

                    const float dy =
                        static_cast<float>(
                            mouseMoved->position.y -
                            pressPosition.y
                        );

                    const float distance =
                        std::sqrt(dx * dx + dy * dy);

                    if (distance >= DragThreshold)
                    {
                        draggingPiece = true;
                    }
                }
            }

            // --------------------------------
            // Mouse released
            // --------------------------------

            if (const auto* mouseButton =
                    event->getIf<sf::Event::MouseButtonReleased>())
            {   
                if (promotionPending)
                {
                    constexpr int MenuX = 200;
                    constexpr int MenuY = 350;
                    constexpr int OptionSize = 100;

                    const int mouseX =
                        mouseButton->position.x;

                    const int mouseY =
                        mouseButton->position.y;

                    if (mouseX >= MenuX &&
                        mouseX < MenuX + OptionSize * 4 &&
                        mouseY >= MenuY &&
                        mouseY < MenuY + OptionSize)
                    {
                        const int option =
                            (mouseX - MenuX) / OptionSize;

                        handlePromotion(option);
                    }

                    continue;
                }
                if (mouseButton->button ==
                    sf::Mouse::Button::Left &&
                    holdingPiece)
                {
                    const int screenColumn =
                        mouseButton->position.x / 100;

                    const int screenRow =
                        mouseButton->position.y / 100;

                    const sf::Vector2i boardPosition =
                        getBoardPosition(screenRow, screenColumn);

                    const int column = boardPosition.x;
                    const int row = boardPosition.y;

                    if (draggingPiece)
                    {
                        // Drag-and-drop
                        if (row >= 0 && row < 8 &&
                            column >= 0 && column < 8)
                        {
                            if (row != selectedRow ||
                                column != selectedColumn)
                            {
                                if (chess::MoveValidator::isValidMove(
                                        board,
                                        selectedRow,
                                        selectedColumn,
                                        row,
                                        column))
                                {
                                    completeMove(row, column);
                                }
                            }
                        }
                    }
                    else
                    {
                        // Normal point-and-click movement.

                        if (row >= 0 && row < 8 &&
                            column >= 0 && column < 8)
                        {   
                            if (clickedSelectedPiece)
                            {
                                // Clicked the selected piece again
                                selected = false;
                                selectedRow = -1;
                                selectedColumn = -1;
                            }
                            else if (row != selectedRow ||
                                column != selectedColumn)
                            {
                                const chess::Piece& clickedPiece =
                                    board.getPiece(row, column);

                                if (!clickedPiece.isEmpty() &&
                                    clickedPiece.color == currentTurn)
                                {
                                    selectedRow = row;
                                    selectedColumn = column;
                                }
                                else if (chess::MoveValidator::isValidMove(
                                            board,
                                            selectedRow,
                                            selectedColumn,
                                            row,
                                            column))
                                {
                                    completeMove(row, column);
                                }
                            }
                        }
                    }

                    holdingPiece = false;
                    draggingPiece = false;
                    clickedSelectedPiece = false;
                }
            }
        }

        window.clear();

        if (!gameStarted)
        {
            sf::RectangleShape background(
                sf::Vector2f(1100.0f, 900.0f)
            );
            background.setFillColor(sf::Color(35, 35, 35));
            window.draw(background);

            sf::Text title(font, "Choose your side");
            title.setCharacterSize(42);
            title.setFillColor(sf::Color::White);
            title.setPosition({365.0f, 285.0f});
            window.draw(title);

            sf::RectangleShape whiteButton(
                sf::Vector2f(220.0f, 100.0f)
            );
            whiteButton.setPosition({320.0f, 430.0f});
            whiteButton.setFillColor(sf::Color(240, 217, 181));
            window.draw(whiteButton);

            sf::RectangleShape blackButton(
                sf::Vector2f(220.0f, 100.0f)
            );
            blackButton.setPosition({560.0f, 430.0f});
            blackButton.setFillColor(sf::Color(75, 75, 75));
            window.draw(blackButton);

            sf::Text whiteText(font, "Play White");
            whiteText.setCharacterSize(26);
            whiteText.setFillColor(sf::Color::Black);
            whiteText.setPosition({360.0f, 465.0f});
            window.draw(whiteText);

            sf::Text blackText(font, "Play Black");
            blackText.setCharacterSize(26);
            blackText.setFillColor(sf::Color::White);
            blackText.setPosition({600.0f, 465.0f});
            window.draw(blackText);

            sf::Text difficultyTitle(font, "AI difficulty");
            difficultyTitle.setCharacterSize(28);
            difficultyTitle.setFillColor(sf::Color::White);
            difficultyTitle.setPosition({465.0f, 555.0f});
            window.draw(difficultyTitle);

            const sf::Color inactiveDifficultyColor(
                75,
                75,
                75
            );

            const sf::Color activeDifficultyColor(
                65,
                125,
                190
            );

            sf::RectangleShape easyButton(
                sf::Vector2f(200.0f, 65.0f)
            );

            easyButton.setPosition({230.0f, 610.0f});
            easyButton.setFillColor(
                selectedDifficulty == chess::AIDifficulty::Easy
                    ? activeDifficultyColor
                    : inactiveDifficultyColor
            );
            easyButton.setOutlineColor(sf::Color::White);
            easyButton.setOutlineThickness(2.0f);
            window.draw(easyButton);

            sf::Text easyText(font, "Easy");
            easyText.setCharacterSize(24);
            easyText.setFillColor(sf::Color::White);
            easyText.setPosition({300.0f, 628.0f});
            window.draw(easyText);

            sf::RectangleShape mediumButton(
                sf::Vector2f(200.0f, 65.0f)
            );

            mediumButton.setPosition({450.0f, 610.0f});
            mediumButton.setFillColor(
                selectedDifficulty == chess::AIDifficulty::Medium
                    ? activeDifficultyColor
                    : inactiveDifficultyColor
            );
            mediumButton.setOutlineColor(sf::Color::White);
            mediumButton.setOutlineThickness(2.0f);
            window.draw(mediumButton);

            sf::Text mediumText(font, "Medium");
            mediumText.setCharacterSize(24);
            mediumText.setFillColor(sf::Color::White);
            mediumText.setPosition({510.0f, 628.0f});
            window.draw(mediumText);

            sf::RectangleShape hardButton(
                sf::Vector2f(200.0f, 65.0f)
            );

            hardButton.setPosition({670.0f, 610.0f});
            hardButton.setFillColor(
                selectedDifficulty == chess::AIDifficulty::Hard
                    ? activeDifficultyColor
                    : inactiveDifficultyColor
            );
            hardButton.setOutlineColor(sf::Color::White);
            hardButton.setOutlineThickness(2.0f);
            window.draw(hardButton);

            sf::Text hardText(font, "Hard");
            hardText.setCharacterSize(24);
            hardText.setFillColor(sf::Color::White);
            hardText.setPosition({745.0f, 628.0f});
            window.draw(hardText);
        }
        else
        {
            // Don't draw the original piece while dragging.
            if (selected && draggingPiece)
            {
                renderer.drawBoard(
                    board,
                    playerColor,
                    selectedRow,
                    selectedColumn
                );
            }
            else
            {
                renderer.drawBoard(board, playerColor);
            }

            drawBoardCoordinates();

            if (hoverRow != -1 && hoverColumn != -1 && draggingPiece)
            {
                renderer.drawHover(
                    hoverRow,
                    hoverColumn,
                    playerColor
                );
            }

            // Draw selection highlight.
            if (selected)
            {
                for (int toRow = 0;
                    toRow < chess::Board::Size;
                    ++toRow)
                {
                    for (int toColumn = 0;
                        toColumn < chess::Board::Size;
                        ++toColumn)
                    {
                        if (!chess::MoveValidator::isValidMove(
                                board,
                                selectedRow,
                                selectedColumn,
                                toRow,
                                toColumn))
                        {
                            continue;
                        }

                        const bool isCapture =
                            !board.getPiece(
                                toRow,
                                toColumn
                            ).isEmpty();

                        renderer.drawMoveHint(
                            toRow,
                            toColumn,
                            playerColor,
                            isCapture
                        );
                    }
                }

                renderer.drawSelection(
                    selectedRow,
                    selectedColumn,
                    playerColor
                );
            }

            // Draw dragged piece on top of everything.
            if (selected && draggingPiece)
            {
                const chess::Piece& piece =
                    board.getPiece(
                        selectedRow,
                        selectedColumn
                    );

                const sf::Vector2i mousePosition =
                    sf::Mouse::getPosition(window);

                renderer.drawDraggedPiece(
                    piece,
                    sf::Vector2f(
                        static_cast<float>(mousePosition.x),
                        static_cast<float>(mousePosition.y)
                    )
                );
            }

            if (promotionPending)
            {
                const chess::Piece& pawn =
                    board.getPiece(
                        promotionRow,
                        promotionColumn
                    );

                renderer.drawPromotionMenu(
                    pawn.color
                );
            }

            sf::RectangleShape controlBar(
                sf::Vector2f(800.0f, 100.0f)
            );

            controlBar.setPosition({0.0f, 800.0f});
            controlBar.setFillColor(sf::Color(45, 45, 45));
            window.draw(controlBar);

            sf::RectangleShape restartButton(
                sf::Vector2f(160.0f, 60.0f)
            );

            restartButton.setPosition({620.0f, 820.0f});
            restartButton.setFillColor(sf::Color(175, 65, 65));
            restartButton.setOutlineColor(sf::Color::White);
            restartButton.setOutlineThickness(2.0f);
            window.draw(restartButton);

            sf::Text restartText(font, "Restart");
            restartText.setCharacterSize(24);
            restartText.setFillColor(sf::Color::White);
            restartText.setPosition({655.0f, 836.0f});
            window.draw(restartText);

            sf::RectangleShape forfeitButton(
                sf::Vector2f(160.0f, 60.0f)
            );

            forfeitButton.setPosition({440.0f, 820.0f});
            forfeitButton.setFillColor(sf::Color(115, 75, 75));
            forfeitButton.setOutlineColor(sf::Color::White);
            forfeitButton.setOutlineThickness(2.0f);
            window.draw(forfeitButton);

            sf::Text forfeitText(font, "Forfeit");
            forfeitText.setCharacterSize(24);
            forfeitText.setFillColor(sf::Color::White);
            forfeitText.setPosition({478.0f, 836.0f});
            window.draw(forfeitText);

            statusText.setPosition({20.0f, 835.0f});
            window.draw(statusText);

            sf::RectangleShape movePanel(
                sf::Vector2f(300.0f, 900.0f)
            );

            movePanel.setPosition({800.0f, 0.0f});
            movePanel.setFillColor(sf::Color(35, 35, 35));
            window.draw(movePanel);

            sf::Text moveTitle(font, "Move history");
            moveTitle.setCharacterSize(28);
            moveTitle.setFillColor(sf::Color::White);
            moveTitle.setPosition({825.0f, 25.0f});
            window.draw(moveTitle);

            std::string moveListText;

            const std::size_t firstMove =
                moveHistory.size() > 24
                    ? moveHistory.size() - 24
                    : 0;

            for (std::size_t index = firstMove;
                index < moveHistory.size();
                index += 2)
            {
                moveListText +=
                    std::to_string(index / 2 + 1) +
                    ". " +
                    moveHistory[index];

                if (index + 1 < moveHistory.size())
                {
                    moveListText +=
                        "   " +
                        moveHistory[index + 1];
                }

                moveListText += "\n";
            }

            sf::Text moveList(font, moveListText);
            moveList.setCharacterSize(20);
            moveList.setFillColor(sf::Color(235, 235, 235));
            moveList.setPosition({825.0f, 80.0f});
            window.draw(moveList);

            if (gameOver)
            {
                sf::RectangleShape overlay(
                    sf::Vector2f(800.0f, 800.0f)
                );

                overlay.setFillColor(sf::Color(0, 0, 0, 120));
                window.draw(overlay);

                sf::RectangleShape menuButton(
                    sf::Vector2f(300.0f, 80.0f)
                );

                menuButton.setPosition({250.0f, 360.0f});
                menuButton.setFillColor(sf::Color(65, 110, 165));
                menuButton.setOutlineColor(sf::Color::White);
                menuButton.setOutlineThickness(3.0f);
                window.draw(menuButton);

                sf::Text menuText(font, "Back to menu");
                menuText.setCharacterSize(28);
                menuText.setFillColor(sf::Color::White);
                menuText.setPosition({292.0f, 384.0f});
                window.draw(menuText);
            }

        }
        
        window.display();
    }

    return 0;
}