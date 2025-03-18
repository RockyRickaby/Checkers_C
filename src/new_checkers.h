#ifndef CHECKERS_IMPROVED_H
#define CHECKERS_IMPROVED_H

#define CHECKERS_PLAYER_ONE          0
#define CHECKERS_PLAYER_TWO          1

/* so far, these can be safely inverted if so desired */
#define CHECKERS_PLAYER_LIGHT        CHECKERS_PLAYER_ONE
#define CHECKERS_PLAYER_DARK         CHECKERS_PLAYER_TWO

#define CHECKERS_BOARD_SIZE         10
#define CHECKERS_PIECES_PER_LINE    (CHECKERS_BOARD_SIZE / 2)
#define CHECKERS_PIECES_AMOUNT      (CHECKERS_PIECES_PER_LINE * ((CHECKERS_BOARD_SIZE - 2) / 2))
#define CHECKERS_FIELDS_AMOUNT      (CHECKERS_PIECES_AMOUNT * 2 + CHECKERS_PIECES_PER_LINE * 2)

#define CHECKERS_CAPTURE_SUCCESS     2
#define CHECKERS_MOVE_SUCCESS        1
#define CHECKERS_NULL_BOARD         -1
#define CHECKERS_MOVE_FAIL          -2
#define CHECKERS_INVALID_MOVE       -3
#define CHECKERS_INVALID_PLAYER     -4
#define CHECKERS_NOT_A_PIECE        -5
#define CHECKERS_DEAD_PIECE         -6
#define CHECKERS_ONGOING_TURN       -7
#define CHECKERS_NOT_LONGEST_PATH   -8
#define CHECKERS_END_GAME           -9
// =======================================================================
#define V_DIRECTION(from, to)       ((int) roundf((float) ((from) - (to)) / (float) CHECKERS_PIECES_PER_LINE))
#define H_DIRECTION(from, to)       ((from) - (to))
#define IS_EVEN_LINE(pos)           (((pos) / CHECKERS_PIECES_PER_LINE) % 2 == 0)
#define IS_ODD_LINE(pos)            (((pos) / CHECKERS_PIECES_PER_LINE) % 2 == 1)
#define LINES_MOVED(from, to)       (abs(V_DIRECTION((from), (to))))

/**
 * POINT_TO_IDX should be used with valid points only.
 * A valid point is one such that (x + y) % 2 == 1
 */
#define POINT_TO_IDX(x,y)           ((y) * CHECKERS_PIECES_PER_LINE + (x) / 2)
#define IDX_TO_POINT(idx)\
(\
    (struct Point) {\
        .x = IS_EVEN_LINE((idx)) ?\
                ((idx) % CHECKERS_PIECES_PER_LINE + 1) * 2 - 1 :\
                    ((idx) % CHECKERS_PIECES_PER_LINE) * 2,\
        .y = (idx) / CHECKERS_PIECES_PER_LINE\
    }\
)

#include <stdint.h>
#include <stddef.h>
#include <math.h>

typedef struct Point { int x, y; } Point;

typedef enum PieceType {
    PIECE_EMPTY_FIELD,
    PIECE_LIGHT_KING,
    PIECE_LIGHT_MAN,
    PIECE_DARK_KING,
    PIECE_DARK_MAN,
} PieceType;

typedef enum GameState {
    CSTATE_P1_TURN,
    CSTATE_P1_TURN_WAITING,
    CSTATE_P1_TURN_CAPTURING,

    CSTATE_P2_TURN,
    CSTATE_P2_TURN_WAITING,
    CSTATE_P2_TURN_CAPTURING,

    CSTATE_END_P1_WIN,
    CSTATE_END_P2_WIN,
    CSTATE_END_DRAW
} GameState;

typedef struct Piece {
    PieceType type;
    uint8_t alive;
    uint8_t recentlyMoved;
} Piece;

typedef struct Board {
    Piece* board;
    size_t boardSize;
    int16_t recentlyMovedPiece; /* default value after end of turn = -1 */

    uint8_t remainingLightMen;
    uint8_t remainingDarkMen;
    uint8_t remainingLightKings;
    uint8_t remainingDarkKings;
} Board;

typedef struct Checkers {
    int turnsTotal;
    GameState state;
    Board checkersBoard;
    struct { /* should this be a single uint8? it has its dangers... */
        /* internal use */
        uint8_t run;
        uint8_t currentlyCapturing;
        uint8_t needsUpdate;
        uint8_t forceCapture;

        /* external use */
        uint8_t autoCapture;
        uint8_t aiEnabled; /* does nothing for now... */
        uint8_t externalCaptureHandling;
    } flags;

    /* extra stuff for handling longest capture sequences */
    int* captures;
    size_t capturesSize;
    size_t capturesIdx;
} Checkers;

typedef struct Moves {
    int from;
    int* to;
    size_t to_size;
} Moves;



int boardInit(Board* gameboard);
int boardReinit(Board* gameboard);
void boardFreeInternalBoard(Board* gameboard);
int boardTryMoveOrCapture(Board* gameboard, int player, int from, int to);
int boardRemainingPiecesTotal(const Board* gameboard);
int boardRemainingPiecesPlayer(const Board* gameboard, int player);
int boardUpdate(Board* gameboard); /* called at the end of a player's turn. used to remove from the board the pieces captured during a turn and to turn pieces into kings */

Piece boardGetPieceAt(const Board* gameboard, int pos);
Piece boardGetPieceAtP(const Board* gameboard, Point pos);
size_t boardGetAvailableMovesForPiece(const Board* gameboard, int piecePos, int** out);
Moves* boardGetAvailableMovesForPlayer(const Board* gameboard, int player, size_t* out_size);
size_t boardGetLongestCaptureStreakForPiece(const Board* gameboard, int piecePos, int** out);
size_t boardGetLongestCaptureStreakForPlayer(const Board* gameboard, int player, int** out);
void boardDestroyMovesList(Moves* moves, size_t moves_size);

int boardCheckIfPieceHasAvailableMoves(const Board* gameboard, int pos);
int boardCheckIfPlayerHasAvailableMoves(const Board* gameboard, int player);
int boardCheckIfPieceCanCapture(const Board* gameboard, int pos);
int boardCheckIfPlayerCanCapture(const Board* gameboard, int player);
void boardPrint(const Board* gameboard);

// ------------------------------------------------------------

int checkersInit(Checkers* game, int forceCapture, int autocapture, int enableAi, int externalCaptureHandling);
int checkersReinit(Checkers* gameboard);
int checkersFreeBoard(Checkers* game);
int checkersMakeMove(Checkers* game, int from, int to);
int checkersMakeMoveP(Checkers* game, Point from, Point to);
void checkersEndTurn(Checkers* game);
int checkersGetCurrentPlayer(const Checkers* game);
int checkersGetCurrentEnemy(const Checkers* game);
int checkersGetWinner(const Checkers* game);

int checkersFlagIsRunning(const Checkers* game);
int checkersFlagIsCurrentlyCapturing(const Checkers* game);
int checkersFlagForceCaptureIsOn(const Checkers* game);
int checkersFlagAutoCapturesIsOn(const Checkers* game);
int checkersFlagIsAIEnabled(const Checkers* game);
int checkersFlagIsExternalCaptureHandleEnabled(const Checkers* game);

int checkersFlagForceCaptureSet(Checkers* game, int on);
int checkersFlagAutoCapturesSet(Checkers* game, int on);
int checkersFlagAISet(Checkers* game, int on);

int checkersFlagNeedsUpdateSet(Checkers* game);

Moves* checkersGetAvailableMovesForPlayer(const Checkers* game, size_t* out_size);
size_t checkersGetLongestCaptureStreakForPlayer(const Checkers* game, int** out);
/**
 * Nothing is allocated if captures are not mandatory.
 * Every call to this function should be (eventually)
 * followed by a call to checkersUnloadCaptureStreak() IFF something is allocated
 * 
 * returns 0 when nothing is allocated
 * returns 1 when something is allocated and -1 on error (nothing is allocated on error)
 */
int checkersLoadCaptureStreak(Checkers* game);
/**
 * just frees the internal capture streak list
 * 
 * returns 0 if nothing needs to be done
 * returns 1 if the list has been freed and -1 on error
 */
int checkersUnloadCaptureStreak(Checkers* game);

/**
 * Works kind of like an iterator. Call it and it returns the
 * next movement in the list and it will increment an index
 * (the first movement is always the origin. Use that to
 * check if the piece chosen by the player is the "right"
 * one)
 * 
 * A return value of -1 means that there are no more moves to check.
 * Any return value less than -1 means error.
 */
int checkersCaptureStreakNext(Checkers* game);
int checkersPlayerCanMove(const Checkers* game);
int checkersPlayerMustCapture(const Checkers* game);
void checkersPrint(const Checkers* game);

#endif /* CHECKERS_IMPROVED_H */