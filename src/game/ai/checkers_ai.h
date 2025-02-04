#ifndef CHECKERS_AI_H
#define CHECKERS_AI_H

#include "../checkers.h"

struct AiMoves {
    int valid;
    struct Point from, to;
};

struct Ai;

struct Ai* checkersAiCreate(struct Checkers* gameboard);
int checkersAiGenMovesAsync(struct Ai* ai);
struct AiMoves checkersAiGenMovesSync(struct Ai* ai);
struct AiMoves checkersAiTryGetMoves(struct Ai* ai);
void checkersAiKill(struct Ai* ai);

#endif /* CHECKERS_AI_H */