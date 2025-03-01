#ifndef CHECKERS_IMPROVED_H
#define CHECKERS_IMPROVED_H

#define CHECKERS_PLAYER_ONE          0
#define CHECKERS_PLAYER_TWO          1

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
#define CHECKERS_END_GAME           -8
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
#define IDX_TO_POINT(idx)                                                               \
(                                                                                       \
    (struct Point) {                                                                    \
        .x = IS_EVEN_LINE((idx)) ?                                                      \
                ((idx) % CHECKERS_PIECES_PER_LINE + 1) * 2 - 1 :                        \
                    ((idx) % CHECKERS_PIECES_PER_LINE) * 2,                             \
        .y = (idx) / CHECKERS_PIECES_PER_LINE                                           \
    }                                                                                   \
)

#include <stdint.h>
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
    } flags;
} Checkers;

typedef struct Moves {
    int from;
    int* to;
    size_t to_size;
} Moves;



int boardInit(Board* gameboard);
int boardTryMoveOrCapture(Board* gameboard, int player, int from, int to);
int boardRemainingPiecesTotal(const Board* gameboard);
int boardRemainingPiecesPlayer(const Board* gameboard, int player);
int boardUpdate(Board* gameboard); /* called at the end of a player's turn. used to remove from the board the pieces captured during a turn and to turn pieces into kings */

struct Piece boardGetPieceAt(const Board* gameboard, int pos);
struct Piece boardGetPieceAtP(const Board* gameboard, Point pos);
size_t boardGetAvailableMovesForPiece(const Board* gameboard, int piecePos, int** out);
Moves* boardGetAvailableMovesForPlayer(const Board* gameboard, int player, size_t* out_size);
size_t boardGetLongestCaptureStreakForPiece(const Board* gameboard, int piecePos, int** out);
size_t boardGetLongestCaptureStreakForPlayer(const Board* gameboard, int player, int** out);
void boardDestroyMovesList(struct Moves* moves, size_t moves_size);

int boardCheckIfPieceHasAvailableMoves(const Board* gameboard, int pos);
int boardCheckIfPlayerHasAvailableMoves(const Board* gameboard, int player);
int boardCheckIfPieceCanCapture(const Board* gameboard, int pos);
int boardCheckIfPlayerCanCapture(const Board* gameboard, int player);
void boardPrint(const Board* gameboard);
void boardFreeInternalBoard(Board* gameboard);

// ------------------------------------------------------------

// TODO - finish this
int checkersInit(Checkers* game, int forceCapture, int autocapture, int enableAi);
int checkersFreeBoard(Checkers* game);
int checkersMakeMove(Checkers* game, int from, int to);
int checkersMakeMoveP(Checkers* game, Point from, Point to);
void checkersEndTurn(Checkers* game);
int checkersGetCurrentPlayer(const Checkers* game);
int checkersGetCurrentEnemy(const Checkers* game);
int checkersGetWinner(const Checkers* game);
int checkersIsRunning(const Checkers* game);

Moves* checkersGetAvailableMovesForPlayer(const Checkers* game, size_t* out_size);
size_t checkersGetLongestCaptureStreakForPlayer(const Checkers* game, int** out);

int checkersPlayerCanMove(const Checkers* game);
int checkersPlayerMustCapture(const Checkers* game);
void checkersPrint(const Checkers* game);

#endif /* CHECKERS_IMPROVED_H */