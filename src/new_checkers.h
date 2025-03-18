#ifndef CHECKERS_IMPROVED_H
#define CHECKERS_IMPROVED_H

#define CHECKERS_PLAYER_ONE          0
#define CHECKERS_PLAYER_TWO          1

/* so far, these can be safely inverted if so desired */
#define CHECKERS_PLAYER_LIGHT        CHECKERS_PLAYER_ONE
#define CHECKERS_PLAYER_DARK         CHECKERS_PLAYER_TWO

/* so far, may be changed to 8 without running into issues */
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
/**
 * This tells the lines moved vertically, l >= 1 for up, l <= -1 for down,
 * l == 0 for same horizontal line
 */
#define V_DIRECTION(from, to)       ((int) roundf((float) ((from) - (to)) / (float) CHECKERS_PIECES_PER_LINE))
/**
 * Misleading name(?)
 * This tells the general direction of a movement.
 * The values depend on the size of the board.
 * Only valid when the equivalent points of {from} and {to}
 * lead to a vector such that |x| = |y|
 */
#define H_DIRECTION(from, to)       ((from) - (to))
#define IS_EVEN_LINE(pos)           (((pos) / CHECKERS_PIECES_PER_LINE) % 2 == 0)
#define IS_ODD_LINE(pos)            (((pos) / CHECKERS_PIECES_PER_LINE) % 2 == 1)
/* vertical lines moved (absolute value) */
#define LINES_MOVED(from, to)       (abs(V_DIRECTION((from), (to))))

/**
 * POINT_TO_IDX should be used with valid points only.
 * A valid point is one such that (x + y) % 2 == 1
 */
#define POINT_TO_IDX(x,y)           ((y) * CHECKERS_PIECES_PER_LINE + (x) / 2)
/**
 * Any index should lead to a valid point, but that doesn't mean
 * it will be within the ranges of the board
 */
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


/**
 * Will allocate the internal representation of the board on the heap.
 * Calls to this function should be eventually followed by a call to
 * boardFreeInternalBoard().
 */
int boardInit(Board* gameboard);
/**
 * Reinitializes the Board's state and everything else.
 * The internal representation of the board is freed before reinitialization.
 * After usage, don't forget to call boardFreeInternalBoard() (only once even after many reinits).
 */
int boardReinit(Board* gameboard);
/**
 * Frees the internal representation of the board and zeroes out the
 * {gameboard} with a call to memset;
 */
void boardFreeInternalBoard(Board* gameboard);
/**
 * This function will try to make a move from a given position to another position.
 * The movement may lead to a capture or not. What matters is that the
 * movement shall be valid and that the piece must be one of the player's
 * actual pieces.
 * 
 * On success, positive values are returned.
 * On failure, negative values are returned.
 * 
 * Each return value has a meaning. It is suggested to check them on return.
 */
int boardTryMoveOrCapture(Board* gameboard, int player, int from, int to);
int boardRemainingPiecesTotal(const Board* gameboard);
int boardRemainingPiecesPlayer(const Board* gameboard, int player);
/**
 * Should be called at the end of a player's turn. This is used to remove from
 * the board the pieces captured during a turn and to turn pieces
 * into kings.
 */
int boardUpdate(Board* gameboard);

Piece boardGetPieceAt(const Board* gameboard, int pos);
Piece boardGetPieceAtP(const Board* gameboard, Point pos);
/* the out buffer is malloc'd and should be freed after usage */
size_t boardGetAvailableMovesForPiece(const Board* gameboard, int piecePos, int** out);
/**
 * the Moves* pointer is malloc'd and should be freed
 * through the boardDestroyMovesList() function after usage.
 */
Moves* boardGetAvailableMovesForPlayer(const Board* gameboard, int player, size_t* out_size);
/* the out buffer is malloc'd and should be freed after usage */
size_t boardGetLongestCaptureStreakForPiece(const Board* gameboard, int piecePos, int** out);
/* the out buffer is malloc'd and should be freed after usage */
size_t boardGetLongestCaptureStreakForPlayer(const Board* gameboard, int player, int** out);
/* frees moves list and all of its internal lists */
void boardDestroyMovesList(Moves* moves, size_t moves_size);

int boardCheckIfPieceHasAvailableMoves(const Board* gameboard, int pos);
int boardCheckIfPlayerHasAvailableMoves(const Board* gameboard, int player);
int boardCheckIfPieceCanCapture(const Board* gameboard, int pos);
int boardCheckIfPlayerCanCapture(const Board* gameboard, int player);
/* prints the board on the terminal */
void boardPrint(const Board* gameboard);

// ------------------------------------------------------------

/**
 * The internal board will undergo a heap allocation, so every call to this
 * function should be eventually followed by a call to checkersFreeBoard().
 */
int checkersInit(Checkers* game, int forceCapture, int autocapture, int enableAi, int externalCaptureHandling);
/**
 * Reinitializes the whole game, including the internal board.
 * This function may be called whenever it is needed to restart the game.
 * Only one of all reinit calls should be followed by a checkersFreeBoard() call.
 */
int checkersReinit(Checkers* gameboard);
/**
 * Frees the internal board's internal representation of the board and anything
 * else that might've been allocated during the game.
 * 
 * This will also zero out the {game} with a call to memset.
 */
int checkersFreeBoard(Checkers* game);
/** 
 * Tries to make a move for the current player, be it a capture or not.
 * 
 * Check the boardTryMoveOrCapture() function for more details - especially on
 * return values, as the behavior of this function in that regard
 * is exactly the same.
 */
int checkersMakeMove(Checkers* game, int from, int to);
int checkersMakeMoveP(Checkers* game, Point from, Point to);
/**
 * Ends a turn. This will call the boardUpdate function on the internal board.
 * 
 * This function will be automatically called after a call to
 * checkersMakeMove(), but it doesn't always finish a turn, as that depends on
 * some circumstances.
 * When it _doesn't_ end a turn, the user may choose to set the
 * needsUpdate flag and then call this function to prematurely end a turn.
 * 
 * This function is allowed to be called whenever the user wants (and so is
 * the needsUpdate flag allowed to be set), but that may ruin the flow of
 * the game.
 */
void checkersEndTurn(Checkers* game);
int checkersGetCurrentPlayer(const Checkers* game);
int checkersGetCurrentEnemy(const Checkers* game);
/**
 * Returns -1 on error or if the game is still ongoing.
 * Return values should be checked agains the CHECKERS_PLAYER
 * macros.
 */
int checkersGetWinner(const Checkers* game);

int checkersFlagIsRunning(const Checkers* game);
int checkersFlagIsCurrentlyCapturing(const Checkers* game);
int checkersFlagIsForceCaptureOn(const Checkers* game);
int checkersFlagIsAutoCapturesOn(const Checkers* game);
int checkersFlagIsAIEnabled(const Checkers* game);
int checkersFlagIsExternalCaptureHandleEnabled(const Checkers* game);

/* The return value corresponds to whether the flag is now on or off. */
int checkersFlagSetForceCapture(Checkers* game, int on);
/* The return value corresponds to whether the flag is now on or off. */
int checkersFlagSetAutoCaptures(Checkers* game, int on);
/* The return value corresponds to whether the flag is now on or off. */
int checkersFlagSetAI(Checkers* game, int on);
/** 
 * Will always set the needsupdate flag to some value greater than zero.
 * The return value corresponds to whether it is now on or off.
 */
int checkersFlagSetNeedsUpdate(Checkers* game);

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
 * Just frees the internal capture streak list and sets it to NULL.
 * 
 * returns 0 if nothing needs to be done
 * returns 1 if the list has been freed and -1 on error (which also means that
 * nothing has been dealocated, so it might be necessary to check the
 * internal list manually. If it's not set to NULL, it hasn't been freed.)
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
/* checks if current player has an available move (that may or may not lead to a capture) */
int checkersPlayerCanMove(const Checkers* game);
int checkersPlayerMustCapture(const Checkers* game);
/**
 * Pretty-prints the board on the terminal.
 * Includes details about the game such as current player,
 * turns played and amount of pieces remaining per player.
 */
void checkersPrint(const Checkers* game);

#endif /* CHECKERS_IMPROVED_H */