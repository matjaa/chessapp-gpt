#include "graphics/Renderer.h"

#include <algorithm>
#include <stdexcept>

namespace graphics
{
    Renderer::Renderer(sf::RenderWindow& window)
        : window(window)
    {
        loadTextures();
    }

    void Renderer::loadTextures()
    {
        const std::string piecesPath = "assets/pieces/";

        const std::string colors[] = {
            "White",
            "Black"
        };

        const std::string pieces[] = {
            "Pawn",
            "Knight",
            "Bishop",
            "Rook",
            "Queen",
            "King"
        };

        for (const auto& color : colors)
        {
            for (const auto& piece : pieces)
            {
                const std::string name =
                    piece + " " +
                    color +
                    " Wood Outline 288px.png";

                sf::Texture texture;

                if (!texture.loadFromFile(piecesPath + name))
                {
                    throw std::runtime_error(
                        "Failed to load chess piece: " +
                        piecesPath + name
                    );
                }

                textures[name] = std::move(texture);
            }
        }
    }

    std::string Renderer::getTextureName(
        chess::PieceType type,
        chess::PieceColor color
    ) const
    {
        std::string piece;

        switch (type)
        {
        case chess::PieceType::Pawn:
            piece = "Pawn";
            break;

        case chess::PieceType::Knight:
            piece = "Knight";
            break;

        case chess::PieceType::Bishop:
            piece = "Bishop";
            break;

        case chess::PieceType::Rook:
            piece = "Rook";
            break;

        case chess::PieceType::Queen:
            piece = "Queen";
            break;

        case chess::PieceType::King:
            piece = "King";
            break;

        default:
            return "";
        }

        std::string colorName;

        switch (color)
        {
        case chess::PieceColor::White:
            colorName = "White";
            break;

        case chess::PieceColor::Black:
            colorName = "Black";
            break;

        default:
            return "";
        }

        return piece + " " +
               colorName +
               " Wood Outline 288px.png";
    }

    sf::Vector2f Renderer::getScreenPosition(
        int row,
        int column,
        chess::PieceColor perspective
    ) const
    {
        if (perspective == chess::PieceColor::Black)
        {
            row = chess::Board::Size - 1 - row;
            column = chess::Board::Size - 1 - column;
        }

        return {
            column * SquareSize,
            row * SquareSize
        };
    }

    void Renderer::drawBoard(
        const chess::Board& board,
        chess::PieceColor perspective,
        int excludedRow,
        int excludedColumn
    )
    {
        sf::RectangleShape square(
            sf::Vector2f(SquareSize, SquareSize)
        );

        for (int row = 0; row < board.getSize(); ++row)
        {
            for (int column = 0; column < board.getSize(); ++column)
            {
                square.setPosition(
                    getScreenPosition(row, column, perspective)
                );

                if ((row + column) % 2 == 0)
                {
                    square.setFillColor(sf::Color(240, 217, 181));
                }
                else
                {
                    square.setFillColor(sf::Color(181, 136, 99));
                }

                window.draw(square);
            }
        }

        drawPieces(
            board,
            perspective,
            excludedRow,
            excludedColumn
        );
    }

    void Renderer::drawPieces(
        const chess::Board& board,
        chess::PieceColor perspective,
        int excludedRow,
        int excludedColumn
    )
    {
        for (int row = 0; row < board.getSize(); ++row)
        {
            for (int column = 0; column < board.getSize(); ++column)
            {
                const chess::Piece& piece =
                    board.getPiece(row, column);

                if (piece.isEmpty())
                {
                    continue;
                }

                if (row == excludedRow &&
                    column == excludedColumn)
                {
                    continue;
                }

                const std::string textureName =
                    getTextureName(
                        piece.type,
                        piece.color
                    );

                auto textureIt = textures.find(textureName);

                if (textureIt == textures.end())
                {
                    continue;
                }

                sf::Sprite sprite(textureIt->second);

                const sf::Vector2u textureSize =
                    textureIt->second.getSize();

                const float scale =
                    SquareSize /
                    static_cast<float>(
                        std::max(
                            textureSize.x,
                            textureSize.y
                        )
                    );

                sprite.setScale({
                    scale,
                    scale
                });

                const float width =
                    textureSize.x * scale;

                const float height =
                    textureSize.y * scale;

                const sf::Vector2f position =
                    getScreenPosition(row, column, perspective);

                sprite.setPosition({
                    position.x + (SquareSize - width) / 2.0f,
                    position.y + (SquareSize - height) / 2.0f
                });

                window.draw(sprite);
            }
        }
    }

    void Renderer::drawDraggedPiece(
        const chess::Piece& piece,
        sf::Vector2f mousePosition
    )
    {
        if (piece.isEmpty())
        {
            return;
        }

        const std::string textureName =
            getTextureName(
                piece.type,
                piece.color
            );

        auto textureIt = textures.find(textureName);

        if (textureIt == textures.end())
        {
            return;
        }

        sf::Sprite sprite(textureIt->second);

        const sf::Vector2u textureSize =
            textureIt->second.getSize();

        const float scale =
            SquareSize /
            static_cast<float>(
                std::max(
                    textureSize.x,
                    textureSize.y
                )
            );

        sprite.setScale({
            scale,
            scale
        });

        const float width =
            textureSize.x * scale;

        const float height =
            textureSize.y * scale;

        sprite.setPosition({
            mousePosition.x - width / 2.0f,
            mousePosition.y - height / 2.0f
        });

        window.draw(sprite);
    }

    void Renderer::drawSelection(
        int row, 
        int column, 
        chess::PieceColor perspective
    )
    {
        sf::RectangleShape highlight(
            sf::Vector2f(SquareSize, SquareSize)
        );

        highlight.setPosition(
            getScreenPosition(row, column, perspective)
        );

        highlight.setFillColor(
            sf::Color(255, 255, 0, 70)
        );

        highlight.setOutlineColor(
            sf::Color(255, 220, 0, 180)
        );

        highlight.setOutlineThickness(3.0f);

        window.draw(highlight);
    }

    void Renderer::drawHover(
        int row, 
        int column,
        chess::PieceColor perspective
    )
    {
        sf::RectangleShape highlight(
            sf::Vector2f(SquareSize, SquareSize)
        );

        highlight.setPosition(
            getScreenPosition(row, column, perspective)
        );

        highlight.setFillColor(
            sf::Color(255, 255, 255, 40)
        );

        window.draw(highlight);
    }

    void Renderer::drawMoveHint(
        int row,
        int column,
        chess::PieceColor perspective,
        bool isCapture
    )
    {
        const sf::Vector2f squarePosition =
            getScreenPosition(row, column, perspective);

        if (isCapture)
        {
            sf::CircleShape ring(38.0f);

            ring.setPosition({
                squarePosition.x + 12.0f,
                squarePosition.y + 12.0f
            });

            ring.setFillColor(sf::Color::Transparent);
            ring.setOutlineColor(sf::Color(50, 190, 90, 220));
            ring.setOutlineThickness(7.0f);

            window.draw(ring);
        }
        else
        {
            sf::CircleShape dot(14.0f);

            dot.setPosition({
                squarePosition.x + 36.0f,
                squarePosition.y + 36.0f
            });

            dot.setFillColor(sf::Color(50, 190, 90, 190));

            window.draw(dot);
        }
    }

    void Renderer::drawPromotionMenu(
        chess::PieceColor color
    )
    {
        constexpr float MenuX = 200.0f;
        constexpr float MenuY = 350.0f;
        constexpr float OptionSize = 100.0f;

        sf::RectangleShape background(
            sf::Vector2f(
                OptionSize * 4.0f,
                OptionSize
            )
        );

        background.setPosition({
            MenuX,
            MenuY
        });

        background.setFillColor(
            sf::Color(40, 40, 40, 240)
        );

        background.setOutlineColor(
            sf::Color::White
        );

        background.setOutlineThickness(3.0f);

        window.draw(background);

        const chess::PieceType promotionTypes[] = {
            chess::PieceType::Queen,
            chess::PieceType::Rook,
            chess::PieceType::Bishop,
            chess::PieceType::Knight
        };

        for (int i = 0; i < 4; ++i)
        {
            const chess::Piece piece{
                promotionTypes[i],
                color
            };

            drawDraggedPiece(
                piece,
                sf::Vector2f{
                    MenuX +
                        i * OptionSize +
                        OptionSize / 2.0f,

                    MenuY +
                        OptionSize / 2.0f
                }
            );
        }
    }
}