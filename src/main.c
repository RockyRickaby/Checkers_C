#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stddef.h>

#include "game/checkers.h"
#include "terminal_ui/terminal_ui.h"
#include "gui/gui.h"

int main(int argc, char const *argv[]) {
    struct Checkers game;
    checkersInit(&game, 1);
    // terminalCheckersBeginF(&game, stdin);
    guiGameBegin(&game);
    // if (argc == 2 && strcmp(argv[1], "help") == 0) {
    //     printf(
    //         "Usage:\n"
    //         "\t./main.exe\n"
    //         "\t./main.exe <enable_forcecapture> <enable_ai>\tEx.: ./main 1 0\n"
    //         "\t./main.exe <moves_file_path> <enable_forcecapture> <enable_ai>\t.Ex: ./main test.txt 0 1\n"
    //         "\t./main.exe <use_gui> <enable_forcecapture> <enable_ai>\tEx.: ./main 1 test.txt 1 1\n"
    //         "\t./main.exe <use_gui> <moves_file_path> <enable_forcecapture> <enable_ai>\tEx.: ./main 1 test.txt 1 1\n"
    //     );
    //     return 0;
    // }

    // if (argc == 3) {
    //     int forcecap, aienable;
    //     forcecap = atoi(argv[1]);
    //     aienable = atoi(argv[2]);

    //     struct Checkers game;
    //     // checkersInit()
    // } else if (argc == 4) {

    // } else if (argc == 5) {

    // } else {
    //     FILE* file = fopen("test.txt", "r");
    //     if (!file) {
    //         printf("fail\n");
    //         return 1;
    //     }
    //     struct Checkers game;
    //     checkersInit(&game, 1);
    //     // terminalCheckersBeginF(&game, stdin);
    //     guiGameBegin(&game);
    //     fclose(file);
    //     return 0;
    // }
}
