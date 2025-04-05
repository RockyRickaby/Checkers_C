#ifndef GUI_H
#define GUI_H

#include "new_checkers.h"
#include "raylib.h"

#include <stdio.h>

#define DEFAULT_SCREEN_SIDE 1000

typedef enum WindowState {
    WSTATE_MENU,
    WSTATE_PREGAME_OPTS,
    WSTATE_RUN,
    WSTATE_MOVING,
    WSTATE_OVER,
    WSTATE_CLOSE_ASK,
    WSTATE_CLOSE,
} WindowState;

typedef struct GameWindow {
    WindowState state;
    WindowState prevState;
    RenderTexture2D gameScreen; /* */
    Font textFont;
    int smoothAnim;
    /* there will be an attempt to have smooth animations for when pieces are moved */
    /* resolution in which the game should be rendered (permanent) */
    int gWidth, gHeight;
    /** 
     * the window's initial size.
     * if either one is 0, the width and height will be set automatically
     */
    int wWidth, wHeight; /* */
    int wMinWidth, wMinHeight;
    float gscale;
    float wscale;
    int wFlags;
    
    /* pieces' colors */
    Color lightManColor;
    Color lightKingColor;
    Color darkManColor;
    Color darkKingColor;
} GameWindow;

/**
 * because of how Raylib works, the gameScreen, wWidth and
 * wHeight members will only be initialized when guiGameBegin
 * is called, as that will be able to call Raylib's InitWindow function.
 */
int guiCreateWindow(GameWindow* win, int gWidth, int gHeight, int smoothAnim, int winflags);
void guiGameBeginW(GameWindow* win, Checkers* game);
void guiGameBegin(Checkers* game);


#endif /* GUI_H */