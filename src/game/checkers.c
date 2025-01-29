#include <string.h>
#include <stdio.h>
#include <math.h>

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

int checkersInit(struct Checkers* game, int forceCapture) {
    if (!game) {
        return 0;
    }
    game->flags.forceCapture = forceCapture > 0;
    game->flags.run = 1;
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
    } else {
        return 0;
    }

    int status = boardTryMoveOrCapture(&game->checkersBoard, player, from, to);
    if (status == CHECKERS_CAPTURE_SUCCESS) {
        if (boardRemainingPiecesPlayer(&game->checkersBoard, enemy) == 0) {
            game->state = nextStateWin;
            game->flags.run = 0;
            game->turnsTotal += 1;
        } else if (game->flags.forceCapture && checkIfPlayerShouldCapture(game, player, to)) {
            game->turnsTotal += 1;
        } else {
            game->turnsTotal += 1;
            game->state = nextStatePlayer;
        }
    } else if (status == CHECKERS_MOVE_SUCCESS) {
        game->turnsTotal += 1;
        game->state = nextStatePlayer;
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

    int prohibitedY = 0;
    if (player == CHECKERS_PLAYER_ONE) {
        playerMan = gameboard->pieceLightMan;
        playerKing = gameboard->pieceLightKing;

        enemyMan = gameboard->pieceDarkMan;
        enemyKing = gameboard->pieceDarkKing;

        prohibitedY = 1;
    } else if (player == CHECKERS_PLAYER_TWO) {
        playerMan = gameboard->pieceDarkMan;
        playerKing = gameboard->pieceDarkKing;

        enemyMan = gameboard->pieceLightMan;
        enemyKing = gameboard->pieceLightKing;
        prohibitedY = -1;
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

        if (capturePiece != gameboard->blank || moveVec.y == prohibitedY) {
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
        return 0; // TODO - check if kings should capture as well
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