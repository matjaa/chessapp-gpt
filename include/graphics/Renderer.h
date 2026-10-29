#pragma once

#include <SFML/Graphics.hpp>

#include <map>
#include <string>

#include "chess/Board.h"

namespace graphics
{
    class Renderer
    {
    public:
        Renderer(sf::RenderWindow& window);

        void drawBoard(
            const chess::Board& board,
            chess::PieceColor perspective,
            int excludedRow = -1,
            int excludedColumn = -1
        );

        void drawSelection(
            int row,
            int column,
            chess::PieceColor perspective
        );

        void drawHover(
            int row,
            int column,
            chess::PieceColor perspective
        );

        void drawMoveHint(
            int row,
            int column,
            chess::PieceColor perspective,
            bool isCapture
        );

        void drawDraggedPiece(
            const chess::Piece& piece,
            sf::Vector2f mousePosition
        );

        void drawPromotionMenu(
            chess::PieceColor color
        );
    private:
        sf::RenderWindow& window;

        static constexpr float SquareSize = 100.0f;

        std::map<std::string, sf::Texture> textures;

        void loadTextures();

        void drawPieces(
            const chess::Board& board,
            chess::PieceColor perspective,
            int excludedRow,
            int excludedColumn
        );

        sf::Vector2f getScreenPosition(
            int row,
            int column,
            chess::PieceColor perspective
        ) const;

        std::string getTextureName(
            chess::PieceType type,
            chess::PieceColor color
        ) const;
    };
}