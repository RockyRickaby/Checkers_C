#include "checkers_ai.h"
#include "../checkers.h"

#include <stdlib.h>
#include <stdint.h>
#include <math.h>
#include <float.h>
#include <stdio.h>
#include <assert.h>

#define false   0
#define true    1

#ifdef _WIN32

#define WIN32_LEAN_AND_MEAN 
#include <windows.h>
#include <process.h>
#include <synchapi.h>

typedef uintptr_t aithread;
typedef HANDLE aimutex;

#else

#include <pthread.h>

typedef pthread_mutex_t aimutex;
typedef pthread_t aithread;

#endif

struct Ai {
    struct {
        struct AiMoves content;
        aimutex mutex;
    } queue;
    aithread tid;
    struct Checkers* checkers;
};

static struct AiMoves invalidMove = {
    .valid = 0,
    .from = { .x = -1, .y = -1 },
    .to = { .x = -1, .y = -1 }
};

static inline int createThread(struct Ai* ai);

static inline int createMutex(struct Ai* ai);
static inline int destroyMutex(struct Ai* ai);
static inline int tryLockMutex(struct Ai* ai);
static inline int lockMutex(struct Ai* ai);
static inline int unlockMutex(struct Ai* ai);

static struct AiMoves minimax(struct Ai* ai);

struct Ai* checkersAiCreate(struct Checkers* gameboard) {
    if (!gameboard) {
        return NULL;
    }
    struct Ai* ai = calloc(1, sizeof(struct Ai));
    if (!ai) {
        return NULL;
    }
    ai->queue.content = invalidMove;
    if (!createMutex(ai)) {
        free(ai);
        return NULL;
    }
    ai->tid = 0;
    ai->checkers = gameboard;
    return ai;
}

int checkersAiGenMovesAsync(struct Ai* ai) {
    if (ai) {
        if (!tryLockMutex(ai)) {
            return 1;
        }
        if (ai->queue.content.valid) {
            unlockMutex(ai);
            return 1;
        }
        // initialize thread and return
        if (!createThread(ai)) {
            unlockMutex(ai);
            return 0;
        }
        unlockMutex(ai);
        return 1;
    }
    return 0;
}

struct AiMoves checkersAiGenMovesSync(struct Ai* ai) {
    return minimax(ai);
}

struct AiMoves checkersAiTryGetMoves(struct Ai* ai) {
    if (!ai || !tryLockMutex(ai)) {
        return invalidMove;
    }
    struct AiMoves res = ai->queue.content;
    ai->queue.content = invalidMove;
    unlockMutex(ai);
    return res;
}

void checkersAiKill(struct Ai* ai) {
    if (ai) {
        // wait for thread to finish if it's still running
        #ifdef _WIN32
        WaitForSingleObject((HANDLE) ai->tid, INFINITE);
        #else
        pthread_join(ai->tid, NULL);
        #endif
        destroyMutex(ai);
        memset(ai, 0, sizeof(struct Ai));
        free(ai);
    }
}

/**
 * STATIC FUNCTIONS
 * 
 */

struct MinimaxRet {
    struct AiMoves move;
    double heuristicEval;
};

static double minimaxr(struct Board* gameboard, int forceCapture, int depth, int maximize, struct AiMoves* out);
static double heuristics(struct Board* gameboard);

static struct AiMoves minimax(struct Ai* ai) {
    if (!ai->checkers->flags.run || ai->checkers->state != CSTATE_P2_TURN) {
        return invalidMove;
    }
    struct AiMoves res = invalidMove;
    struct AiMoves dum;
    size_t mSize;
    struct Moves* movesList = boardGetAvailableMovesForPlayer(&ai->checkers->checkersBoard, CHECKERS_PLAYER_TWO, ai->checkers->flags.forceCapture, &mSize);

    struct Moves moveRes;
    double heuristic = LONG_MIN;
    if (ai->checkers->flags.forceCapture && boardCheckIfPlayerCanCapture(&ai->checkers->checkersBoard, CHECKERS_PLAYER_TWO)) {
        for (size_t i = 0; i < mSize; i++) {
            if (!boardCheckIfPieceCanCapture(&ai->checkers->checkersBoard, CHECKERS_PLAYER_TWO, movesList[i].from)) {
                continue;
            }
            for (size_t j = 0; j < movesList[i].to_size; j++) {
                struct Point from = movesList[i].from;
                struct Point to = movesList[i].to[j];
                struct Board future = ai->checkers->checkersBoard;
                int status = boardTryMoveOrCapture(&future, CHECKERS_PLAYER_TWO, from, to);
                if (status != CHECKERS_CAPTURE_SUCCESS) {
                    continue;
                }
                double tmp = minimaxr(&future, ai->checkers->flags.forceCapture, 6, false, &dum);
                if (tmp > heuristic) {
                    heuristic = tmp;
                    moveRes = movesList[i];
                    res = (struct AiMoves){ .valid = 1, .from = moveRes.from, .to = moveRes.to[j] };
                }
            }
        }
    } else {
        for (size_t i = 0; i < mSize; i++) {
            for (size_t j = 0; j < movesList[i].to_size; j++) {
                struct Point from = movesList[i].from;
                struct Point to = movesList[i].to[j];
                struct Board future = ai->checkers->checkersBoard;
                int status = boardTryMoveOrCapture(&future, CHECKERS_PLAYER_TWO, from, to);
                if (status != CHECKERS_MOVE_SUCCESS && status != CHECKERS_CAPTURE_SUCCESS) {
                    continue;   
                }
                double tmp = minimaxr(&future, ai->checkers->flags.forceCapture, 6, false, &dum);
                if (tmp > heuristic) {
                    heuristic = tmp;
                    moveRes = movesList[i];
                    res = (struct AiMoves){ .valid = 1, .from = moveRes.from, .to = moveRes.to[j] };
                }
            }
        }
    }
    checkersDestroyMovesList(movesList, mSize);
    // minimaxr(&ai->checkers->checkersBoard, ai->checkers->flags.forceCapture, 5, true, &res);
    if (!res.valid) {
        return invalidMove;
    }
    return res;
}

static double minimaxr(struct Board* gameboard, int forceCapture, int depth, int maximize, struct AiMoves* out) {
    if (depth == 0) {
        return heuristics(gameboard);
    }
    if (maximize) {
        // ai
        double res = INT_MIN;
        size_t movesSize = 0;
        struct Moves* moves = boardGetAvailableMovesForPlayer(gameboard, CHECKERS_PLAYER_TWO, forceCapture, &movesSize);
        if (forceCapture && boardCheckIfPlayerCanCapture(gameboard, CHECKERS_PLAYER_TWO)) {
            for (size_t i = 0; i < movesSize; i++) {
                if (!boardCheckIfPieceCanCapture(gameboard, CHECKERS_PLAYER_TWO, moves[i].from)) {
                    continue;
                }
                for (size_t j = 0; j < moves[i].to_size; j++) {
                    struct Point from = moves[i].from;
                    struct Point to = moves[i].to[j];
                    struct Board future = *gameboard;
                    int status = boardTryMoveOrCapture(&future, CHECKERS_PLAYER_TWO, from, to);
                    if (status != CHECKERS_CAPTURE_SUCCESS) {
                        continue;
                    }
                    double tmp = minimaxr(&future, forceCapture, depth - 1, false, out);
                    if (tmp > res) {
                        res = tmp;
                        *out = (struct AiMoves){ .valid = 1, .from = from, .to = to };
                    }
                }
            }
        } else {
            for (size_t i = 0; i < movesSize; i++) {
                for (size_t j = 0; j < moves[i].to_size; j++) {
                    struct Point from = moves[i].from;
                    struct Point to = moves[i].to[j];
                    struct Board future = *gameboard;
                    int status = boardTryMoveOrCapture(&future, CHECKERS_PLAYER_TWO, from, to);
                    if (status != CHECKERS_MOVE_SUCCESS && status != CHECKERS_CAPTURE_SUCCESS) {
                        continue;
                    }
                    double tmp = minimaxr(&future, forceCapture, depth - 1, false, out);
                    if (tmp > res) {
                        res = tmp;
                        *out = (struct AiMoves){ .valid = 1, .from = from, .to = to };   
                    }
                }
            }
        }
        checkersDestroyMovesList(moves, movesSize);
        return res;
    } else {
        // player
        double res = INT_MAX;
        size_t movesSize = 0;
        struct Moves* moves = boardGetAvailableMovesForPlayer(gameboard, CHECKERS_PLAYER_ONE, forceCapture, &movesSize);
        if (forceCapture && boardCheckIfPlayerCanCapture(gameboard, CHECKERS_PLAYER_ONE)) {
            for (size_t i = 0; i < movesSize; i++) {
                if (!boardCheckIfPieceCanCapture(gameboard, CHECKERS_PLAYER_ONE, moves[i].from)) {
                    continue;
                }
                for (size_t j = 0; j < moves[i].to_size; j++) {
                    struct Point from = moves[i].from;
                    struct Point to = moves[i].to[j];
                    struct Board future = *gameboard;
                    int status = boardTryMoveOrCapture(&future, CHECKERS_PLAYER_ONE, from, to);
                    if (status != CHECKERS_CAPTURE_SUCCESS) {
                        continue;
                    }
                    double tmp = minimaxr(&future, forceCapture, depth - 1, true, out);
                    if (tmp < res) {
                        res = tmp;
                        *out = (struct AiMoves){ .valid = 1, .from = from, .to = to };
                    }
                    // gameboard = past;
                }
            }
        } else {
            for (size_t i = 0; i < movesSize; i++) {
                for (size_t j = 0; j < moves[i].to_size; j++) {
                    struct Point from = moves[i].from;
                    struct Point to = moves[i].to[j];
                    struct Board future = *gameboard;
                    int status = boardTryMoveOrCapture(&future, CHECKERS_PLAYER_ONE, from, to);
                    if (status != CHECKERS_MOVE_SUCCESS && status != CHECKERS_CAPTURE_SUCCESS) {
                        continue;
                    }
                    double tmp = minimaxr(&future, forceCapture, depth - 1, true, out);
                    if (tmp < res) {
                        res = tmp;
                        *out = (struct AiMoves){ .valid = 1, .from = from, .to = to };   
                    }
                    // gameboard = past;

                }
            }
        }
        checkersDestroyMovesList(moves, movesSize);
        return res;
    }
}

// light - opponent
// dark  - me
static double heuristics(struct Board* gameboard) {
    double rewards = 0.0f;
    if (gameboard->remainingDarkPieces == 0) {
        rewards -= 1000.0f;
    } else if (gameboard->remainingLightPieces == 0) {
        rewards += 1000.0f;
    }
    for (uint8_t i = 0; i < gameboard->boardSize; i++) {
        for (uint8_t j = 0; j < gameboard->boardSize; j++) {
            char ch = gameboard->board[i][j];
            if (ch == gameboard->pieceLightMan) {
                rewards -= 20.0f;
                rewards -= ((10.0f - (i + 1)) / 10.0f) * 10.0f;
                rewards -= (1 - (0.5f/fabs(j - 5.5f))) * 20;
            } else if (ch == gameboard->pieceLightKing) {
                rewards -= 100.0f;
                rewards -= ((10.0f - (i + 1)) / 10.0f) * 10.0f;
                rewards -= (1 - (0.5f/fabs(j - 5.5f))) * 20;
            } else if (ch == gameboard->pieceDarkMan) {
                rewards += 20.0f;
                rewards += ((i + 1) / 10.0f) * 10.0f;
                rewards += (1 - (0.5f/fabs(j - 5.5f))) * 20;
            } else if (ch == gameboard->pieceDarkKing) {
                rewards += 100.0f;
                rewards += ((i + 1) / 10.0f) * 10.0f;
                rewards += (1 - (0.5f/fabs(j - 5.5f))) * 20;
            }
        }
    }
    return rewards;
}

/**
 * 
 * 
 * 
 * n o t h i n g
 * 
 * 
 * 
 */

#ifdef _WIN32
static void threadGenMoves(void* arg) {
#else
static void* threadGenMoves(void* arg) {
#endif
    struct Ai* ai = (struct Ai*) arg;

    lockMutex(ai);
    struct AiMoves res = minimax(ai);
    if (res.valid) {
        ai->queue.content = res;
    } else {
        ai->queue.content = invalidMove;
    }
    unlockMutex(ai);
    
    #ifdef _WIN32
    _endthread();
    #else
    pthread_exit(NULL);
    #endif
}

static inline int createThread(struct Ai* ai) {
    #ifdef _WIN32
    ai->tid = _beginthread(threadGenMoves, 0, (void*) ai);
    return ai->tid != (uintptr_t) -1L;
    #else
    return pthread_create(&ai->tid, NULL, threadGenMoves, (void*) ai) == 0;
    #endif
}

static inline int createMutex(struct Ai* ai) {
    #ifdef _WIN32
    ai->queue.mutex = CreateMutexA(NULL, FALSE, NULL);
    return ai->queue.mutex != NULL;
    #else
    return pthread_mutex_init(&ai->queue.mutex) == 0;
    #endif
}

static inline int destroyMutex(struct Ai* ai) {
    #ifdef _WIN32
    return CloseHandle(ai->queue.mutex);
    #else
    return pthread_mutex_destroy(&ai->queue.mutex) == 0;
    #endif
}

static inline int tryLockMutex(struct Ai* ai) {
    #ifdef _WIN32
    return WaitForSingleObject(ai->queue.mutex, 0) == WAIT_OBJECT_0;
    #else
    return pthread_mutex_trylock(&ai->queue.mutex) == 0;
    #endif
}

static inline int lockMutex(struct Ai* ai) {
    #ifdef _WIN32
    return WaitForSingleObject(ai->queue.mutex, INFINITE) == WAIT_OBJECT_0;
    #else
    return pthread_mutex_lock(&ai->queue.mutex) == 0;
    #endif
}

static inline int unlockMutex(struct Ai* ai) {
    #ifdef _WIN32
    return ReleaseMutex(ai->queue.mutex);
    #else
    return pthread_mutex_unlock(&ai->queue.mutex) == 0;
    #endif
}