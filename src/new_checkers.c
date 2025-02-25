#include "new_checkers.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <math.h>

#define BLK "\e[0;30m"
#define BWHT "\e[1;37m"
#define CRESET "\e[0m"

typedef struct Node {
    struct Node* next;
    int value;
} Node;

typedef struct CaptureStack {
    Node* top;
    size_t size;
} CaptureStack;

static const Piece emptyField = {
    .alive = 0,
    .type = PIECE_EMPTY_FIELD,
    .recentlyMoved = 0
};

/* may have some holes if the enum changes */
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
static inline int validField(int x, int y) { return validIndex(x, y) && (x + y) % 2 == 1; }
static inline int validPos(int pos) { return pos >= 0 && pos < (CHECKERS_FIELDS_AMOUNT); }
static inline int validPlayer(int player) { return player == CHECKERS_PLAYER_ONE || player == CHECKERS_PLAYER_TWO; }
static inline int validOff(int off) { return movementOffsets[0] == off || movementOffsets[1] == off || movementOffsets[2] == off || movementOffsets[3] == off; }

static inline int isDark(const Piece* p) { return p->type == PIECE_DARK_MAN || p->type == PIECE_DARK_KING; }
static inline int isLight(const Piece* p) { return p->type == PIECE_LIGHT_MAN || p->type == PIECE_LIGHT_KING; }
static inline int isMan(const Piece* p) { return p->type == PIECE_LIGHT_MAN || p->type == PIECE_DARK_MAN; }
static inline int isKing(const Piece* p) { return p->type == PIECE_LIGHT_KING || p->type == PIECE_DARK_KING; }
static inline int isEmptyField(const Piece* p) { return p->type == PIECE_EMPTY_FIELD; }
static inline int isPiece(const Piece* p) { return isDark(p) || isLight(p); }
static inline int isEnemy(const Piece* p1, const Piece* p2) { return (isLight(p1) && isDark(p2)) || (isLight(p2) && isDark(p1)); }
static inline int isFriend(const Piece* p1, const Piece* p2) { return (isLight(p1) && isLight(p2)) || (isDark(p1) && isDark(p2)); }
static int fromVecToOffset(Point vec, int from);

static inline int deepcopy(const Board* src, Board* dst);
static inline int canCapture(const Board* gameboard, int from, int to, Point toP, Point vec);
static int movePiece(Board* gameboard, int from, int to);
static int validMove(Board* gameboard, int player, int from, int to);

int boardInit(Board* gameboard) {
    if (!gameboard) {
        return 0;
    }
    const int bsize = CHECKERS_FIELDS_AMOUNT;
    const int pamount = CHECKERS_PIECES_AMOUNT;

    memset(gameboard, 0, sizeof(Board));
    gameboard->remainingDarkMen = pamount;
    gameboard->remainingLightMen = pamount;
    gameboard->remainingDarkKings = 0;
    gameboard->remainingLightKings = 0;
    gameboard->boardSize = bsize;
    gameboard->recentlyMovedPiece = -1;
    
    Piece darkman = {
        .alive = 1,
        .type = PIECE_DARK_MAN,
        .recentlyMoved = 0,
    };
    Piece lightman = {
        .alive = 1,
        .type = PIECE_LIGHT_MAN,
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

/* only the same piece can be moved per turn */
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
    if (gameboard->recentlyMovedPiece >= 0 && gameboard->recentlyMovedPiece != from) {
        return CHECKERS_ONGOING_TURN;
    }
    return movePiece(gameboard, from, to);
}

/* includes dead pieces that haven't been removed from the board yet */
int boardRemainingPiecesTotal(const Board* gameboard) {
    if (!gameboard) {
        return -1;
    }
    return gameboard->remainingDarkMen + gameboard->remainingLightMen +
        gameboard->remainingDarkKings + gameboard->remainingLightKings;
}

/* also includes dead pieces that haven't been removed from the board yet */
int boardRemainingPiecesPlayer(const Board* gameboard, int player) {
    if (!gameboard || !validPlayer(player)) {
        return -1;
    }
    if (player == CHECKERS_PLAYER_LIGHT) {
        return gameboard->remainingLightMen + gameboard->remainingLightKings;
    } else {
        return gameboard->remainingDarkMen + gameboard->remainingDarkKings;
    }
}

/* should always be called at the end of a turn */
int boardUpdate(Board* gameboard) {
    if (gameboard) {
        for (size_t i = 0; i < gameboard->boardSize; i++) {
            Piece* p = gameboard->board + i;
            if (isPiece(p) && !p->alive) {
                if (p->type == PIECE_LIGHT_MAN) {
                    gameboard->remainingLightMen -= 1;
                } else if (p->type == PIECE_DARK_MAN) {
                    gameboard->remainingDarkMen -= 1;
                } else if (p->type == PIECE_LIGHT_KING) {
                    gameboard->remainingLightKings -= 1;
                } else if (p->type == PIECE_DARK_KING) {
                    gameboard->remainingDarkKings -= 1;
                }
                *p = emptyField;
            }
        }
        Piece* p = gameboard->board + gameboard->recentlyMovedPiece;
        if (p->type == PIECE_LIGHT_MAN && gameboard->recentlyMovedPiece < CHECKERS_PIECES_PER_LINE) {
            p->type = PIECE_LIGHT_KING;
            gameboard->remainingLightKings += 1;
            gameboard->remainingLightMen -= 1;
        } else if (p->type == PIECE_DARK_MAN && gameboard->recentlyMovedPiece >= (CHECKERS_PIECES_PER_LINE * (CHECKERS_BOARD_SIZE - 1))) {
            p->type = PIECE_DARK_KING;
            gameboard->remainingDarkKings += 1;
            gameboard->remainingDarkMen -= 1;
        }
        gameboard->board[gameboard->recentlyMovedPiece].recentlyMoved = 0;
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
    if (!gameboard || !validField(piecePos.x, piecePos.y)) {
        return emptyField;
    }
    return gameboard->board[POINT_TO_IDX(piecePos.x, piecePos.y)];
}

/* won't be calculated if the piece is already dead */
size_t boardGetAvailableMovesForPiece(const Board* gameboard, int piecePos, int** out) {
    if (!gameboard || !validPos(piecePos) || !out) {
        if (out) {
            *out = NULL;
        }
        return 0;
    }
    Piece* p = gameboard->board + piecePos;
    if (!isPiece(p) || !p->alive) {
        *out = NULL;
        return 0;
    }
    int illegalY = 0; /* 1, up; -1, down*/
    if (isLight(p)) {
        illegalY = -1;
    } else {
        illegalY = 1;
    }
    size_t cap = 10;
    size_t size = 0;
    int* buf = malloc(sizeof(int) * cap);
    if (!buf) {
        *out = NULL;
        return 0;
    }

    if (isKing(p)) {
        int offsetsIdx = 0;
        int from = piecePos;
        Point fromP = IDX_TO_POINT(piecePos);
        int m = IS_EVEN_LINE(piecePos) ? 1 : -1;
        while (offsetsIdx < 4) {
            int to = 0;
            Point toP = {0};
            to = from + m * movementOffsets[offsetsIdx];
            toP = IDX_TO_POINT(to);
            Point vec = { /* to get the correct offsets for the direction in the inner loop */
                .x = toP.x - fromP.x < 0 ? -1 : 1,
                .y = toP.y - fromP.y < 0 ? -1 : 1,
            };
            to = from;
            toP = IDX_TO_POINT(to);
            int off = 0;
            int hasCaptured = 0;
            while (1) {
                off = fromVecToOffset(vec, to);
                to += off;
                toP.x += vec.x;
                toP.y += vec.y;
                if (!validPos(to) || !validIndex(toP.x, toP.y)) {
                    break;
                }
                Piece* p = gameboard->board + to;
                if (isEmptyField(p)) {
                    buf[size] = to;
                    size += 1;
                } else if (canCapture(gameboard, from, to, toP, vec)) {
                    if (hasCaptured) {
                        break;
                    }
                    hasCaptured = 1;
                    /* kings may be able to jump beyond this point, so let's jump to the field behind the enemy piece */
                    off = fromVecToOffset(vec, to);
                    to += off;
                    toP.x += vec.x;
                    toP.y += vec.y;
                    buf[size] = to;
                    size += 1;
                } else {
                    break;
                }

                if (size >= cap) {
                    cap *= 1.5f;
                    int* tmp = realloc(buf, sizeof(int) * cap);
                    if (!tmp) {
                        free(buf);
                        *out = NULL;
                        return 0;
                    }
                    buf = tmp;
                }
            }
            offsetsIdx += 1;
        }
    } else {
        int from = piecePos;
        int to = 0;
        int m = IS_EVEN_LINE(piecePos) ? 1 : -1;
        Point fromP = IDX_TO_POINT(from);
        for (int i = 0; i < 4; i++) {
            to = from + m * movementOffsets[i];
            Point toP = IDX_TO_POINT(to);
            /* y < 0, up; y > 0, down */
            Point dir = {
                .x = toP.x - fromP.x,
                .y = toP.y - fromP.y, 
            };
            if (validPos(to) && abs(dir.x) == abs(dir.y)) {
                if (canCapture(gameboard, from, to, toP, dir)) {
                    int jmp = POINT_TO_IDX(toP.x + dir.x, toP.y + dir.y);
                    buf[size] = jmp;
                    size++;
                } else if (isEmptyField(gameboard->board + to) && (-dir.y) * illegalY < 0) { /* just checking if signs are different */
                    buf[size] = to;
                    size++;
                }
            }
        }
    }
    
    if (size == 0) {
        free(buf);
        *out = NULL;
    } else {
        int* tmp = realloc(buf, sizeof(int) * size);
        if (!tmp) {
            free(buf);
            *out = NULL;
            return 0;
        }
        *out = tmp;
    }
    return size;
}

Moves* boardGetAvailableMovesForPlayer(const Board* gameboard, int player, size_t* out_size) {
    if (!gameboard || !validPlayer(player) || !out_size) {
        return NULL;
    }
    size_t size = 0;
    size_t cap = 10;
    Moves* moves = malloc(sizeof(Moves) * cap);
    if (!moves) {
        *out_size = 0;
        return NULL;
    }
    for (size_t i = 0; i < gameboard->boardSize; i++) {
        if ((player == CHECKERS_PLAYER_LIGHT && isLight(gameboard->board + i)) || (player == CHECKERS_PLAYER_DARK && isDark(gameboard->board + i))) {
            int* buf = NULL;
            size_t bufsize = boardGetAvailableMovesForPiece(gameboard, i, &buf);
            if (bufsize > 0) {
                moves[size] = (Moves) {
                    .from = i,
                    .to = buf,
                    .to_size = bufsize
                };
                size++;
                if (size >= cap) {
                    cap *= 1.5f;
                    Moves* tmp = realloc(moves, sizeof(Moves) * cap);
                    if (!tmp) {
                        *out_size = 0;
                        free(moves);
                        return NULL;
                    }
                    moves = tmp;
                }
            }
        }
    }
    if (size == 0) {
        free(moves);
        moves = NULL;
        *out_size = 0;
    } else {
        Moves* tmp = realloc(moves, sizeof(Moves) * size);
        if (!tmp) {
            free(moves);
            *out_size = 0;
            return NULL;
        }
        moves = tmp;
        *out_size = size;
    }
    return moves;
}

static int longestCaptureForMan(const Board* gameboard, int player, int piecePos, int** out, size_t* size, CaptureStack* stack);
static size_t longestCaptureForKing(const Board* gameboard, int piecePos, int** out) {
    *out = NULL;
    return 0;
}

/* TODO - finish these. it's gonna be complicated... */
size_t boardGetLongestCaptureStreakForPiece(const Board* gameboard, int piecePos, int** out) {
    if (!gameboard || !validPos(piecePos) || !out) {
        if (out) {
            *out = NULL;
        }
        return 0;
    }
    if (!gameboard->board[piecePos].alive) {
        *out = NULL;
        return 0;
    }
    *out = NULL;
    if (isKing(gameboard->board + piecePos)) {
        size_t size = longestCaptureForKing(gameboard, piecePos, out);
        return size;
    } else if (isMan(gameboard->board + piecePos)) {
        size_t size = 0;
        CaptureStack stack = { .top = NULL, .size = 0 };
        stack.top = &(Node) {
            .next = NULL,
            .value = piecePos,
        };
        stack.size = 1;
        int l = longestCaptureForMan(gameboard, (isLight(gameboard->board + piecePos) ? CHECKERS_PLAYER_LIGHT : CHECKERS_PLAYER_DARK), piecePos, out, &size, &stack);
        return size;
    } else {
        *out = NULL;
        return 0;
    }
}

/* will this be slow...? */
Moves* boardGetLongestCaptureStreakForPlayer(const Board* gameboard, int player, size_t* out_size);

void boardDestroyMovesList(struct Moves* moves, size_t moves_size) {
    for (size_t i = 0; i < moves_size; i++) {
        free(moves[i].to);
        moves[i].to = NULL;
    }
    free(moves);
}

int boardCheckIfPieceHasAvailableMoves(const Board* gameboard, int pos) {
    if (!gameboard || !validPos(pos)) {
        return 0;
    }
    /* TODO - consider if it's possible to not allocate memory for this */
    int* buf = NULL;
    size_t s = boardGetAvailableMovesForPiece(gameboard, pos, &buf);
    if (s == 0) {
        return 0;
    } else {
        free(buf);
        return 1;
    }
}

int boardCheckIfPlayerHasAvailableMoves(const Board* gameboard, int player) {
    if (!gameboard || !validPlayer(player)) {
        return 0;
    }
    /* TODO - consider if it's possible to not allocate memory for this */
    size_t s = 0;
    Moves* m = boardGetAvailableMovesForPlayer(gameboard, player, &s);
    if (s == 0) {
        return 0;
    } else {
        boardDestroyMovesList(m, s);
        return 1;
    }
}

int boardCheckIfPieceCanCapture(const Board* gameboard, int pos) {
    if (!gameboard || !validPos(pos)) {
        return 0;
    }
    Piece* p = gameboard->board + pos;
    if (!isPiece(p) || !p->alive) {
        return 0;
    }

    if (isKing(p)) {
        int offsetsIdx = 0;
        Point fromP = IDX_TO_POINT(pos);
        int m = IS_EVEN_LINE(pos) ? 1 : -1;
        while (offsetsIdx < 4) {
            int to = 0;
            Point toP = {0};
            to = pos + m * movementOffsets[offsetsIdx];
            toP = IDX_TO_POINT(to);
            Point vec = { /* to get the correct offsets for the direction in the inner loop */
                .x = toP.x - fromP.x < 0 ? -1 : 1,
                .y = toP.y - fromP.y < 0 ? -1 : 1,
            };
            to = pos;
            toP = IDX_TO_POINT(to);
            while (1) {
                int off = fromVecToOffset(vec, to);
                to += off;
                toP.x += vec.x;
                toP.y += vec.y;
                if (!validPos(to) || !validIndex(toP.x, toP.y)) {
                    break;
                }
                Piece* p = gameboard->board + to;
                if (isEmptyField(p)) {
                    continue;
                } else if (canCapture(gameboard, pos, to, toP, vec)) {
                    return 1;
                } else {
                    break;
                }
            }
            offsetsIdx += 1;
        }
    } else {
        int to = 0;
        int m = IS_EVEN_LINE(pos) ? 1 : -1;
        Point fromP = IDX_TO_POINT(pos);
        for (int i = 0; i < 4; i++) {
            to = pos + m * movementOffsets[i];
            Point toP = IDX_TO_POINT(to);
            /* y < 0, up; y > 0, down */
            Point dir = {
                .x = toP.x - fromP.x,
                .y = toP.y - fromP.y, 
            };
            if (validPos(to) && abs(dir.x) == abs(dir.y)) {
                if (canCapture(gameboard, pos, to, toP, dir)) {
                    return 1;
                }
            }
        }
    }
    return 0;
}

int boardCheckIfPlayerCanCapture(const Board* gameboard, int player) {
    if (!gameboard || !validPlayer(player)) {
        return 0;
    }
    int can = 0;
    for (size_t i = 0; i < gameboard->boardSize && !can; i++) {
        if ((player == CHECKERS_PLAYER_LIGHT && isLight(gameboard->board + i)) || (player == CHECKERS_PLAYER_DARK && isDark(gameboard->board + i))) {
            can = boardCheckIfPieceCanCapture(gameboard, i);         
        }
    }
    return can;
}

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
        memset(gameboard, 0, sizeof(Board));
        gameboard->board = NULL; /* prevent use after free */
    }
}

/**
 * 
 * 
 * 
 */

/* this assumes that every argument is valid */
static inline int canCapture(const Board* gameboard, int from, int to, Point toP, Point vec) {
    return (
        isEnemy(gameboard->board + from, gameboard->board + to) && gameboard->board[to].alive &&
        validIndex(toP.x + vec.x, toP.y + vec.y) &&
        isEmptyField(gameboard->board + POINT_TO_IDX(toP.x + vec.x, toP.y + vec.y))
    );
}

static inline int deepcopy(const Board* src, Board* dst) {
    if (!boardInit(dst)) {
        return 0;
    }
    Piece* dstBoard = dst->board;
    memcpy(dst, src, sizeof(Board));
    memcpy(dstBoard, src->board, sizeof(Piece) * src->boardSize);
    dst->board = dstBoard;
    return 1;
}

 /**
  * If we can recreate the {to} value from the starting position {from}
  * plus a few offsets, as long as the movement can be done without
  * obstructions or leads to a valid capture, it is valid.
  * 
  * This function doesn't make any checks to make sure that the arguments
  * are valid nor that they actually point to valid pieces (as in, that the 
  * player is moving the right piece and the piece is alive). This should be
  * done BEFORE calling it.
  */
static int validMove(Board* gameboard, int player, int from, int to) {
    Point playerPos = IDX_TO_POINT(from);
    Point toPos = IDX_TO_POINT(to);
    Point direction = {
        .x = toPos.x - playerPos.x,
        .y = toPos.y - playerPos.y,
    };
    Point vec = {
        .x = direction.x < 0 ? -1 : 1,  /* -1, up; 1, down */
        .y = direction.y < 0 ? -1 : 1,  
    };
    if (abs(direction.x) != abs(direction.y)) {
        return 0;
    }

    int illegalMove = player == CHECKERS_PLAYER_LIGHT ? -1 : 1; /* -1, down; 1, up */
    int vdir = V_DIRECTION(from, to);
    unsigned int lines = abs(vdir);

    Piece* playerPiece = gameboard->board + from;

    if (isKing(playerPiece)) {
        int canMakeMove = 1;
        int off = 0, enemyCount = 0;
        Piece* p = NULL;
        while (from != to) {
            off = fromVecToOffset(vec, from);
            from += off;

            p = gameboard->board + from;
            if (isEnemy(playerPiece, p)) {
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
            return illegalMove != vdir;
        } else if (lines == 2) { /* should be capture only */
            playerPos.x += vec.x;
            playerPos.y += vec.y;
            int idx = POINT_TO_IDX(playerPos.x, playerPos.y);
            return isEnemy(playerPiece, gameboard->board + idx) &&
                gameboard->board[idx].alive;
        } else {
            return 0;
        }
    } else {
        return 0;
    }
}

/* movement/capture should be guaranteed to happen before this function is called  */
static int movePiece(Board* gameboard, int from, int to) {
    gameboard->recentlyMovedPiece = to;
    Point playerPos = IDX_TO_POINT(from);
    Point toPos = IDX_TO_POINT(to);
    Point vec = {
        .x = toPos.x - playerPos.x < 0 ? -1 : 1,
        .y = toPos.y - playerPos.y < 0 ? -1 : 1,  /* -1, up; 1, down */
    };
    unsigned int lines = LINES_MOVED(from, to);
    Piece* playerPiece = gameboard->board + from;
    Piece* targetField = gameboard->board + to;
    playerPiece->recentlyMoved = 1;
    /* all movement is assumed to be valid */
    if (isKing(playerPiece)) {
        int returnStatus = CHECKERS_MOVE_SUCCESS;
        int off = 0;
        Piece* p = NULL;
        while (from != to) {
            off = fromVecToOffset(vec, from);
            from += off;

            p = gameboard->board + from;
            if (isEnemy(playerPiece, p)) {
                returnStatus = CHECKERS_CAPTURE_SUCCESS;
                p->alive = 0;
                from = to;
            }
        }
        *targetField = *playerPiece;
        *playerPiece = emptyField;
        return returnStatus;
    } else if (isMan(playerPiece)) {
        int returnStatus = CHECKERS_MOVE_SUCCESS;
        if (lines == 2) {
            playerPos.x += vec.x;
            playerPos.y += vec.y;
            gameboard->board[POINT_TO_IDX(playerPos.x, playerPos.y)].alive = 0;
            returnStatus = CHECKERS_CAPTURE_SUCCESS;
        }
        *targetField = *playerPiece;
        *playerPiece = emptyField;
        return returnStatus;
    } else {
        return CHECKERS_NOT_A_PIECE;
    }
}

/* y = -1, up; 1, down */
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

/* base case: piece can't capture. recursive case: piece can do at least one capturing move  */
static int longestCaptureForMan(const Board* gameboard, int player, int piecePos, int** out, size_t* size, CaptureStack* stack) {
    int m = IS_EVEN_LINE(piecePos) ? 1 : -1;
    Point fromP = IDX_TO_POINT(piecePos);

    int couldCapture = 0;
    for (int i = 0; i < 4; i++) {
        int to = piecePos + m * movementOffsets[i];
        Point toP = IDX_TO_POINT(to);
        Point dir = {
            .x = toP.x - fromP.x,
            .y = toP.y - fromP.y,
        };
        if (validPos(to) && abs(dir.x) == abs(dir.y)) {
            if (canCapture(gameboard, piecePos, to, toP, dir)) {
                couldCapture = 1;
                /**
                 * clone board, make capture, store move, recursively check for
                 * more captures, save longest moves list, backtrack
                 */
                Board tmp = {0};
                deepcopy(gameboard, &tmp);
                Point dest = {
                    .x = toP.x + dir.x,
                    .y = toP.y + dir.y,
                };
                int newpos = POINT_TO_IDX(dest.x, dest.y);
                /* this should never happen if we can, in fact, capture a piece */
                if (boardTryMoveOrCapture(&tmp, player, piecePos, newpos) != CHECKERS_CAPTURE_SUCCESS) {
                    boardFreeInternalBoard(&tmp);
                    continue;
                }
                /* push move to stack */
                Node t = { /* this will be lost once we leave the context */
                    .next = stack->top,
                    .value = newpos,
                };
                stack->top = &t;
                stack->size++;
                /* check if size is greater than prev and pop move from stack */
                if (!longestCaptureForMan(&tmp, player, newpos, out, size, stack)) {
                    /* found out last move for this path. save it, then compare */
                    if (stack->size > *size) {
                        *size = stack->size;
                        size_t counter = stack->size;
                        int* tmp = malloc(sizeof(int) * stack->size);
                        if (*out) {
                            free(*out);
                            *out = NULL;
                        }
                        for (Node* n = stack->top; n != NULL; n = n->next) {
                            tmp[counter - 1] = n->value;
                            counter--;
                        }
                        *out = tmp;
                    }
                }
                stack->top = stack->top->next;
                stack->size--;
                boardFreeInternalBoard(&tmp);
            }
        }
    }
    return couldCapture;
}