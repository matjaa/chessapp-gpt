# Chess

A desktop chess game written in C++ with SFML. Play against a Minimax-based computer opponent, choose your side, and select one of three AI difficulty levels.

## Features

- Complete standard piece movement and legal-move validation
- Check, checkmate, and stalemate detection
- Castling, en passant, and pawn promotion
- Threefold-repetition and fifty-move-rule draws
- Board rotation when playing as Black
- Click-to-move and drag-and-drop movement
- Legal-move hints for the selected piece
- Algebraic move history
- Restart, forfeit, and return-to-menu controls
- Minimax AI with alpha-beta pruning

## AI difficulty

| Difficulty | Search depth |
| --- | --- |
| Easy | 2 plies |
| Medium | 3 plies |
| Hard | 4 plies |

The AI evaluates material, pawn advancement, and central-square control. Alpha-beta pruning reduces the number of positions it needs to search.

## Controls

- On the opening screen, select **Play White** or **Play Black**.
- Choose an AI difficulty before starting.
- Click a piece, then click a highlighted destination to move it.
- You can also drag a selected piece to a legal square.
- Click a different piece of your color to switch selection.
- Use **Restart** to begin again with the current side.
- Use **Forfeit** or **Back to menu** to return to side selection.

## Building

The project uses CMake and SFML. From the project root:

```powershell
cmake -S . -B build
cmake --build build
```

Run the generated executable from the build folder. The executable needs access to the `assets/` folder for chess-piece textures and the Arial font.

## Project layout

```text
assets/          Fonts and piece textures
include/ai/      Computer-player declarations
include/chess/   Board, pieces, and move validation declarations
include/graphics/ Renderer declarations
src/ai/          Computer-player implementation
src/chess/       Board and move validation implementation
src/graphics/    Board rendering implementation
src/main.cpp     Application and user-interface loop
```
