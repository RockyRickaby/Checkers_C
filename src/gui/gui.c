#include "raylib.h"
#include "raymath.h"
#include "gui.h"
#include "../game/checkers.h"
#include "../game/ai/checkers_ai.h"

#include <stdio.h>
#include <ctype.h>

#define MAX(a, b) ((a)>(b)? (a) : (b))
#define MIN(a, b) ((a)<(b)? (a) : (b))

static void handleMove(struct Checkers* game, struct Point move[2]);

void guiGameBegin(struct Checkers* game) {
    struct Ai* ai = NULL;
    struct AiMoves aiMove;
    int allowPlayerMove = 1;
    if (game->flags.aiEnabled) {
        ai = checkersAiCreate(game);
    }
    Color playerTwoColor = (Color){.r = 105, .g = 71, .b =62, .a = 255};
    Color playerOneColor = GOLD;

    InitWindow(1, 1, "International Checkers");
    SetWindowState(FLAG_WINDOW_RESIZABLE);

    const int height = GetMonitorHeight(0);
    const int width = GetMonitorWidth(0);

    const int screenWidth = height / 1.5f;
    const int screenHeight = screenWidth;
    SetWindowSize(screenWidth, screenHeight);
    SetWindowMinSize(200, 200);
    SetWindowPosition((width - screenWidth) / 2, (height - screenHeight) / 2);

    int framesCounter = 0;          // Useful to count frames
    // float delta = GetFrameTime();
    int refreshRate = GetMonitorRefreshRate(0);
    SetTargetFPS(refreshRate);
    ShowCursor();

    const int gameWidth = 800;
    const int gameHeight = 800;
    RenderTexture2D gameScreen = LoadRenderTexture(gameWidth, gameHeight);

    int boardQuadSize = gameWidth / game->checkersBoard.boardSize;
    int moveIdx = 0;
    struct Point move[2] = {0};
    while (!WindowShouldClose()) {
        switch (game->state) {
            case CSTATE_P1_TURN:    
                if (checkersPlayerShallCapture(game)) {
                    SetWindowTitle("International Checkers - Player one's turn, light pieces. Must capture!!");
                } else {
                    SetWindowTitle("International Checkers - Player one's turn, light pieces");
                }
                break;
            case CSTATE_P2_TURN:    
                if (game->flags.aiEnabled && ai) {
                    SetWindowTitle("International Checkers - Player two is thinking...");
                    allowPlayerMove = 0;
                    int ok = checkersAiGenMovesAsync(ai);
                    if (!ok) {
                        CloseWindow();
                    }
                    aiMove = checkersAiTryGetMoves(ai);
                    if (aiMove.valid) {
                        move[0] = aiMove.from;
                        move[1] = aiMove.to;
                        handleMove(game, move);
                        allowPlayerMove = 1;
                    }
                } else {
                    if (checkersPlayerShallCapture(game)) {
                        SetWindowTitle("International Checkers - Player two's turn, dark pieces. Must capture!!");
                    } else {
                        SetWindowTitle("International Checkers - Player two's turn, dark pieces");
                    }
                }
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
        float scale = MIN((float) GetScreenWidth() / gameWidth, (float) GetScreenHeight() / gameHeight);
        struct Point mousePos = {.x = -1, .y = -1};
        int x = GetMouseX();
        int y = GetMouseY();

        Vector2 virtualMouse;
        virtualMouse.x = (x - (GetScreenWidth() - (gameWidth * scale)) * 0.5f) / scale;
        virtualMouse.y = (y - (GetScreenHeight() - (gameHeight * scale)) * 0.5f) / scale;
        virtualMouse = Vector2Clamp(virtualMouse, (Vector2){ 0, 0 }, (Vector2){ (float) gameWidth, (float) gameHeight });

        x = virtualMouse.x / boardQuadSize;
        y = virtualMouse.y / boardQuadSize;
        
        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && allowPlayerMove) {
            if (moveIdx == 0) {
                move[moveIdx] = (struct Point){.x = x, .y = y};
                moveIdx += 1;
            } else if (moveIdx == 1) {
                if (x != move[0].x || y != move[0].y) {
                    move[moveIdx] = (struct Point){.x = x, .y = y};
                    moveIdx = 0;
                    handleMove(game, move);
                    move[0] = move[1] = (struct Point) {-1, -1};
                } else {
                    moveIdx = 0;
                }
            }
        }

        BeginTextureMode(gameScreen);
            ClearBackground(BLACK);
            for (int i = 0; i < game->checkersBoard.boardSize; i++) {
                for (int j = 0; j < game->checkersBoard.boardSize; j++) {
                    if ((j + i) % 2 == 0) {
                        DrawRectangle(j * boardQuadSize, i * boardQuadSize, boardQuadSize, boardQuadSize, (Color){.r = 232, .g = 208, .b = 170, .a = 255});
                    } else {
                        DrawRectangle(j * boardQuadSize, i * boardQuadSize, boardQuadSize, boardQuadSize, (Color){.r = 166, .g = 125, .b =93, .a = 255});
                    }

                    char ch = game->checkersBoard.board[i][j];
                    if (ch == game->checkersBoard.pieceLightMan) {
                        DrawCircle(j * boardQuadSize + (boardQuadSize / 2), i * boardQuadSize + (boardQuadSize / 2), (float) (boardQuadSize * 0.85f) / 2, playerOneColor);
                    } else if (ch == game->checkersBoard.pieceDarkMan) {
                        DrawCircle(j * boardQuadSize + (boardQuadSize / 2), i * boardQuadSize + (boardQuadSize / 2), (float) (boardQuadSize * 0.85f) / 2, playerTwoColor);
                    } else if (ch == game->checkersBoard.pieceLightKing) {
                        DrawCircle(j * boardQuadSize + (boardQuadSize / 2), i * boardQuadSize + (boardQuadSize / 2), (float) (boardQuadSize * 0.85f) / 2, Fade(playerOneColor, .5f));
                    } else if (ch == game->checkersBoard.pieceDarkKing) {
                        DrawCircle(j * boardQuadSize + (boardQuadSize / 2), i * boardQuadSize + (boardQuadSize / 2), (float) (boardQuadSize * 0.85f) / 2, Fade(playerTwoColor, .5f));
                    }
                }
            }
            DrawRectangleLinesEx((Rectangle){x * boardQuadSize, y * boardQuadSize, boardQuadSize, boardQuadSize}, 4, checkersGetCurrentPlayer(game) == CHECKERS_PLAYER_ONE ? playerOneColor : playerTwoColor);
            if (moveIdx == 1) {
                DrawRectangleLinesEx((Rectangle){move[0].x * boardQuadSize, move[0].y * boardQuadSize, boardQuadSize, boardQuadSize}, 4, BLUE);
            }
        EndTextureMode();

        BeginDrawing();
            ClearBackground(BLACK);
            DrawTexturePro(
                gameScreen.texture,
                (Rectangle){0.0f, 0.0f, (float) gameScreen.texture.width, (float) -gameScreen.texture.height},
                (Rectangle){(GetScreenWidth() - ((float) gameWidth*scale)) * 0.5f, (GetScreenHeight() - ((float) gameHeight*scale)) * 0.5f, (float) gameWidth*scale, (float) gameHeight*scale},
                (Vector2){0,0},
                0.0f,
                WHITE
            );
        EndDrawing();
    }

    if (WindowShouldClose() && game->flags.aiEnabled && ai) {
        SetWindowTitle("Just a second...");
        checkersAiKill(ai);
    }
    // TODO: Unload all loaded data (textures, fonts, audio) here!
    UnloadRenderTexture(gameScreen);
    CloseWindow();
}

static void handleMove(struct Checkers* game, struct Point move[2]) {
    if (checkersPlayerShallCapture(game)) {
        struct Checkers future = *game;
        int status = checkersMakeMove(&future, move[0], move[1]);
        if (status == CHECKERS_CAPTURE_SUCCESS) {
            *game = future;
        }
    } else {
        int status = checkersMakeMove(game, move[0], move[1]);
    }
}