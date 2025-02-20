#include "new_checkers.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <math.h>

#define BLK "\e[0;30m"
#define BWHT "\e[1;37m"
#define CRESET "\e[0m"

static const Piece emptyField = {
    .alive = 0,
    .type = PIECE_EMPTY_FIELD,
    .enemies.man = PIECE_NONE,
    .enemies.king = PIECE_NONE,
    .recentlyMoved = 0
};

static const char pieceChars[] = {
    [PIECE_EMPTY_FIELD] = '.',
    [PIECE_LIGHT_KING] = 'K',
    [PIECE_LIGHT_MAN] = 'M',
    [PIECE_DARK_KING] = 'k',
    [PIECE_DARK_MAN] = 'm',
};

static const int movementOffsets[] = {
    -(CHECKERS_PIECES_PER_LINE - 1),
    -(CHECKERS_PIECES_PER_LINE),
    (CHECKERS_PIECES_PER_LINE),
    (CHECKERS_PIECES_PER_LINE + 1)
};

static inline int validIndex(int x, int y) { return x >= 0 && x < CHECKERS_BOARD_SIZE && y >= 0 && y < CHECKERS_BOARD_SIZE; }
static inline int validField(int x, int y) { return (x + y) % 2 == 1; }
static inline int validPos(int pos) { return pos >= 0 && pos < (CHECKERS_FIELDS_AMOUNT); }
static inline int validPlayer(int player) { return player == CHECKERS_PLAYER_ONE || player == CHECKERS_PLAYER_TWO; }
static inline int validOff(int off) { return movementOffsets[0] == off || movementOffsets[1] == off || movementOffsets[2] == off || movementOffsets[3] == off; }

static inline int isDark(Piece* p) { return p->type == PIECE_DARK_MAN || p->type == PIECE_DARK_KING; }
static inline int isLight(Piece* p) { return p->type == PIECE_LIGHT_MAN || p->type == PIECE_LIGHT_KING; }
static inline int isMan(Piece* p) { return p->type == PIECE_LIGHT_MAN || p->type == PIECE_DARK_MAN; }
static inline int isKing(Piece* p) { return p->type == PIECE_LIGHT_KING || p->type == PIECE_DARK_KING; }
static inline int isEmptyField(Piece* p) { return p->type == PIECE_EMPTY_FIELD; }
static inline int isPiece(Piece* p) { return isDark(p) || isLight(p); }
static inline int isEnemyOf(Piece* p1, Piece* p2) { return (p1->type == p2->enemies.man || p1->type == p2->enemies.king) && (p2->type == p1->enemies.man || p2->type == p1->enemies.king); }
static inline int isFriend(Piece* p1, Piece* p2) { return (isLight(p1) && isLight(p2)) || (isDark(p1) && isDark(p2)); }

static int canCapture(Board* gameboard, int from);
static int fromVecToOffset(Point vec, int from);
static int movePiece(Board* gameboard, int player, int from, int to);
static int validMove(Board* gameboard, int player, int from, int to);

int boardInit(Board* gameboard) {
    if (!gameboard) {
        return 0;
    }
    const int bsize = CHECKERS_FIELDS_AMOUNT;
    const int pamount = CHECKERS_PIECES_AMOUNT;

    memset(gameboard, 0, sizeof(Board));
    gameboard->remainingDarkPieces = pamount;
    gameboard->remainingLightPieces = pamount;
    gameboard->boardSize = bsize;
    gameboard->recentlyMovedPiece = -1;
    
    Piece darkman = {
        .alive = 1,
        .type = PIECE_DARK_MAN,
        .enemies.man = PIECE_LIGHT_MAN,
        .enemies.king = PIECE_LIGHT_KING,
        .recentlyMoved = 0,
    };
    Piece lightman = {
        .alive = 1,
        .type = PIECE_LIGHT_MAN,
        .enemies.man = PIECE_DARK_MAN,
        .enemies.king = PIECE_DARK_KING,
        .recentlyMoved = 0,
    };
    
    Piece* board = calloc(bsize, sizeof(Piece));
    if (board == NULL) {
        memset(gameboard, 0, sizeof(Board));
        return 0;
    }
    const int offset = CHECKERS_PIECES_AMOUNT + CHECKERS_BOARD_SIZE;
    for (int i = 0; i < pamount; i++) {
        board[i] = darkman;
        board[offset + i] = lightman;
    }
    const int emptyOffset = pamount;
    for (int i = 0; i < CHECKERS_BOARD_SIZE; i++)  {
        board[emptyOffset + i] = emptyField;
    }
    gameboard->board = board;
    return 1;
}

int boardTryMoveOrCapture(Board* gameboard, int player, int from, int to) {
    if (!gameboard) {
        return CHECKERS_NULL_BOARD;
    }
    if (!validPlayer(player)) {
        return CHECKERS_INVALID_PLAYER;
    }
    if (!validPos(from) || !validPos(to)) {
        return CHECKERS_INVALID_MOVE;
    }

    Piece* p = gameboard->board + from;
    Piece* p2 = gameboard->board + to;
    if (player == CHECKERS_PLAYER_LIGHT && !isLight(p)) {
        return CHECKERS_NOT_A_PIECE;
    }
    if (player == CHECKERS_PLAYER_DARK && !isDark(p)) {
        return CHECKERS_NOT_A_PIECE;
    }
    if (!p->alive) {
        return CHECKERS_DEAD_PIECE;
    }
    if (!isEmptyField(p2) || !validMove(gameboard, player, from, to)) {
        return CHECKERS_INVALID_MOVE;
    }
    return movePiece(gameboard, player, from, to);
}

int boardTryTurnKing(Board* gameboard, int piecePos) {
    if (gameboard && validPos(piecePos)) {
        Piece* p = gameboard->board + piecePos;
        if (p->alive) {
            if (p->type == PIECE_LIGHT_MAN && piecePos < CHECKERS_PIECES_PER_LINE) {
                p->type == PIECE_LIGHT_KING;
                return 1;
            } else if (p->type == PIECE_DARK_MAN && piecePos >= (CHECKERS_PIECES_PER_LINE * (CHECKERS_BOARD_SIZE - 1))) {
                p->type == PIECE_DARK_KING;
                return 1;
            } else {
                return 0;
            }
        } else {
            return 0;
        }
    }
    return 0;
}

int boardRemainingPiecesTotal(const Board* gameboard) {
    if (!gameboard) {
        return -1;
    }
    return gameboard->remainingDarkPieces + gameboard->remainingLightPieces;
}

int boardRemainingPiecesPlayer(const Board* gameboard, int player) {
    if (!gameboard || !validPlayer(player)) {
        return -1;
    }
    if (player == CHECKERS_PLAYER_LIGHT) {
        return gameboard->remainingLightPieces;
    } else {
        return gameboard->remainingDarkPieces;
    }
}

int boardUpdate(Board* gameboard) {
    if (gameboard) {
        for (int i = 0; i < gameboard->boardSize; i++) {
            Piece* p = gameboard->board + i;
            if (p->type != PIECE_EMPTY_FIELD && !p->alive) {
                if (isLight(p)) {
                    gameboard->remainingLightPieces -= 1;
                } else if (isDark(p)) {
                    gameboard->remainingDarkPieces -= 1;
                }
                *p = emptyField;
            }
        }
        gameboard->recentlyMovedPiece = -1; 
        return 1;
    }
    return 0;
}

struct Piece boardGetPieceAt(const Board* gameboard, int pos) {
    if (!gameboard || !validPos(pos)) {
        return emptyField;
    }
    return gameboard->board[pos];
}

struct Piece boardGetPieceAtP(const Board* gameboard, Point piecePos) {
    if (!gameboard || !validIndex(piecePos.x, piecePos.y) || !validField(piecePos.x, piecePos.y)) {
        return emptyField;
    }
    return gameboard->board[POINT_TO_IDX(piecePos.x, piecePos.y)];
}

int boardGetAvailableMovesForPiece(const Board* gameboard, int piecePos, int** out);
Moves* boardGetAvailableMovesForPlayer(const Board* gameboard, int player, size_t* out_size);
int boardGetLongestCaptureStreakForPiece(const Board* gameboard, int piecePos, int** out);
Moves* boardGetLongestCaptureStreakForPlayer(const Board* gameboard, int player, size_t* out_size);

int boardCheckIfPieceHasAvailableMoves(const Board* gameboard, int player, int pos);
int boardCheckIfPlayerHasAvailableMoves(const Board* gameboard, int player);
int boardCheckIfPieceCanCapture(const Board* gameboard, int player, int pos);
int boardCheckIfPlayerCanCapture(const Board* gameboard, int player);

void boardPrint(const Board* gameboard) {
    if (gameboard) {
        for (int i = 0; i < CHECKERS_BOARD_SIZE; i++) {
            for (int j = 0; j < CHECKERS_BOARD_SIZE; j++) {
                if (validField(j, i) && isPiece(gameboard->board + POINT_TO_IDX(j, i))) {
                    printf("%c ", pieceChars[gameboard->board[POINT_TO_IDX(j, i)].type]);
                } else {
                    printf("%c ", pieceChars[PIECE_EMPTY_FIELD]);
                }
            }
            printf("\n");
        }
    }
}

void boardFreeInternalBoard(Board* gameboard) {
    if (gameboard) {
        free(gameboard->board);
        gameboard->board = NULL; /* prevent use after free */
        memset(gameboard, 0, sizeof(Board));
    }
}

/**
 * 
 * 
 * 
 */

static int canCapture(Board* gameboard, int from) {
    return 0; // TODO - implement this
}

 /**
  * If we can recreate the {to} value from the starting position {from}
  * plus a few offsets, as long as the movement can be done without
  * obstructions or leads to a valid capture, it is valid.
  */
static int validMove(Board* gameboard, int player, int from, int to) {
    Point playerPos = IDX_TO_POINT(from);
    Point toPos = IDX_TO_POINT(to);
    Point direction = {
        .x = toPos.x - playerPos.x,
        .y = toPos.y - playerPos.y,
    };
    Point vec = {
        .x = direction.x < 0 ? -1 : 1,  /* -1, cima; 1, baixo */
        .y = direction.y < 0 ? -1 : 1,  
    };
    if (abs(direction.x) != abs(direction.y)) {
        return 0;
    }

    int illegalMoveFlag = player == CHECKERS_PLAYER_LIGHT ? -1 : 1; /* -1, baixo; 1, cima */
    int vdir = V_DIRECTION(from, to);
    int hdir = H_DIRECTION(from, to);
    unsigned int lines = abs(vdir);

    Piece* playerPiece = gameboard->board + from;

    if (isKing(playerPiece)) {
        /**
         * iterate over positions and check if the king can properly move
         * (and maybe capture ONE alive enemy)
         */
        int canMakeMove = 1;
        int off = 0, enemyCount = 0;
        Piece* p = NULL;
        while (from != to) {
            off = fromVecToOffset(vec, from);
            from += off;

            p = gameboard->board + from;
            if (isEnemyOf(playerPiece, p)) {
                if (p->alive) {
                    enemyCount++;
                    if (enemyCount > 1) {
                        canMakeMove = 0;
                        from = to;
                    }
                } else {
                    canMakeMove = 0;
                    from = to;
                }
            } else if (isFriend(playerPiece, p)) {
                canMakeMove = 0;
                from = to;
            }
        }
        return canMakeMove;
    } else if (isMan(playerPiece)) {
        if (lines == 1) {
            return illegalMoveFlag != vdir;
        } else if (lines == 2) { /* should be capture only */
            playerPos.x += vec.x;
            playerPos.y += vec.y;
            int idx = POINT_TO_IDX(playerPos.x, playerPos.y);
            return isEnemyOf(playerPiece, gameboard->board + idx) &&
                gameboard->board[idx].alive;
        } else {
            return 0;
        }
    } else {
        return 0;
    }
}

/* movement/capture should be guaranteed to happen before this function is called  */
static int movePiece(Board* gameboard, int player, int from, int to) {
    Point playerPos = IDX_TO_POINT(from);
    Point toPos = IDX_TO_POINT(to);
    Point direction = {
        .x = toPos.x - playerPos.x,
        .y = toPos.y - playerPos.y,
    };
    Point vec = {
        .x = direction.x < 0 ? -1 : 1,  /* -1, cima; 1, baixo */
        .y = direction.y < 0 ? -1 : 1,  
    };
    unsigned int lines = LINES_MOVED(from, to);
    Piece* playerPiece = gameboard->board + from;
    Piece* targetField = gameboard->board + to;
    Piece playerBackup = *playerPiece;

    /* all movement is assumed to be valid */
    if (isKing(playerPiece)) {
        int off = 0, enemyCount = 0;
        Piece* p = NULL;
        while (from != to) {
            off = fromVecToOffset(vec, from);
            from += off;

            /* find enemy, capture and jump to destination */
            p = gameboard->board + from;
            if (isEnemyOf(playerPiece, p)) {
                p->alive = 0;
                from = to;
            }
        }
        *targetField = *playerPiece;
        *playerPiece = emptyField;
        return CHECKERS_MOVE_SUCCESS;
    } else if (isMan(playerPiece)) {
        if (lines == 2) {
            playerPos.x += vec.x;
            playerPos.y += vec.y;
            gameboard->board[POINT_TO_IDX(playerPos.x, playerPos.y)].alive = 0;
            *targetField = *playerPiece;
            *playerPiece = emptyField;
            return CHECKERS_CAPTURE_SUCCESS;
        } else {
            *targetField = *playerPiece;
            *playerPiece = emptyField;
            return CHECKERS_MOVE_SUCCESS;
        }
    } else {
        return CHECKERS_NOT_A_PIECE;
    }
}

/* -1, up; 1, down */
static int fromVecToOffset(Point vec, int from) {
    if (abs(vec.x) > 1 || abs(vec.y) > 1) {
        vec.x = vec.x < 0 ? -1 : 1;
        vec.y = vec.y < 0 ? -1 : 1;
    }
    if (IS_EVEN_LINE(from)) {
        if (vec.x == 1 && vec.y == 1) {         /* down, right */
            return movementOffsets[3];
        } else if (vec.x == -1 && vec.y == 1) { /* down, left */
            return movementOffsets[2];
        } else if (vec.x == 1 && vec.y == -1) { /* up, right */
            return movementOffsets[0];
        } else {                                /* up, left */
            return movementOffsets[1] ;
        }
    } else {
        if (vec.x == 1 && vec.y == 1) {         /* down, right */
            return -movementOffsets[1];
        } else if (vec.x == -1 && vec.y == 1) { /* down, left */
            return -movementOffsets[0];
        } else if (vec.x == 1 && vec.y == -1) { /* up, right */
            return -movementOffsets[2];
        } else {                                /* up, left */
            return -movementOffsets[3];
        }
    }
}