#include <string.h>
#include <stdio.h>
#include <math.h>
#include <stdlib.h>

#include "checkers.h"

#define BLK "\e[0;30m"
#define BWHT "\e[1;37m"
#define CRESET "\e[0m"

static int movePiece(struct Board* gameboard, int player, struct Point piecePos, struct Point newPos);
static int checkIfPlayerShouldCapture(struct Checkers* game, int player, struct Point pos);

static int validIndex(int x, int y) { return x >= 0 && x < CHECKERS_BOARD_SIZE && y >= 0 && y < CHECKERS_BOARD_SIZE; }

/**
 * BOARD LOGIC
 * 
 * 
 */

int boardInit(struct Board* gameboard) {
    if (!gameboard) {
        return 0;
    }
    memset(gameboard, '.', sizeof(struct Board));
    gameboard->boardSize = CHECKERS_BOARD_SIZE;
    gameboard->remainingLightPieces = CHECKERS_PIECES_AMOUNT;
    gameboard->remainingDarkPieces = CHECKERS_PIECES_AMOUNT;

    gameboard->pieceLightMan = 'M';
    gameboard->pieceLightKing ='K';

    gameboard->pieceDarkMan = 'm';
    gameboard->pieceDarkKing = 'k';

    gameboard->blank = '.';

    // adding dark pieces
    for (int i = 0; i < (CHECKERS_BOARD_SIZE - 2) / 2; i++) {
        for (int j = 0; j < CHECKERS_BOARD_SIZE; j++) {
            if ((i + j) % 2 == 1) {
                gameboard->board[i][j] = gameboard->pieceDarkMan;
            } else {
                gameboard->board[i][j] = gameboard->blank;
            }
        }
    }

    // adding light pieces
    for (int i = (CHECKERS_BOARD_SIZE - 2) / 2 + 2; i < CHECKERS_BOARD_SIZE; i++) {
        for (int j = 0; j < CHECKERS_BOARD_SIZE; j++) {
            if ((i + j) % 2 == 1) {
                gameboard->board[i][j] = gameboard->pieceLightMan;
            } else {
                gameboard->board[i][j] = gameboard->blank;
            }
        }
    }
    return 1;
}

int boardTryMoveOrCapture(struct Board* gameboard, int player, struct Point piecePos, struct Point newPos) {
    if (!gameboard) {
        return CHECKERS_NULL_BOARD;
    } else if (player == CHECKERS_PLAYER_ONE || player == CHECKERS_PLAYER_TWO) {
        return movePiece(gameboard, player, piecePos, newPos);
    } else {
        return CHECKERS_INVALID_PLAYER;
    }
}

// int boardMultiMoveCapture(struct Board* gameboard, int player, struct Point piecePos, struct Point* movesList, size_t listSize) {
//     if (!gameboard) {
//         return CHECKERS_NULL_BOARD;
//     }
//     if (!movesList) {
//         return CHECKERS_NULL_MOVES_LIST;
//     }
//     if (listSize == 0) {
//         return CHECKERS_EMPTY_MOVES_LIST;
//     }
//     if (player != CHECKERS_PLAYER_ONE && player != CHECKERS_PLAYER_TWO) {
//         return CHECKERS_INVALID_PLAYER;
//     }
//     if (!validIndex(piecePos.x, piecePos.y)) {
//         return CHECKERS_INVALID_MOVE;
//     }
//     size_t i = 0;
//     struct Board boardCopy = *gameboard;
//     while (i < listSize) {
//         struct Point p = movesList[i];
//         // - validate points. they should necessarily lead to captures
//         // kings should be taken into consideration
//         if (!validIndex(p.x, p.y)) {
//             return CHECKERS_INVALID_MOVES_LIST;
//         }
//         int success = boardTryMoveOrCapture(&boardCopy, player, piecePos, p);
//         piecePos = p;
//         if (success != CHECKERS_MOVE_SUCCESS) {
//             return CHECKERS_INVALID_MOVES_LIST;
//         }
//         i++;
//     }
//     *gameboard = boardCopy;
//     return CHECKERS_MOVE_SUCCESS;
// }

void boardTryTurnKing(struct Board* gameboard, struct Point piecePos) {
    if (!validIndex(piecePos.x, piecePos.y)) {
        return;
    }
    if (piecePos.y == 0 && gameboard->board[piecePos.y][piecePos.x] == gameboard->pieceLightMan) {
        gameboard->board[piecePos.y][piecePos.x] = gameboard->pieceLightKing;
    } else if (piecePos.y == CHECKERS_BOARD_SIZE - 1 && gameboard->board[piecePos.y][piecePos.x] == gameboard->pieceDarkMan) {
        gameboard->board[piecePos.y][piecePos.x] = gameboard->pieceDarkKing;
    }
}

int boardRemainingPiecesTotal(struct Board* gameboard) {
    if (gameboard) {
        return gameboard->remainingDarkPieces + gameboard->remainingLightPieces;
    }
    return CHECKERS_NULL_BOARD;
}

int boardRemainingPiecesPlayer(struct Board* gameboard, int player) {
    if (!gameboard) {
        return CHECKERS_NULL_BOARD;
    }
    if (player == CHECKERS_PLAYER_ONE) {
        return gameboard->remainingLightPieces;
    } else if (player == CHECKERS_PLAYER_TWO) {
        return gameboard->remainingDarkPieces;
    } else {
        return CHECKERS_INVALID_PLAYER;
    }
}

static inline int canCapture(struct Board* gameboard, char enemy, char enemyKing, struct Point enemyPos, struct Point moveVec) {
    return (
        (gameboard->board[enemyPos.y][enemyPos.x] == enemy || gameboard->board[enemyPos.y][enemyPos.x] == enemyKing) &&
        validIndex(enemyPos.x + moveVec.x, enemyPos.y + moveVec.y) &&
        gameboard->board[enemyPos.y + moveVec.y][enemyPos.x + moveVec.x] == gameboard->blank
    );
}

int boardGetAvailableMovesForPiece(struct Board* gameboard, struct Point piecePos, struct Point** out, int includeBackwardsCaptures) {
    if (!gameboard || !out) {
        return CHECKERS_NULL_BOARD;
    }
    if (!validIndex(piecePos.x, piecePos.y)) {
        *out = NULL;
        return CHECKERS_INVALID_MOVE;
    }
    if (gameboard->board[piecePos.y][piecePos.x] == gameboard->blank) {
        *out = NULL;
        return 0;
    }
    // finished!!
    int illegalY;
    int bufCap = 2;
    if (includeBackwardsCaptures) {
        bufCap = 4;
    }
    char enemy, enemyKing;
    if (gameboard->board[piecePos.y][piecePos.x] == gameboard->pieceLightMan) {
        enemy = gameboard->pieceDarkMan;
        enemyKing = gameboard->pieceDarkKing;
        illegalY = 1;
    } else if (gameboard->board[piecePos.y][piecePos.x] == gameboard->pieceDarkMan) {
        enemy = gameboard->pieceLightMan;
        enemyKing = gameboard->pieceLightKing;
        illegalY = -1;
    } else {
        if (gameboard->board[piecePos.y][piecePos.x] == gameboard->pieceLightKing) {
            enemy = gameboard->pieceDarkMan;
            enemyKing = gameboard->pieceDarkKing;
        } else if (gameboard->board[piecePos.y][piecePos.x] == gameboard->pieceDarkKing) {
            enemy = gameboard->pieceLightMan;
            enemyKing = gameboard->pieceLightKing;
        } else {
            *out = NULL;
            return 0;
        }
        illegalY = 0;
        bufCap = 12;
    }

    int count = 0;
    float resizeFact = 1.5f;
    struct Point* buf = malloc(sizeof(struct Point) * bufCap);
    if (buf == NULL) {
        *out = NULL;
        return 0;
    }
    if (gameboard->board[piecePos.y][piecePos.x] == gameboard->pieceLightMan || gameboard->board[piecePos.y][piecePos.x] == gameboard->pieceDarkMan) {
        struct Point left = { .x = piecePos.x - 1, .y = piecePos.y + (illegalY * -1)};
        struct Point right = { .x = piecePos.x + 1, .y = left.y};
        if (validIndex(left.x, left.y)) {
            if (canCapture(gameboard, enemy, enemyKing, left, (struct Point){ .x = -1, .y = illegalY * -1})) {
                buf[count++] = (struct Point) {
                    .x = left.x - 1,
                    .y = left.y + (illegalY * -1)
                };
            } else if (gameboard->board[left.y][left.x] == gameboard->blank) {
                buf[count++] = left;
            }
        }
        if (validIndex(right.x, right.y)) {
            if (canCapture(gameboard, enemy, enemyKing, right, (struct Point){ .x = 1, .y = illegalY * -1})) {
                buf[count++] = (struct Point) {
                    .x = right.x + 1,
                    .y = right.y + (illegalY * -1)
                };
            } else if (gameboard->board[right.y][right.x] == gameboard->blank) {
                buf[count++] = right;
            }
        }
        if (includeBackwardsCaptures) {
            struct Point left2 = { .x = piecePos.x - 1, .y = piecePos.y + illegalY};
            struct Point right2 = { .x = piecePos.x + 1, .y = left2.y};
            if (validIndex(left2.x, left2.y) && canCapture(gameboard, enemy, enemyKing, left2, (struct Point){ .x = -1, .y = illegalY})) {
                buf[count++] = (struct Point) {
                    .x = left2.x - 1,
                    .y = left2.y + illegalY
                };
            }
            if (validIndex(right2.x, right2.y) && canCapture(gameboard, enemy, enemyKing, right2, (struct Point){ .x = 1, .y = illegalY})) {
                buf[count++] = (struct Point) {
                    .x = right2.x + 1,
                    .y = right2.y + illegalY
                };
            }
        }
    } else {
        int index = 0;
        struct Point moveVecs[4] = {
            { .x = -1, .y = -1 },
            { .x = -1, .y = 1 },
            { .x = 1, .y = -1 },
            { .x = 1, .y = 1 }
        };
        struct Point positions[4] = {
            {piecePos.x + moveVecs[0].x, .y = piecePos.y + moveVecs[0].y},
            {piecePos.x + moveVecs[1].x, .y = piecePos.y + moveVecs[1].y},
            {piecePos.x + moveVecs[2].x, .y = piecePos.y + moveVecs[2].y},
            {piecePos.x + moveVecs[3].x, .y = piecePos.y + moveVecs[3].y}
        };
        int hasCaptured[4] = {0, 0, 0, 0};

        while (index < 4) {
            while (1) {
                if (!validIndex(positions[index].x, positions[index].y)) {
                    break;
                }
                if (canCapture(gameboard, enemy, enemyKing, positions[index], moveVecs[index])) {
                    if (hasCaptured[index]) {
                        break;
                    }
                    hasCaptured[index] = 1;
                    buf[count++] = (struct Point) {
                        .x = positions[index].x + moveVecs[index].x,
                        .y = positions[index].y + moveVecs[index].y
                    };
                    positions[index].x += 2 * moveVecs[index].x;
                    positions[index].y += 2 * moveVecs[index].y;
                } else if (gameboard->board[positions[index].y][positions[index].x] == gameboard->blank) {
                    buf[count++] = positions[index];
                    positions[index].x += moveVecs[index].x;
                    positions[index].y += moveVecs[index].y;
                } else {
                    break;
                }
                if (count >= bufCap) {
                    bufCap *= resizeFact;
                    struct Point* tmp = realloc(buf, sizeof(struct Point) * bufCap);
                    if (tmp == NULL) {
                        free(buf);
                        *out = NULL;
                        return 0;
                    }
                    buf = tmp;
                }
            }
            index++;
        }
    }
    if (count == 0) {
        *out = NULL;
        free(buf);
    } else {
        struct Point* tmp = realloc(buf, sizeof(struct Point) * count);
        if (tmp == NULL) {
            free(buf);
            *out = NULL;
            return 0;
        }
        *out = tmp;
    }
    return count;
}

void boardPrint(struct Board* gameboard) {
    if (gameboard) {
        printf(
            ">> C h e c k e r s ! <<\n"
            "Light pieces: %c%c\n"
            "Dark pieces: %c%c\n\n"
            "LIGHT PIECES REMAINING: %d\n"
            "DARK PIECES REMAINING: %d\n\n"
            "Current board state:\n",
            gameboard->pieceLightMan,
            gameboard->pieceLightKing,
            gameboard->pieceDarkMan,
            gameboard->pieceDarkKing,
            gameboard->remainingLightPieces,
            gameboard->remainingDarkPieces
        );
        for (int i = 0; i < CHECKERS_BOARD_SIZE; i++) {
            for (int j = 0; j < CHECKERS_BOARD_SIZE; j++) {
                printf("%c ", gameboard->board[i][j]);
            }
            printf("\n");
        }
    }
}

/**
 * GAME LOGIC
 * 
 */

int checkersInit(struct Checkers* game, int forceCapture, int enableAi) {
    if (!game) {
        return 0;
    }
    game->flags.forceCapture = forceCapture > 0;
    game->flags.run = 1;
    game->flags.aiEnabled = enableAi > 0;
    game->state = CSTATE_P1_TURN;
    game->turnsTotal = 0;
    boardInit(&game->checkersBoard);
    return 1;
}

// TODO - Handle CSTATE_END_DRAW
int checkersMakeMove(struct Checkers* game, struct Point from, struct Point to) {
    if (!game || !game->flags.run) {
        return 0;
    }

    int player, enemy;
    enum GameState nextStateWin, nextStatePlayer;
    if (game->state == CSTATE_P1_TURN) {
        player = CHECKERS_PLAYER_ONE;
        enemy = CHECKERS_PLAYER_TWO;
        nextStateWin = CSTATE_END_P1_WIN;
        nextStatePlayer = CSTATE_P2_TURN;
    } else if (game->state == CSTATE_P2_TURN) {
        player = CHECKERS_PLAYER_TWO;
        enemy = CHECKERS_PLAYER_ONE;
        nextStateWin = CSTATE_END_P2_WIN;
        nextStatePlayer = CSTATE_P1_TURN;

        // handle ai here?
    } else {
        return 0;
    }

    int status = boardTryMoveOrCapture(&game->checkersBoard, player, from, to);
    if (status == CHECKERS_CAPTURE_SUCCESS) {
        if (boardRemainingPiecesPlayer(&game->checkersBoard, enemy) == 0) {
            game->state = nextStateWin;
            game->flags.run = 0;
            game->turnsTotal += 1;
            boardTryTurnKing(&game->checkersBoard, to);
        } else if (game->flags.forceCapture && checkIfPlayerShouldCapture(game, player, to)) {
            game->turnsTotal += 1;
        } else {
            game->turnsTotal += 1;
            game->state = nextStatePlayer;
            boardTryTurnKing(&game->checkersBoard, to);
        }
    } else if (status == CHECKERS_MOVE_SUCCESS) {
        game->turnsTotal += 1;
        game->state = nextStatePlayer;
        boardTryTurnKing(&game->checkersBoard, to);
    }
    return status;
}

int checkersGetCurrentPlayer(struct Checkers* game) {
    if (!game || !game->flags.run) {
        return -1;
    }
    if (game->state == CSTATE_P1_TURN) {
        return CHECKERS_PLAYER_ONE;
    } else if (game->state == CSTATE_P2_TURN) {
        return CHECKERS_PLAYER_TWO;
    } else {
        return -1;
    }
}

int checkersGetWinner(struct Checkers* game) {
    if (!game || !game->flags.run) {
        return -1;
    }
    if (game->state == CSTATE_END_P1_WIN) {
        return CHECKERS_PLAYER_ONE;
    } else if (game->state == CSTATE_END_P2_WIN) {
        return CHECKERS_PLAYER_TWO;
    } else {
        return -1;
    }
}

int checkersPlayerShallCapture(struct Checkers* game) {
    if (!game || !game->flags.run || !game->flags.forceCapture) {
        return 0;
    }
    int player = checkersGetCurrentPlayer(game);
    int should = 0;
    for (unsigned int y = 0; y < game->checkersBoard.boardSize && !should; y++) {
        for (unsigned int x = 0; x < game->checkersBoard.boardSize && !should; x++) {
            should = checkIfPlayerShouldCapture(game, player, (struct Point){ .x = x, .y = y });
        }
    }
    return should;
}

struct Moves* checkersGetAvailableMovesForPlayer(struct Checkers* game, size_t* out_size) {
    if (!out_size) {
        return NULL;
    }
    if (!game || !game->flags.run) {
        *out_size = 0;
        return NULL;
    }
    char piece, king;
    if (game->state == CSTATE_P1_TURN) {
        piece = game->checkersBoard.pieceLightMan;
        king = game->checkersBoard.pieceLightKing;
    } else if (game->state == CSTATE_P2_TURN) {
        piece = game->checkersBoard.pieceDarkMan;
        king = game->checkersBoard.pieceDarkKing;
    } else {
        *out_size = 0;
        return NULL;
    }

    size_t cap = 10;
    size_t size = 0;
    struct Moves* list = malloc(sizeof(struct Moves) * cap);
    if (!list) {
        *out_size = 0;
        return NULL;
    }
    for (int y = 0; y < game->checkersBoard.boardSize; y++) {
        for (int x = 0; x < game->checkersBoard.boardSize; x++) {
            if (game->checkersBoard.board[y][x] == piece || game->checkersBoard.board[y][x] == king) {
                struct Moves mov = {0};
                mov.from = (struct Point){ .x = x, .y = y };
                int count = boardGetAvailableMovesForPiece(&game->checkersBoard, mov.from, &mov.to, game->flags.forceCapture);
                if (count > 0 && mov.to != NULL) {
                    mov.to_size = count;
                    list[size++] = mov;
                }
                if (size >= cap) {
                    cap *= 1.5f;
                    struct Moves* tmp = realloc(list, sizeof(struct Moves) * cap);
                    if (!tmp) {
                        free(list);
                        *out_size = 0;
                        return NULL;
                    }
                    list = tmp;
                }
            }
        }
    }
    struct Moves* tmp = realloc(list, sizeof(struct Moves) * size);
    if (!tmp) {
        free(list);
        *out_size = 0;
        return NULL;
    }
    list = tmp;
    *out_size = size;
    return list;
}

void checkersDestroyMovesList(struct Moves* moves, size_t moves_size) {
    if (moves) {
        for (size_t i = 0; i < moves_size; i++) {
            free(moves[i].to);
        }
        free(moves);
    }
}

void checkersPrint(struct Checkers* game) {
    printf(BWHT "Light pieces:" CRESET " %d\n", game->checkersBoard.remainingLightPieces);
    printf(BLK "Dark pieces:" CRESET " %d\n", game->checkersBoard.remainingDarkPieces);
    for (int y = 0; y < CHECKERS_BOARD_SIZE; y++) {
        printf("%d  ", CHECKERS_BOARD_SIZE - 1 - y);
        for (int x = 0; x < CHECKERS_BOARD_SIZE; x++) {
            char ch = game->checkersBoard.board[y][x];
            if (ch == game->checkersBoard.pieceLightMan || ch == game->checkersBoard.pieceLightKing) {
                printf(BWHT "%c" CRESET " ", ch);
            } else if (ch == game->checkersBoard.pieceDarkMan || ch == game->checkersBoard.pieceDarkKing) {
                printf(BLK "%c" CRESET " ", ch);
            } else {
                printf("%c ", ch);
            }
        }
        printf("\n");
    }
    printf("   ");
    for (int i = 0; i < CHECKERS_BOARD_SIZE; i++) {
        printf("%c ", 'A' + i);
    }
    
    char* color = "";
    int player = checkersGetCurrentPlayer(game);
    if (player == CHECKERS_PLAYER_ONE) {
        color = BWHT;
    } else if (player == CHECKERS_PLAYER_TWO) {
        color = BLK;
    }

    printf("\t%sTurn: %d%s\n", color, game->turnsTotal, CRESET);
}

// ----

static int movePiece(struct Board* gameboard, int player, struct Point piecePos, struct Point newPos) {
    if (!validIndex(newPos.x, newPos.y) || !validIndex(piecePos.x, piecePos.y)) {
        return CHECKERS_INVALID_MOVE;
    }
    char playerMan = 0, playerKing = 0;
    char enemyMan = 0, enemyKing = 0;

    int illegalY = 0;
    if (player == CHECKERS_PLAYER_ONE) {
        playerMan = gameboard->pieceLightMan;
        playerKing = gameboard->pieceLightKing;

        enemyMan = gameboard->pieceDarkMan;
        enemyKing = gameboard->pieceDarkKing;

        illegalY = 1;
    } else if (player == CHECKERS_PLAYER_TWO) {
        playerMan = gameboard->pieceDarkMan;
        playerKing = gameboard->pieceDarkKing;

        enemyMan = gameboard->pieceLightMan;
        enemyKing = gameboard->pieceLightKing;
        illegalY = -1;
    } else {
        return CHECKERS_INVALID_PLAYER;
    }

    int x = newPos.x - piecePos.x;
    int y = newPos.y - piecePos.y;

    int absX = abs(x);
    int absY = abs(y);

    char capturePiece = 0;
    struct Point moveVec = {
        .x = x > 0 ? 1 : -1,
        .y = y > 0 ? 1 : -1
    };

    if (gameboard->board[piecePos.y][piecePos.x] == playerKing) {
        if (absX != absY) {
            return CHECKERS_INVALID_MOVE;
        }
        int enemyPieceCount = 0;
        struct Point enemyPiece = { -1, -1 };
        struct Point testPos = piecePos;

        while (testPos.x + moveVec.x != newPos.x + moveVec.x && testPos.y + moveVec.y != newPos.y + moveVec.y) {
            capturePiece = gameboard->board[testPos.y + moveVec.y][testPos.x + moveVec.x];
            if (capturePiece == playerMan || capturePiece == playerKing) {
                return CHECKERS_MOVE_FAIL;
            }
            if (capturePiece == enemyMan || capturePiece == enemyKing) {
                capturePiece = gameboard->board[testPos.y + moveVec.y][testPos.x + newPos.y];
                if (enemyPieceCount >= 1) {
                    return CHECKERS_MOVE_FAIL;
                }
                enemyPieceCount++;
                enemyPiece.x = testPos.x + moveVec.x;
                enemyPiece.y = testPos.y + moveVec.y;
            }
            testPos.x += moveVec.x;
            testPos.y += moveVec.y;
        }

        if (enemyPiece.x == testPos.x && enemyPiece.y == testPos.y) {
            return CHECKERS_MOVE_FAIL;
        }
        int capture = 0;
        if (enemyPiece.x != -1 && enemyPiece.y != -1) {
            if (player == CHECKERS_PLAYER_ONE) {
                gameboard->remainingDarkPieces -= 1;
            } else {
                gameboard->remainingLightPieces -= 1;
            }
            capture = 1;
            gameboard->board[enemyPiece.y][enemyPiece.x] = gameboard->blank;
        }

        gameboard->board[piecePos.y][piecePos.x] = gameboard->blank;
        gameboard->board[newPos.y][newPos.x] = playerKing;
        return capture ? CHECKERS_CAPTURE_SUCCESS : CHECKERS_MOVE_SUCCESS;
    } else if (gameboard->board[piecePos.y][piecePos.x] == playerMan) {
        capturePiece = gameboard->board[piecePos.y + moveVec.y][piecePos.x + moveVec.x];
        
        if (absX != 1 || absY != 1) {
            if (absX == 2 && absY == 2 && (capturePiece == enemyMan || capturePiece == enemyKing)) {
                if (player == CHECKERS_PLAYER_ONE) {
                    gameboard->remainingDarkPieces -= 1;
                } else {
                    gameboard->remainingLightPieces -= 1;
                }
                gameboard->board[piecePos.y + moveVec.y][piecePos.x + moveVec.x] = gameboard->blank;
                gameboard->board[piecePos.y][piecePos.x] = gameboard->blank;
                gameboard->board[newPos.y][newPos.x] = playerMan;
                return CHECKERS_CAPTURE_SUCCESS;
            }
            return CHECKERS_INVALID_MOVE;
        }

        if (capturePiece != gameboard->blank || moveVec.y == illegalY) {
            return CHECKERS_MOVE_FAIL;   
        }

        gameboard->board[piecePos.y][piecePos.x] = gameboard->blank;
        gameboard->board[newPos.y][newPos.x] = playerMan;
        return CHECKERS_MOVE_SUCCESS;
    } else {
        return CHECKERS_NOT_A_PIECE;
    }
}

// ----

static int checkIfPlayerShouldCapture(struct Checkers* game, int player, struct Point pos) {
    char playerPiece = game->checkersBoard.board[pos.y][pos.x];
    char playerMan, playerKing;
    char enemyMan, enemyKing;

    if (player == CHECKERS_PLAYER_ONE) {
        playerMan = game->checkersBoard.pieceLightMan;
        playerKing = game->checkersBoard.pieceLightKing;

        enemyMan = game->checkersBoard.pieceDarkMan;
        enemyKing = game->checkersBoard.pieceDarkKing;
    } else if (player == CHECKERS_PLAYER_TWO) {
        playerMan = game->checkersBoard.pieceDarkMan;
        playerKing = game->checkersBoard.pieceDarkKing;

        enemyMan = game->checkersBoard.pieceLightMan;
        enemyKing = game->checkersBoard.pieceLightKing;
    } else {
        return 0;
    }

    if (playerPiece == playerKing) {
        int index = 0;
        struct Point moveVecs[4] = {
            { .x = -1, .y = -1 },
            { .x = -1, .y = 1 },
            { .x = 1, .y = -1 },
            { .x = 1, .y = 1 }
        };
        struct Point positions[4] = {
            {pos.x + moveVecs[0].x, .y = pos.y + moveVecs[0].y},
            {pos.x + moveVecs[1].x, .y = pos.y + moveVecs[1].y},
            {pos.x + moveVecs[2].x, .y = pos.y + moveVecs[2].y},
            {pos.x + moveVecs[3].x, .y = pos.y + moveVecs[3].y}
        };
        while (index < 4) {
            while (1) {
                if (!validIndex(positions[index].x, positions[index].y)) {
                    break;
                }
                if (game->checkersBoard.board[positions[index].y][positions[index].x] == enemyMan || game->checkersBoard.board[positions[index].y][positions[index].x] == enemyKing) {
                    if (canCapture(&game->checkersBoard, enemyMan, enemyKing, positions[index], moveVecs[index])) {
                        return 1;
                    } else {
                        break;
                    }
                } else if (game->checkersBoard.board[positions[index].y][positions[index].x] != game->checkersBoard.blank) {
                    break;
                }
                positions[index].x += moveVecs[index].x;
                positions[index].y += moveVecs[index].y;
            }
            index++;
        }
        return 0;
    } else if (playerPiece == playerMan) {
        char p[4] = {
            validIndex(pos.x - 1, pos.y - 1) ? game->checkersBoard.board[pos.y - 1][pos.x - 1] : game->checkersBoard.blank,
            validIndex(pos.x + 1, pos.y - 1) ? game->checkersBoard.board[pos.y - 1][pos.x + 1] : game->checkersBoard.blank,
            validIndex(pos.x + 1, pos.y + 1) ? game->checkersBoard.board[pos.y + 1][pos.x + 1] : game->checkersBoard.blank,
            validIndex(pos.x - 1, pos.y + 1) ? game->checkersBoard.board[pos.y + 1][pos.x - 1] : game->checkersBoard.blank
        };
        char c[4] = {
            validIndex(pos.x - 2, pos.y - 2) && game->checkersBoard.board[pos.y - 2][pos.x - 2] == game->checkersBoard.blank ? game->checkersBoard.blank : '\0',
            validIndex(pos.x + 2, pos.y - 2) && game->checkersBoard.board[pos.y - 2][pos.x + 2] == game->checkersBoard.blank ? game->checkersBoard.blank : '\0',
            validIndex(pos.x + 2, pos.y + 2) && game->checkersBoard.board[pos.y + 2][pos.x + 2] == game->checkersBoard.blank ? game->checkersBoard.blank : '\0',
            validIndex(pos.x - 2, pos.y + 2) && game->checkersBoard.board[pos.y + 2][pos.x - 2] == game->checkersBoard.blank ? game->checkersBoard.blank : '\0'
        };
        return (
            ((p[0] == enemyMan || p[0] == enemyKing) && c[0]) ||
            ((p[1] == enemyMan || p[1] == enemyKing) && c[1]) ||
            ((p[2] == enemyMan || p[2] == enemyKing) && c[2]) ||
            ((p[3] == enemyMan || p[3] == enemyKing) && c[3])
        );
    } else {
        return 0;
    }
}