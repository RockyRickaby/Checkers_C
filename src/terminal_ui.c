#include "terminal_ui.h"
#include "checkers.h"
#include "checkers_ai.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

static inline int validateInput(char* input);
static char* readLine(FILE* file, size_t* out_size);
static void handleMove(struct Checkers* game, struct Point orig, struct Point dest);

/** 
 * (a-j)(0-9) || (0-9)(0-9)
 * Ex.: a6, 34
*/
static struct Point getPositionFromStr(char* pos);

void terminalCheckersBegin(struct Checkers* game) {
    terminalCheckersBeginF(game, stdin);
}

void terminalCheckersBeginF(struct Checkers* game, FILE* stepsfile) {
    struct Ai* ai = NULL;
    if (game->flags.aiEnabled) {
        ai = checkersAiCreate(game);
    }
    while (game->flags.run) {
        checkersPrint(game);
        int currPlayer = checkersGetCurrentPlayer(game);
        if (currPlayer == CHECKERS_PLAYER_ONE) {
            printf("Current player: Player one, light pieces (%c and %c)\n", game->checkersBoard.pieceLightMan, game->checkersBoard.pieceLightKing);
        } else {
            printf("Current player: Player two, dark pieces (%c and %c)\n", game->checkersBoard.pieceDarkMan, game->checkersBoard.pieceDarkKing);
            if (game->flags.aiEnabled && ai) {
                printf("Thinking...\n");
                struct AiMoves moves = checkersAiGenMovesSync(ai);
                if (!moves.valid) {
                    break;
                }
                handleMove(game, moves.from, moves.to);
                continue;
            }
        }
        size_t linesize = 0;
        char* move = NULL;
        int valid = 0;

        while (!valid) {
            move = readLine(stepsfile, &linesize);
            if (strcmp("exit", move) == 0) {
                free(move);
                return;
            }
            if (!validateInput(move)) {
                printf("Invalid indices\n");
                free(move);
            } else {
                valid = 1;
            }
        }

        char* mov1 = strtok(move, " ");
        char* mov2 = strtok(NULL, " ");
        struct Point orig = getPositionFromStr(mov1);
        struct Point dest = getPositionFromStr(mov2);
        free(move);

        handleMove(game, orig, dest);
    }

    checkersPrint(game);

    if (game->state == CSTATE_END_P1_WIN) {
        printf("Player one wins!! Turns: %d\n", game->turnsTotal);
    } else if (game->state == CSTATE_END_P2_WIN) {
        printf("Player two wins!! Turns: %d\n", game->turnsTotal);
    } else if (game->state == CSTATE_END_DRAW) {
        printf("Draw!! Turns: %d\n", game->turnsTotal);
    } else {
        printf("Something went wrong...\n");
    }

    checkersAiKill(ai);
}

static void handleMove(struct Checkers* game, struct Point orig, struct Point dest) {
    if (checkersPlayerShallCapture(game)) {
        struct Checkers future = *game;
        int status = checkersMakeMove(&future, orig, dest);
        if (status != CHECKERS_CAPTURE_SUCCESS) {
            printf("Player shall capture!!\n");
            printf("Capture failed!\n\n");
        } else {
            *game = future;
            printf("successful capture!\n\n");
        }
    } else {
        int status = checkersMakeMove(game, orig, dest);
        switch (status) {
            case CHECKERS_CAPTURE_SUCCESS: printf("successful capture!\n\n"); break;
            case CHECKERS_MOVE_SUCCESS:    printf("successful move!\n\n"); break;
            case CHECKERS_NULL_BOARD:      printf("invalid arg. null *board*\n\n"); break;
            case CHECKERS_MOVE_FAIL:       printf("move attempt failed!\n\n"); break;
            case CHECKERS_INVALID_MOVE:    printf("invalid move attempt\n\n"); break;
            case CHECKERS_INVALID_PLAYER:  printf("not a player!\n\n"); break;
            case CHECKERS_NOT_A_PIECE:     printf("index does not represent a current player's piece\n\n"); break;
            // case CHECKERS_NULL_MOVES_LIST: break;
            // case CHECKERS_EMPTY_MOVES_LIST: break;
            // case CHECKERS_INVALID_MOVES_LIST: break;
            default:
                printf("unknown status\n\n");
        }
    }
}

/* format: oo dd, o -> origin, a1 or 11, d -> destination, a1 or 11 */
static inline int validateInput(char* input) {
    return (
        strlen(input) == 5 &&
        input[2] == ' ' &&
        (
            (isalpha((unsigned char) input[0]) && tolower((unsigned char) input[0]) <= 'j') ||
            isdigit((unsigned char) input[0])
        ) &&
        isdigit((unsigned char) input[1]) &&
        (
            (isalpha((unsigned char) input[3]) && tolower((unsigned char) input[3]) <= 'j') ||
            isdigit((unsigned char) input[3])
        ) &&
        isdigit((unsigned char) input[4])
    );
}

static struct Point getPositionFromStr(char* pos) {
    int sub = '0';
    if (isalpha((unsigned char) pos[0])) {
        sub = 'a';
    }
    return (struct Point) {
        .x = tolower((unsigned char) pos[0]) - sub,
        .y = CHECKERS_BOARD_SIZE - 1 - (pos[1] - '0')
    };
}

static char* readLine(FILE* file, size_t* out_size) {
    if (file == NULL || out_size == NULL) {
        return NULL;
    }
    if (feof(file) || ferror(file)) {
        *out_size = 0;
        return NULL;   
    }

    size_t buf_size = 0;
    size_t buf_cap = 10;
    char* buf = calloc(buf_cap, sizeof(char));
    if (buf == NULL) {
        *out_size = 0;
        return NULL;
    }

    int ch = 0;
    int beg = 1; /* to skip over the leftover carriage returns and similar newline chars */
    while ((ch = fgetc(file)) != EOF) {
        if (ch == '\n' || ch == '\r') {
            if (beg && file != stdin) {
                continue;
            }
            break;
        }
        beg = 0;
        if (ch == '\b') {
            if (buf_size > 0) {
                buf_size -= 1;
            }
            continue;
        }
        if (buf_size >= buf_cap) {
            buf_cap *= 1.5f;
            char* new_buf = realloc(buf, buf_cap);
            if (new_buf == NULL) {
                free(buf);
                *out_size = 0;
                return NULL;
            }
            buf = new_buf;
        }
        buf[buf_size++] = ch;
    }

    char* new_buf = realloc(buf, buf_size + 1); /* +1 for the null character */
    if (new_buf == NULL) {
        free(buf);
        *out_size = 0;
        return NULL;
    }
    buf = new_buf;
    buf[buf_size] = '\0';
    *out_size = buf_size;
    return buf;
}