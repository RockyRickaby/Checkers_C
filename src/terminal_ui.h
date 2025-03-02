#ifndef TERMINAL_UI_H
#define TERMINAL_UI_H

#include "new_checkers.h"

#include <stdio.h>

void terminalCheckersBegin(Checkers* game);
void terminalCheckersBeginF(Checkers* game, FILE* stepsfile);

#endif /* TERMINAL_UI_H */