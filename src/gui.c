#include "raylib.h"
#include "raymath.h"
#include "gui.h"
#include "new_checkers.h"
// #include "checkers_ai.h"

#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>

#define MAX(a, b) ((a)>(b)? (a) : (b))
#define MIN(a, b) ((a)<(b)? (a) : (b))
#define RESCALE(win, width, height) \
    do {\
        (win).wscale = MIN((float) (width) / (win).gWidth, (float) (height) / (win).gHeight); \
    } while(0)

/* This whole thing is a mess, dude... */

typedef struct Text {
    char* text;
    Vector2 position;
    Vector2 origin;
    float size;
    float spacing;
    float rotation;
    Color color;
    Font* font;
} Text;

static inline void DrawMenuButton(const GameWindow* win, const Rectangle* outline, const Rectangle* inner, Color outlineColor, Color innerColor, char* text, Color textColor);
static inline void DrawTextS(const Text* text);
static inline void DrawCheckersBoard(const GameWindow* win, const Checkers* game);

static int initGUI(GameWindow* win);

static void renderHandleMenu(GameWindow* win, Checkers* game);
static void renderHandlePreGameOpts(GameWindow* win, Checkers* game);
static void renderHandleRun(GameWindow* win, Checkers* game);
static void renderHandleMovingAndAnim(GameWindow* win, Checkers* game);
static void renderHandleOver(GameWindow* win, Checkers* game);
static void renderHandleCloseWindow(GameWindow* win, Checkers* game);

static inline Vector2 getVirtualMousePosition(GameWindow* win) {
    Vector2 virtualMouse;
    virtualMouse.x = (GetMouseX() - (GetScreenWidth() - (win->gWidth * win->wscale)) * 0.5f) / win->wscale;
    virtualMouse.y = (GetMouseY() - (GetScreenHeight() - (win->gHeight * win->wscale)) * 0.5f) / win->wscale;
    virtualMouse = Vector2Clamp(virtualMouse, (Vector2){ 0, 0 }, (Vector2){ (float) win->gWidth, (float) win->gHeight });

    return virtualMouse;
}

static inline void renderWindowScreen(GameWindow* win) {
    BeginDrawing();
    DrawTexturePro(
        win->gameScreen.texture,
        (Rectangle){ .x = 0, .y = 0, .width = win->gWidth, .height = -win->gHeight},
        (Rectangle){
            .x = (GetScreenWidth() - win->gWidth * win->wscale) * 0.5f,
            .y = (GetScreenHeight() - win->gHeight * win->wscale) * 0.5f,
            .width = win->gWidth * win->wscale,
            .height = win->gHeight * win->wscale
        },
        (Vector2){0, 0},
        0.0f,
        WHITE
    );
    EndDrawing();
}

/* TODO - draw a little crown on top of king pieces? */

static inline void unloadResources(GameWindow* game) {
    UnloadRenderTexture(game->gameScreen);
    UnloadFont(game->textFont);
}

/**
 * because of how Raylib works, the gameScreen, wWidth and wHeight members,
 * as well as possibly other things, will only be initialized when guiGameBegin
 * is called, as that will be able to call Raylib's InitWindow function
 */
int guiCreateWindow(GameWindow* win, int gWidth, int gHeight, int smoothAnim, int winflags) {
    if (!win) {
        return 0;
    }
    memset(win, 0, sizeof(GameWindow));
    win->state = WSTATE_MENU;
    win->prevState = 0;

    win->smoothAnim = smoothAnim;
    win->gWidth = gWidth;
    win->gHeight = gHeight;
    win->wFlags = winflags;
    win->gscale = win->gWidth / CHECKERS_BOARD_SIZE;
    win->wscale = 0;

    win->wMinHeight = 200;
    win->wMinWidth = 200;

    win->lightManColor = GOLD;
    win->lightKingColor = Fade(win->lightManColor, 0.5f);
    win->darkManColor = (Color){.r = 105, .g = 71, .b =62, .a = 255}; /* brown */
    win->darkKingColor = Fade(win->darkManColor, 0.5f);

    return 1;
}

void guiGameBegin(Checkers* game) {
    if (game) {
        const int wSide = DEFAULT_SCREEN_SIDE;
        GameWindow window = {0};
        guiCreateWindow(&window, wSide, wSide, 1, FLAG_WINDOW_RESIZABLE | FLAG_VSYNC_HINT | FLAG_WINDOW_ALWAYS_RUN);
        guiGameBeginW(&window, game);
    }
}

/* TODO - reimplement this from the ground up*/
void guiGameBeginW(GameWindow* win, Checkers* game) {
    if (!win || !game) {
        return;
    }
    initGUI(win);
    int moveIdx = 0;
    Point move[2] = {0};
    while (!WindowShouldClose() && win->state != WSTATE_CLOSE) {
        switch (win->state) {
            case WSTATE_MENU: renderHandleMenu(win, game); break;
            case WSTATE_PREGAME_OPTS: renderHandlePreGameOpts(win, game); break;
            case WSTATE_RUN: renderHandleRun(win, game); break;
            case WSTATE_MOVING: renderHandleMovingAndAnim(win, game); break;
            case WSTATE_OVER: renderHandleOver(win, game); break;
            case WSTATE_CLOSE_ASK: renderHandleOver(win, game); break;
        }
    }

    // if (WindowShouldClose() && game->flags.aiEnabled && ai) {
    //     SetWindowTitle("Just a second...");
    //     checkersAiKill(ai);
    // }
    // TODO: Unload all loaded data (textures, fonts, audio) here!
    unloadResources(win);
    CloseWindow();
}

/**
 * 
 * 
 * 
 * 
 * 
 * 
 */

static int initGUI(GameWindow* win)  {
    InitWindow(1, 1, "International Checkers");
    SetWindowState(win->wFlags);

    const int height = GetMonitorHeight(0);
    const int width = GetMonitorWidth(0);

    if (win->wHeight <= 0 || win->wWidth <= 0) {
        win->wWidth = height / 1.5f;
        win->wHeight = win->wWidth;
    }

    if (win->gHeight <= 0 || win->gWidth <= 0) {
        win->gWidth = 800;
        win->gHeight = 800;
    }
    win->gscale = win->gWidth / CHECKERS_BOARD_SIZE;
    
    SetWindowSize(win->wWidth, win->wHeight);
    SetWindowMinSize(win->wMinWidth, win->wMinHeight);
    SetWindowPosition((width - win->wWidth) / 2, (height - win->wHeight) / 2);
    
    if (!IsWindowState(FLAG_VSYNC_HINT)) {
        const int refreshRate = GetMonitorRefreshRate(0);
        SetTargetFPS(refreshRate);
    }
    ShowCursor();

    win->gameScreen = LoadRenderTexture(win->gWidth, win->gHeight);
    win->textFont = GetFontDefault(); /* TODO - consider testing other fonts */
}

/**
 *
 *  
 */

static inline void DrawMenuButton(const GameWindow* win, const Rectangle* outline, const Rectangle* inner, Color outlineColor, Color innerColor, char* text, Color textColor) {    
    const float spacing = 2.3f;
    float textsize = 43.0f * ((float) win->gWidth / DEFAULT_SCREEN_SIDE);
    Vector2 measure = MeasureTextEx(win->textFont, text, textsize, spacing);
    if (measure.x >= inner->width) {
        // printf("oh no\n");
        float fac = inner->width / (measure.x + 15); /* magic numbers oooo */
        textsize = textsize * fac;
        measure = MeasureTextEx(win->textFont, text, textsize, spacing);
    }
    const Text txt = {
        .text = text,
        .position = { .x = outline->x, .y =outline->y },
        .origin = { .x = measure.x / 2, .y = measure.y / 2 },
        .size = textsize,
        .spacing = spacing,
        .rotation = 0,
        .color = textColor,
        .font = &win->textFont
    };
    DrawRectanglePro(*outline, (Vector2){ .x = outline->width / 2.0f, .y = outline->height / 2.0f}, 0.0f, outlineColor);
    DrawRectanglePro(*inner, (Vector2){ .x = outline->width / 2.0f, .y = outline->height / 2.0f}, 0.0f, innerColor);
    DrawTextS(&txt);
}

static inline void DrawTextS(const Text* text) {
    DrawTextPro(
        *text->font,
        text->text,
        text->position,
        text->origin,
        text->rotation,
        text->size,
        text->spacing,
        text->color
    );
}

static inline void DrawCheckersBoard(const GameWindow* win, const Checkers* game) {
    for (size_t i = 0; i < CHECKERS_BOARD_SIZE; i++) {
        for (size_t j = 0; j < CHECKERS_BOARD_SIZE; j++) {
            if ((j + i) % 2 == 0) {
                DrawRectangle(j * win->gscale, i * win->gscale, win->gscale, win->gscale, (Color){.r = 232, .g = 208, .b = 170, .a = 255});
            } else {
                DrawRectangle(j * win->gscale, i * win->gscale, win->gscale, win->gscale, (Color){.r = 166, .g = 125, .b =93, .a = 255});
                Piece* piece = game->checkersBoard.board + POINT_TO_IDX(j, i);
                if (piece->type == PIECE_LIGHT_MAN) {
                    DrawCircle(j * win->gscale + (win->gscale / 2), i * win->gscale + (win->gscale / 2), (float) (win->gscale * 0.85f) / 2, win->lightManColor);
                } else if (piece->type == PIECE_DARK_MAN) {
                    DrawCircle(j * win->gscale + (win->gscale / 2), i * win->gscale + (win->gscale / 2), (float) (win->gscale * 0.85f) / 2, win->darkManColor);
                } else if (piece->type == PIECE_LIGHT_KING) {
                    DrawCircle(j * win->gscale + (win->gscale / 2), i * win->gscale + (win->gscale / 2), (float) (win->gscale * 0.85f) / 2, win->lightKingColor);
                } else if (piece->type == PIECE_DARK_KING) {
                    DrawCircle(j * win->gscale + (win->gscale / 2), i * win->gscale + (win->gscale / 2), (float) (win->gscale * 0.85f) / 2, win->darkKingColor);
                }
            }
        }
    }
}

/**
 * 
 * 
 * 
 */

static void renderHandleMenu(GameWindow* win, Checkers* game) {
    const float percent = 0.87f;
    float offset = 0;
    const Rectangle vsAIButton = {
        .x = win->gWidth * 0.25f,
        .y = win->gHeight * 0.65f,
        .height = win->gHeight * 0.08f,
        .width = win->gWidth * 0.22f
    };
    offset = MIN(vsAIButton.height - vsAIButton.height * percent, vsAIButton.width - vsAIButton.width * percent);
    const Rectangle vsAIButtonInner = {
        .x = vsAIButton.x + offset * 0.5f,
        .y = vsAIButton.y + offset * 0.5f,
        .width = vsAIButton.width - offset,
        .height = vsAIButton.height - offset
    };


    const Rectangle vsPButton = {
        .x = win->gWidth * 0.75f,
        .y = win->gHeight * 0.65f,
        .height = win->gHeight * 0.08f,
        .width = win->gWidth * 0.22f
    };
    offset = MIN(vsPButton.height - vsPButton.height * percent, vsPButton.width - vsPButton.width * percent);
    const Rectangle vsPButtonInner = {
        .x = vsPButton.x + offset * 0.5f,
        .y = vsPButton.y + offset * 0.5f,
        .width = vsPButton.width - offset,
        .height = vsPButton.height - offset
    };


    const Rectangle exitButton = {
        .x = win->gWidth * 0.5f,
        .y = win->gHeight * 0.8f,
        .height = win->gHeight * 0.08f,
        .width = win->gWidth * 0.22f
    };
    offset = MIN(exitButton.height - exitButton.height * percent, exitButton.width - exitButton.width * percent);
    const Rectangle exitButtonInner = {
        .x = exitButton.x + offset * 0.5f,
        .y = exitButton.y + offset * 0.5f,
        .width = exitButton.width - offset,
        .height = exitButton.height - offset
    };
    
    /* collisions */
    const Rectangle AIbBox = {
        .x = vsAIButton.x - vsAIButton.width / 2,
        .y = vsAIButton.y - vsAIButton.height / 2,
        .width = vsAIButton.width,
        .height = vsAIButton.height,
    };
    const Rectangle vsPBox = {
        .x = vsPButton.x - vsPButton.width / 2,
        .y = vsPButton.y - vsPButton.height / 2,
        .width = vsPButton.width,
        .height = vsPButton.height,
    };
    const Rectangle exitBox = {
        .x = exitButton.x - exitButton.width / 2,
        .y = exitButton.y - exitButton.height / 2,
        .width = exitButton.width,
        .height = exitButton.height,
    };

    const Color defaultFontColor = DARKGRAY;

    char* title1text = "International";
    const float title1size = 60 * ((float) win->gWidth / DEFAULT_SCREEN_SIDE);
    Vector2 title1Measure = MeasureTextEx(win->textFont, title1text, title1size, 2.3f);
    const Text title1 = {
        .text = title1text,
        .position = {.x = win->gWidth / 2, .y = win->gHeight * 0.12f },
        .origin = { .x = title1Measure.x / 2, .y = title1Measure.y / 2 },
        .size = title1size,
        .spacing = 2.3f,
        .rotation = 0,
        .color = defaultFontColor,
        .font = &win->textFont
    };

    float rotationSpeed = 0.0f;
    float gravity = 55.0f;
    char* title2text = "Checkers";
    float title2size = 120 * ((float) win->gWidth / DEFAULT_SCREEN_SIDE);
    Vector2 title2Measure = MeasureTextEx(win->textFont, title2text, title2size, 10);
    Text title2 = {
        .text = title2text,
        .position = {.x = win->gWidth / 2, .y = win->gHeight * 0.29f },
        .origin = { .x = title2Measure.x / 2, .y = title2Measure.y / 2 },
        .size = title2size,
        .spacing = 10,
        .rotation = -12,
        .color = defaultFontColor,
        .font = &win->textFont
    };
    int v = 1;
    while (!WindowShouldClose() && win->state == WSTATE_MENU) {
        // IsWindowResized

        /* title2 will rotate around a bit, a bit to the right, and then to the left, smoothly */
        float delta = GetFrameTime();
        rotationSpeed += gravity * delta;
        rotationSpeed = Clamp(rotationSpeed, -30.0f, 30.0f);
        title2.rotation += rotationSpeed * delta;
        // title2.rotation = Clamp(title2.rotation, -rotLimit, rotLimit);

        if ((v == 1 && title2.rotation >= 3) || (v == -1 && title2.rotation <= -3)) {
            gravity *= -1;
            v *= -1;
        }

        // if (fabsf(title2.rotation) >= rotLimit) {
        //     printf("Rev sped\n");
        //     rotationSpeed *= -1;
        // }
        Color vsAIColor = defaultFontColor;
        Color vsPColor = defaultFontColor;
        Color exitColor = defaultFontColor;

        int w = GetScreenWidth();
        int h = GetScreenHeight();
        RESCALE(*win, w, h);
        Vector2 virtualMouse = getVirtualMousePosition(win);

        bool leftMouseButtonPressed = IsMouseButtonPressed(MOUSE_BUTTON_LEFT);

        if (CheckCollisionPointRec(virtualMouse, AIbBox)) {
            vsAIColor = RED;
            if (leftMouseButtonPressed) {
                checkersFlagSetAI(game, 1);
                win->state = WSTATE_PREGAME_OPTS;
            }
        } else if (CheckCollisionPointRec(virtualMouse, vsPBox)) {
            vsPColor = RED;
            if (leftMouseButtonPressed) {
                checkersFlagSetAI(game, 0);
                win->state = WSTATE_PREGAME_OPTS;
            }
        } else if (CheckCollisionPointRec(virtualMouse, exitBox)) {
            exitColor = RED;
            if (leftMouseButtonPressed) {
                win->state = WSTATE_CLOSE;
            }
        }

        BeginTextureMode(win->gameScreen);
            ClearBackground(RAYWHITE);
            DrawTextS(&title1);
            DrawTextS(&title2);
            DrawMenuButton(win, &vsAIButton, &vsAIButtonInner, GRAY, LIGHTGRAY, "vs. Computer", vsAIColor);
            DrawMenuButton(win, &vsPButton, &vsPButtonInner, GRAY, LIGHTGRAY, "2 Player", vsPColor);
            DrawMenuButton(win, &exitButton, &exitButtonInner, GRAY, LIGHTGRAY, "Exit", exitColor);
        EndTextureMode();
        renderWindowScreen(win);
    }
}

static void renderHandlePreGameOpts(GameWindow* win, Checkers* game) {
    /* TODO - finish this */
    win->state = WSTATE_RUN;
}

static int moveIdx = 0;
static Point move[2] = {{-1, -1}, {-1, -1}};
static void renderHandleRun(GameWindow* win, Checkers* game) {
    // struct Ai* ai = NULL;
    // struct AiMoves aiMove;
    int allowPlayerMove = 1;
    // if (game->flags.aiEnabled) {
    //     ai = checkersAiCreate(game);
    // }
    float keepCapturingTimerLimit = 5.0f;
    float keepCapturingTimer = 0;
    while (!WindowShouldClose() && win->state == WSTATE_RUN) {
        switch (game->state) {
            case CSTATE_P1_TURN:    
                if (checkersPlayerMustCapture(game)) {
                    SetWindowTitle("International Checkers - Player one's turn, light pieces. Must capture!!");
                } else {
                    SetWindowTitle("International Checkers - Player one's turn, light pieces");
                }
                break;
            case CSTATE_P2_TURN:    
                // if (checkersFlagIsAIEnabled(game) && ai) {
                    // SetWindowTitle("International Checkers - Player two is thinking...");
                    // allowPlayerMove = 0;
                    // int ok = checkersAiGenMovesAsync(ai);
                    // if (!ok) {
                        // CloseWindow();
                    // }
                    // aiMove = checkersAiTryGetMoves(ai);
                    // if (aiMove.valid) {
                        // move[0] = aiMove.from;
                        // move[1] = aiMove.to;
                        // handleMove(game, move);
                        // allowPlayerMove = 1;
                    // }
                // } else {
                    if (checkersPlayerMustCapture(game)) {
                        SetWindowTitle("International Checkers - Player two's turn, dark pieces. Must capture!!");
                    } else {
                        SetWindowTitle("International Checkers - Player two's turn, dark pieces");
                    }
                // }
                break;
            case CSTATE_END_P1_WIN: 
                SetWindowTitle("International Checkers - Player one wins!");
                break;
            case CSTATE_END_P2_WIN: 
                SetWindowTitle("International Checkers - Player two wins!");
                break;
            case CSTATE_END_DRAW:   
                SetWindowTitle("International Checkers - Draw!!!");
                break;
        }

        if (IsWindowResized()) {
            int w = GetScreenWidth();
            int h = GetScreenHeight();
            RESCALE(*win, w, h);
        }
        Vector2 virtualMouse = getVirtualMousePosition(win);

        int x = virtualMouse.x / win->gscale;
        int y = virtualMouse.y / win->gscale;

        if (!checkersFlagIsForceCaptureOn(game) && checkersFlagIsCurrentlyCapturing(game)) {
            keepCapturingTimer += GetFrameTime();
            if (keepCapturingTimer >= keepCapturingTimerLimit) {
                checkersFlagSetNeedsUpdate(game);
                checkersEndTurn(game);
                keepCapturingTimer = 0;
                move[moveIdx] = (Point){.x = x, .y = y};
                moveIdx = 0;
            }
        }

        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && allowPlayerMove) {
            if (moveIdx == 0) {
                move[moveIdx] = (Point){.x = x, .y = y};
                moveIdx += 1;
            } else if (moveIdx == 1) {
                if (x != move[0].x || y != move[0].y) {
                    move[moveIdx] = (Point){.x = x, .y = y};
                    moveIdx = 0;
                    if (checkersFlagIsExternalCaptureHandleEnabled(game)) {
                        if (checkersPlayerMustCapture(game)) {
                            /* TODO - check if player movement is within longest path */
                            if (checkersFlagIsAutoCapturesOn(game)) {
                                /* maybe a while loop where we keep updating the movements... */
                                printf("Should autocapture and follow longest path\n");
                            } else {
                                /*  */
                                printf("Should capture manually, but follow longest path\n");
                            }
                        } else {
                            checkersMakeMoveP(game, move[0], move[1]);
                            move[0] = move[1] = (Point) {-1, -1};
                            // win->state = WSTATE_MOVING; 
                        }
                    } else {
                        checkersMakeMoveP(game, move[0], move[1]);
                        move[0] = move[1] = (Point) {-1, -1};
                    }
                } else {
                    moveIdx = 0;
                }
            }
        }

        BeginTextureMode(win->gameScreen);
            ClearBackground(BLACK);
            DrawCheckersBoard(win, game);
            DrawRectangleLinesEx((Rectangle){x * win->gscale, y * win->gscale, win->gscale, win->gscale}, 4, checkersGetCurrentPlayer(game) == CHECKERS_PLAYER_LIGHT ? win->lightManColor : win->darkManColor);
            if (moveIdx == 1) {
                DrawRectangleLinesEx((Rectangle){move[0].x * win->gscale, move[0].y * win->gscale, win->gscale, win->gscale}, 4, BLUE);
            }
        EndTextureMode();
        renderWindowScreen(win);
    }
}

/* TODO - finish this */
static void renderHandleMovingAndAnim(GameWindow* win, Checkers* game) {
    if (checkersPlayerMustCapture(game)) {
        if (game->capturesSize == 0) {
            game->capturesSize = checkersGetLongestCaptureStreakForPlayer(game, &game->captures);
            game->capturesIdx = 0;
            if (game->capturesSize == 0) {
                game->captures = NULL;
                game->capturesIdx = 0;
            }
        }
        /**
         * the longest capture sequence leads to the capture
         * of a single piece, so any move will be valid as long
         * as it leads to a capture, which was already confirmed
         * to be the case when we get to this point
         */
        if (game->capturesSize == 2) {
            free(game->captures);
            game->captures = NULL;
            game->capturesSize = 0;
            game->capturesIdx = 0;
            // win->state = WSTATE_RUN;
        }
    }
    int from = POINT_TO_IDX(move[0].x, move[0].y);
    int to = POINT_TO_IDX(move[1].x, move[1].y);

    int animating = 0;
    int changestate = 0;
    float speed = 20.0f;
    Vector2 orig = { .x = move[0].x, .y = move[0].y };
    Vector2 dest = { .x = move[1].x, .y = move[1].y };
    Vector2 vec = Vector2Normalize((Vector2) { .x = dest.x - orig.x, .y = dest.y - orig.y });
    while (!WindowShouldClose() && win->state == WSTATE_MOVING) {
        /* TODO - make the pieces move from one place to another */
        
        int w = GetScreenWidth();
        int h = GetScreenHeight();
        RESCALE(*win, w, h);
        Vector2 virtualMouse = getVirtualMousePosition(win);

        int x = virtualMouse.x / win->gscale;
        int y = virtualMouse.y / win->gscale;

        BeginTextureMode(win->gameScreen);
            ClearBackground(BLACK);
            DrawCheckersBoard(win, game);
            /* TODO - draw the one piece that's moving */
            DrawRectangleLinesEx((Rectangle){x * win->gscale, y * win->gscale, win->gscale, win->gscale}, 4, checkersGetCurrentPlayer(game) == CHECKERS_PLAYER_LIGHT ? win->lightManColor : win->darkManColor);
        EndTextureMode();
        renderWindowScreen(win);

        win->state = WSTATE_RUN;
        // checkersMakeMoveP(game, move[0], move[1]);
    }
    move[0] = (Point) {-1, -1};
    move[1] = (Point) {-1, -1};
}

static void renderHandleOver(GameWindow* win, Checkers* game) {
    /* TODO - finish this */
    win->state = WSTATE_RUN;
}

static void renderHandleCloseWindow(GameWindow* win, Checkers* game) {
    /* TODO - */
    win->state = WSTATE_RUN;
}

