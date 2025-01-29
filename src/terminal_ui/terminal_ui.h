#ifndef TERMINAL_UI_H
#define TERMINAL_UI_H

#include "../game/checkers.h"

#include <stdio.h>

void terminalCheckersBegin(struct Checkers* game);
void terminalCheckersBeginF(struct Checkers* game, FILE* stepsfile);

#endif /* TERMINAL_UI_H */