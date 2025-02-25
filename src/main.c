#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stddef.h>

// #include "checkers.h"
// #include "terminal_ui.h"
// #include "gui.h"

#include "new_checkers.h"

typedef struct Node {
    struct Node* next;
    int value;
} Node;

typedef struct CaptureStack {
    Node* top;
    size_t size;
} CaptureStack;

void test(CaptureStack* stack, int level) {
    if (level == 0) {
        for (Node* curr = stack->top; curr != NULL; curr = curr->next) {
            printf("%d ", curr->value);
        }
        printf("\n");
        return;
    }
    /**
     * exploitable!!!
     * I assume this is not safe...
     */
    Node t = {
        .next = NULL,
        .value = level,
    };
    t.next = stack->top;
    stack->top = &t;
    stack->size++;
    test(stack, level - 1);
}

int main(int argc, char const *argv[]) {
    test(&(CaptureStack) { .top = NULL, .size = 0}, 3);
    // struct Checkers game;
    // checkersInit(&game, 1, 1);
    // terminalCheckersBeginF(&game, stdin);
    // guiGameBegin(&game);

    struct Board board = {0};
    boardInit(&board);
    const int pos = 17;
    const int dest = 5;
    memset(board.board, 0, sizeof(Piece) * board.boardSize);
    board.board[10] = (Piece) {
        .alive = 1,
        .type = PIECE_DARK_MAN,
        .recentlyMoved = 0,
    };

    board.board[11] = (Piece) {
        .alive = 1,
        .type = PIECE_DARK_MAN,
        .recentlyMoved = 0,
    };

    board.board[13] = (Piece) {
        .alive = 1,
        .type = PIECE_DARK_MAN,
        .recentlyMoved = 0,
    };

    board.board[22] = (Piece) {
        .alive = 1,
        .type = PIECE_DARK_MAN,
        .recentlyMoved = 0,
    };

    board.board[23] = (Piece) {
        .alive = 1,
        .type = PIECE_DARK_MAN,
        .recentlyMoved = 0,
    };

    board.board[32] = (Piece) {
        .alive = 1,
        .type = PIECE_DARK_MAN,
        .recentlyMoved = 0,
    };


    board.board[pos] = (Piece) {
        .alive = 1,
        .type = PIECE_LIGHT_MAN,
        .recentlyMoved = 0,
    };
    // printf("Counters: Men %d, Kings %d\n", board.remainingLightMen, board.remainingLightKings);
    // boardPrint(&board);
    printf("%d\n", boardTryMoveOrCapture(&board, CHECKERS_PLAYER_ONE, 66, 11));
    // boardUpdate(&board);
    printf("%d\n", boardTryMoveOrCapture(&board, CHECKERS_PLAYER_ONE, 11, 25));
    printf("recently moved %d, flag %d\n", board.recentlyMovedPiece, boardGetPieceAt(&board, board.recentlyMovedPiece).recentlyMoved);
    // printf("Counters: Men %d, Kings %d\n", board.remainingLightMen, board.remainingLightKings);
    boardPrint(&board);
    // int* buf = NULL;
    // size_t s = boardGetAvailableMovesForPiece(&board, pos, &buf);
    // printf("Moves: %lld\n", s);
    // for (size_t i = 0; i < s; i++) {
    //     printf("%d\n", buf[i]);
    // }
    // free(buf);
    size_t si = 0;
    Moves* mov = boardGetAvailableMovesForPlayer(&board, CHECKERS_PLAYER_LIGHT, &si);
    for (size_t i = 0; i < si; i++) {
        printf("%lld -> ", i);
        for (size_t j = 0; j < mov[i].to_size; j++) {
            printf("%d ", mov[i].to[j]);
        }
        printf("\n");
    }
    boardDestroyMovesList(mov, si);
    printf("Dark, capture? %d\n", boardCheckIfPlayerCanCapture(&board, CHECKERS_PLAYER_DARK));
    printf("Light, capture? %d\n", boardCheckIfPlayerCanCapture(&board, CHECKERS_PLAYER_LIGHT));

    int* buf = NULL;
    size_t s = boardGetLongestCaptureStreakForPiece(&board, pos, &buf);
    printf("%llu\n", s);
    for (size_t i = 0; i < s; i++) {
        printf("%d ", buf[i]);
    }
    printf("\n");
    // small macro test
    // for (int i = 0; i < board.boardSize; i++) {
    //     Point p = IDX_TO_POINT(board.boardSize - 1 - i);
    //     printf("(%d,%d) - %d = %d\n", p.x, p.y, board.boardSize - 1 - i, POINT_TO_IDX(p.x, p.y));
    // }
    return 0;
}
