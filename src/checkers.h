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

#include <stddef.h>
#include <stdint.h>

struct Point {
    int x, y;
};

struct Moves {
    struct Point from;
    struct Point* to;
    size_t to_size;
};

struct Board {
    uint8_t boardSize;
    uint8_t remainingLightPieces;
    uint8_t remainingDarkPieces;
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
        uint8_t forceCapture;
        uint8_t run;
        uint8_t aiEnabled;
    } flags;
    int turnsTotal;
    enum GameState state;
    struct Board checkersBoard;
};

int boardInit(struct Board* gameboard);
int boardTryMoveOrCapture(struct Board* gameboard, int player, struct Point piecePos, struct Point newPos);
void boardTryTurnKing(struct Board* gameboard, struct Point piecePos);
int boardRemainingPiecesTotal(struct Board* gameboard);
int boardRemainingPiecesPlayer(struct Board* gameboard, int player);
int boardGetAvailableMovesForPiece(struct Board* gameboard, struct Point piecePos, struct Point** out, int includeBackwardsCaptures);
struct Moves* boardGetAvailableMovesForPlayer(struct Board* gameboard, int player, int forceCapture, size_t* out_size);
int boardCheckIfPieceCanCapture(struct Board* gameboard, int player, struct Point pos);
int boardCheckIfPlayerCanCapture(struct Board* gameboard, int player);
void boardPrint(struct Board* gameboard);

// ---

int checkersInit(struct Checkers* game, int forceCapture, int enableAi);
int checkersMakeMove(struct Checkers* game, struct Point from, struct Point to);
int checkersGetCurrentPlayer(struct Checkers* game);
int checkersGetWinner(struct Checkers* game);
// int checkersGetClosestEnemies(struct Checkers* game, struct Point playerPos, struct Point* enemiesPos); /* receives output buffer and returns size */
int checkersPlayerShallCapture(struct Checkers* game);
struct Moves* checkersGetAvailableMovesForPlayer(struct Checkers* game, size_t* out_size);
void checkersDestroyMovesList(struct Moves* moves, size_t moves_size);
void checkersPrint(struct Checkers* game);

#endif /* CHECKERS_BOARD_H */