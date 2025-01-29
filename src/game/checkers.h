#ifndef CHECKERS_BOARD_H
#define CHECKERS_BOARD_H

#define CHECKERS_PLAYER_ONE          0
#define CHECKERS_PLAYER_TWO          1

#define CHECKERS_BOARD_SIZE         10
#define CHECKERS_PIECES_AMOUNT      (CHECKERS_BOARD_SIZE / 2) * ((CHECKERS_BOARD_SIZE - 2) / 2)

#define CHECKERS_CAPTURE_SUCCESS     2
#define CHECKERS_MOVE_SUCCESS        1
#define CHECKERS_NULL_BOARD         -1
#define CHECKERS_MOVE_FAIL          -2
#define CHECKERS_INVALID_MOVE       -3
#define CHECKERS_INVALID_PLAYER     -4
#define CHECKERS_NOT_A_PIECE        -5
// #define CHECKERS_NULL_MOVES_LIST    -6
// #define CHECKERS_EMPTY_MOVES_LIST   -7
// #define CHECKERS_INVALID_MOVES_LIST -8

#include <stddef.h>

struct Point {
    int x, y;
};

struct Board {
    unsigned int boardSize;
    unsigned int remainingLightPieces;
    unsigned int remainingDarkPieces;
    char pieceLightMan;
    char pieceLightKing;
    char pieceDarkMan;
    char pieceDarkKing;
    char blank;
    char board[CHECKERS_BOARD_SIZE][CHECKERS_BOARD_SIZE];
};

enum GameState {
    CSTATE_P1_TURN,
    CSTATE_P2_TURN,
    CSTATE_END_P1_WIN,
    CSTATE_END_P2_WIN,
    CSTATE_END_DRAW
};

struct Checkers {
    struct {
        int forceCapture : 1;
        int run : 1;
    } flags;
    int turnsTotal;
    enum GameState state;
    struct Board checkersBoard;
};

int boardInit(struct Board* gameboard);
int boardTryMoveOrCapture(struct Board* gameboard, int player, struct Point piecePos, struct Point newPos);
// int boardMultiMoveCapture(struct Board* gameboard, int player, struct Point piecePos, struct Point* movesList, size_t listSize);
void boardTryTurnKing(struct Board* gameboard, struct Point piecePos);
int boardRemainingPiecesTotal(struct Board* gameboard);
int boardRemainingPiecesPlayer(struct Board* gameboard, int player);
void boardPrint(struct Board* gameboard);

// ---

int checkersInit(struct Checkers* game, int forceCapture);
int checkersMakeMove(struct Checkers* game, struct Point from, struct Point to);
int checkersGetCurrentPlayer(struct Checkers* game);
int checkersGetWinner(struct Checkers* game);
// int checkersGetClosestEnemies(struct Checkers* game, struct Point playerPos, struct Point* enemiesPos); /* receives output buffer and returns size */
int checkersPlayerShallCapture(struct Checkers* game);
void checkersPrint(struct Checkers* game);

#endif /* CHECKERS_BOARD_H */