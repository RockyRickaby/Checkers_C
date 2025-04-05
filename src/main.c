#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stddef.h>

// #include "checkers.h"
#include "terminal_ui.h"
#include "gui.h"

#include "new_checkers.h"

int main(int argc, char const *argv[]) {
    struct Checkers game;
    // checkersInit(&game, 1, 1, 1, 0);
    // terminalCheckersBeginF(&game, stdin);

    // printf("%d\n", checkersMakeMove(&game, 30, 26));
    // printf("%d\n", checkersMakeMove(&game, 16, 20));
    // printf("%d\n", checkersMakeMove(&game, 26, 21));
    // printf("%d\n", checkersMakeMove(&game, 17, 26));
    // printf("%d\n", checkersMakeMove(&game, 36, 30));
    // printf("%d\n", checkersMakeMove(&game, 12, 17));

    // printf("%d\n", checkersMakeMove(&game, 34, 29));
    // printf("%d\n", checkersMakeMove(&game, 18, 23));
    // printf("%d\n", checkersMakeMove(&game, 32, 27));
    // printf("%d\n", checkersMakeMove(&game, 23, 34));
    // printf("%d\n", checkersMakeMove(&game, 27, 22));
    // printf("%d\n", checkersMakeMove(&game, 13, 18));
    // printf("%d\n", checkersMakeMove(&game, 30, 25));
    // printf("%d\n", checkersMakeMove(&game, 18, 27));
    // printf("%d\n", checkersMakeMove(&game, 27, 36));
    
    // printf("%d\n", checkersMakeMove(&game, 41, 30));
    // printf("%d\n", checkersMakeMove(&game, 25, 16));
    // printf("%d\n", checkersMakeMove(&game, 30, 21));
    // printf("%d\n", checkersMakeMove(&game, 21, 12));

    // printf("%d\n", checkersMakeMove(&game, 30, 21));
    // printf("%d\n", checkersMakeMove(&game, 21, 12));
    // printf("%d\n", checkersMakeMove(&game, 12, 23));
    // checkersPrint(&game);

    GameWindow window = {0};
    checkersInit(&game, 1, 1, 1, 0);
    // guiCreateWindow(
    //     &window, 800, 800, 1,
    //     FLAG_WINDOW_RESIZABLE | FLAG_VSYNC_HINT | FLAG_MSAA_4X_HINT | FLAG_WINDOW_ALWAYS_RUN
    // );
    // guiGameBeginW(&window, &game);
    guiGameBegin(&game);

    
    // struct Board board = {0};
    // boardInit(&board);
    // memset(board.board, 0, sizeof(Piece) * board.boardSize);
    // board.board[27] = (Piece) {
    //     .alive = 1,
    //     .type = PIECE_DARK_MAN,
    //     .recentlyMoved = 0,
    // };

    // board.board[26] = (Piece) {
    //     .alive = 1,
    //     .type = PIECE_DARK_MAN,
    //     .recentlyMoved = 0,
    // };

    // board.board[28] = (Piece) {
    //     .alive = 1,
    //     .type = PIECE_DARK_MAN,
    //     .recentlyMoved = 0,
    // };

    // board.board[33] = (Piece) {
    //     .alive = 1,
    //     .type = PIECE_LIGHT_MAN,
    //     .recentlyMoved = 0,
    // };


    // Checkers chek = {0};
    // checkersInit(&chek, 1, 1, 1, 1);
    // free(chek.checkersBoard.board);
    // chek.checkersBoard.board = board.board;

    // checkersPrint(&chek);

    // printf("%d\n", checkersLoadCaptureStreak(&chek));
    // int step = 0;
    // while ((step = checkersCaptureStreakNext(&chek)) != -1) {
    //     printf("%d\n", step);
    // }
    // checkersUnloadCaptureStreak(&chek);



    // struct Board board = {0};
    // boardInit(&board);
    // const int pos = 13;
    // const int dest = 5;
    // memset(board.board, 0, sizeof(Piece) * board.boardSize);
    // board.board[8] = (Piece) {
    //     .alive = 1,
    //     .type = PIECE_DARK_MAN,
    //     .recentlyMoved = 0,
    // };

    // board.board[9] = (Piece) {
    //     .alive = 1,
    //     .type = PIECE_DARK_MAN,
    //     .recentlyMoved = 0,
    // };

    // board.board[17] = (Piece) {
    //     .alive = 1,
    //     .type = PIECE_DARK_MAN,
    //     .recentlyMoved = 0,
    // };

    // board.board[19] = (Piece) {
    //     .alive = 1,
    //     .type = PIECE_DARK_MAN,
    //     .recentlyMoved = 0,
    // };

    // board.board[26] = (Piece) {
    //     .alive = 1,
    //     .type = PIECE_DARK_MAN,
    //     .recentlyMoved = 0,
    // };

    // board.board[22] = (Piece) {
    //     .alive = 1,
    //     .type = PIECE_LIGHT_MAN,
    //     .recentlyMoved = 0,
    // };

    // board.board[32] = (Piece) {
    //     .alive = 1,
    //     .type = PIECE_LIGHT_MAN,
    //     .recentlyMoved = 0,
    // };

    // board.board[33] = (Piece) {
    //     .alive = 1,
    //     .type = PIECE_LIGHT_MAN,
    //     .recentlyMoved = 0,
    // };

    // board.board[37] = (Piece) {
    //     .alive = 1,
    //     .type = PIECE_LIGHT_MAN,
    //     .recentlyMoved = 0,
    // };

    // board.board[43] = (Piece) {
    //     .alive = 1,
    //     .type = PIECE_LIGHT_MAN,
    //     .recentlyMoved = 0,
    // };

    // board.board[pos] = (Piece) {
    //     .alive = 1,
    //     .type = PIECE_DARK_KING,
    //     .recentlyMoved = 0,
    // };
    // // printf("Counters: Men %d, Kings %d\n", board.remainingLightMen, board.remainingLightKings);
    // // boardPrint(&board);
    // printf("%d\n", boardTryMoveOrCapture(&board, CHECKERS_PLAYER_ONE, 66, 11));
    // // boardUpdate(&board);
    // printf("%d\n", boardTryMoveOrCapture(&board, CHECKERS_PLAYER_ONE, 11, 25));
    // printf("recently moved %d, flag %d\n", board.recentlyMovedPiece, boardGetPieceAt(&board, board.recentlyMovedPiece).recentlyMoved);
    // // printf("Counters: Men %d, Kings %d\n", board.remainingLightMen, board.remainingLightKings);
    // // int* buf = NULL;
    // // size_t s = boardGetAvailableMovesForPiece(&board, pos, &buf);
    // // printf("Moves: %lld\n", s);
    // // for (size_t i = 0; i < s; i++) {
    // //     printf("%d\n", buf[i]);
    // // }
    // // free(buf);
    // size_t si = 0;
    // Moves* mov = boardGetAvailableMovesForPlayer(&board, CHECKERS_PLAYER_LIGHT, &si);
    // for (size_t i = 0; i < si; i++) {
    //     printf("%lld -> ", i);
    //     for (size_t j = 0; j < mov[i].to_size; j++) {
    //         printf("%d ", mov[i].to[j]);
    //     }
    //     printf("\n");
    // }
    // boardDestroyMovesList(mov, si);
    // printf("Dark, capture? %d\n", boardCheckIfPlayerCanCapture(&board, CHECKERS_PLAYER_DARK));
    // printf("Light, capture? %d\n", boardCheckIfPlayerCanCapture(&board, CHECKERS_PLAYER_LIGHT));

    // int* buf = NULL;
    // size_t s = boardGetLongestCaptureStreakForPiece(&board, pos, &buf);
    // printf("%llu\n", s);
    // for (size_t i = 0; i < s; i++) {
    //     printf("%d ", buf[i]);
    // }
    // printf("\n");
    // boardPrint(&board);

    // int* allbuf = NULL;
    // size_t bufs = boardGetLongestCaptureStreakForPlayer(&board, CHECKERS_PLAYER_LIGHT, &allbuf);

    // for (size_t i = 0; i < bufs; i++) {
    //     printf("%d ", allbuf[i]);
    // }
    // printf("|%*s|\n", -16, "success");
    // small macro test
    // for (int i = 0; i < board.boardSize; i++) {
    //     Point p = IDX_TO_POINT(board.boardSize - 1 - i);
    //     printf("(%d,%d) - %d = %d\n", p.x, p.y, board.boardSize - 1 - i, POINT_TO_IDX(p.x, p.y));
    // }
    return 0;
}
