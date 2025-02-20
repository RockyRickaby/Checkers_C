#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stddef.h>

// #include "checkers.h"
// #include "terminal_ui.h"
// #include "gui.h"

#include "new_checkers.h"

int main(int argc, char const *argv[]) {
    // struct Checkers game;
    // checkersInit(&game, 1, 1);
    // terminalCheckersBeginF(&game, stdin);
    // guiGameBegin(&game);

    struct Board board = {0};
    boardInit(&board);
    const int pos = 26;
    const int dest = 12;
    memset(board.board, 0, sizeof(Piece) * board.boardSize);
    board.board[21] = (Piece) {
        .alive = 1,
        .type = PIECE_DARK_MAN,
        .enemies.man = PIECE_LIGHT_MAN,
        .enemies.king = PIECE_LIGHT_KING,
        .recentlyMoved = 0,
    };

    board.board[pos] = (Piece) {
        .alive = 1,
        .type = PIECE_LIGHT_KING,
        .enemies.man = PIECE_DARK_MAN,
        .enemies.king = PIECE_DARK_KING,
        .recentlyMoved = 0,
    };
    printf("%d\n", boardTryMoveOrCapture(&board, CHECKERS_PLAYER_ONE, pos, dest));
    // boardUpdate(&board);
    boardPrint(&board);
    // small macro test
    // for (int i = 0; i < board.boardSize; i++) {
    //     Point p = IDX_TO_POINT(board.boardSize - 1 - i);
    //     printf("(%d,%d) - %d = %d\n", p.x, p.y, board.boardSize - 1 - i, POINT_TO_IDX(p.x, p.y));
    // }
    return 0;
}
