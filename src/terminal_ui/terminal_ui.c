#include "terminal_ui.h"
#include "../game/checkers.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

static char* getline(FILE* file, size_t* out_size);
static int validateInput(char* input);

/** 
 * (a-h)(1-10) || (0-9)(0-9)
 * Ex.: a6, 34
*/
static struct Point getPositionFromStr(char* pos);

void terminalGameBegin(void) {
    terminalGameBeginF(stdin);
}

void terminalGameBeginF(FILE* stepsfile) {
    struct Checkers game;
    checkersInit(&game, 1);
    int quit = 0;

    while (game.flags.run) {
        checkersPrint(&game);
        int currPlayer = checkersGetCurrentPlayer(&game);
        printf("Current player: %d\n", currPlayer);
        size_t linesize = 0;
        char* move = NULL;
        int valid = 0;

        while (!valid) {
            move = getline(stepsfile, &linesize);
            if (strcmp("exit", move) == 0) {
                quit = 1;
                free(move);
                break;
            }
            if (!validateInput(move)) {
                printf("Invalid indices\n");
                free(move);
            } else {
                valid = 1;
            }
        }

        if (quit) {
            break;
        }

        char* mov1 = strtok(move, " ");
        char* mov2 = strtok(NULL, " ");
        struct Point orig = getPositionFromStr(mov1);
        struct Point dest = getPositionFromStr(mov2);
        free(move);
        
        if (checkersPlayerShallCapture(&game)) {
            printf("Player shall capture!!\n");
            struct Checkers future = game;
            int status = checkersMakeMove(&future, orig, dest);
            if (status != CHECKERS_CAPTURE_SUCCESS) {
                printf("Capture failed!\n");
            } else {
                game = future;
            }
        } else {
            int status = checkersMakeMove(&game, orig, dest);
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
}

/* format: oo dd, o -> origin, a1 or 11, d -> destination, a1 or 11 */
static int validateInput(char* input) {
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

static char* getline(FILE* file, size_t* out_size) {
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