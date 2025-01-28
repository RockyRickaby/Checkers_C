#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stddef.h>

#include "game/checkers.h"
#include "terminal_ui/terminal_ui.h"

int main(/*int argc, char const *argv[]*/ void)
{
    terminalGameBegin();
    // printf("Hello World!\n");
    // struct Board board;
    // boardInit(&board);
    // memset(board.board, '.', (size_t) (board.boardSize * board.boardSize));
    // // board.board[8][1] = board.pieceDarkMan;
    // board.board[7][2] = board.pieceDarkMan;
    // // board.board[6][3] = board.pieceDarkMan;
    // boardPrint(&board);
    // printf("%d\n", boardTryMoveOrCapture(&board, CHECKERS_PLAYER_TWO, (struct Point){.y = 7, .x = 2}, (struct Point){.y = 8, .x = 1}));
    // // printf("%d\n", checkersTryMoveOrCapture(&board, CHECKERS_PLAYER_TWO, (struct Point){ .y = 3, .x = 0 }, (struct Point){ .y = 4, .x = 1 }));
    // // printf("%d\n", checkersTryMoveOrCapture(&board, CHECKERS_PLAYER_ONE, (struct Point){ .y = 6, .x = 1 }, (struct Point){ .y = 5, .x = 2 }));
    // // printf("%d\n", checkersTryMoveOrCapture(&board, CHECKERS_PLAYER_ONE, (struct Point){ .y = 5, .x = 2 }, (struct Point){ .y = 3, .x = 0 }));
    // // printf("%d\n", checkersTryMoveOrCapture(&board, CHECKERS_PLAYER_ONE, (struct Point){ .y = 3, .x = 0 }, (struct Point){ .y = 4, .x = 1 }));
    // // printf("%d\n", checkersTryMoveOrCapture(&board, CHECKERS_PLAYER_TWO, (struct Point){ .y = 3, .x = 2 }, (struct Point){ .y = 5, .x = 0 }));
    // // struct Point piecePos = { .x = 1, .y = 6 };
    // // struct Point moves[3] = {
    // //     {.x = 0, .y =5 },
    // //     {.x = 1, .y =4 },
    // //     {.x = 2, .y =5 }
    // // };
    // // printf("%d\n", checkersMultiMoveCapture(&board, CHECKERS_PLAYER_ONE, piecePos, moves, 3ULL));
    // // printf("%d\n", checkersTryMoveOrCapture(&board, CHECKERS_PLAYER_ONE, piecePos, moves[0]));
    // boardPrint(&board);
    return 0;
}
