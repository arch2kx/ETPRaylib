#include "raylib.h"
#include <cmath>
#include "game.hpp"
#include "difficulty.hpp"
#include "paths.hpp"

typedef enum GameScreen { LOGO = 0, TITLE, DIFFICULTY_SELECT, GAMEPLAY, ENDING, WIN } GameScreen;

const char* DIFF_NAMES[]              = { "EASY", "NORMAL", "HARD", "EXTREME" };
const DifficultySettings DIFF_LIST[]  = { DIFF_EASY, DIFF_NORMAL, DIFF_HARD, DIFF_EXTREME };
const Color DIFF_COLORS[]             = { GREEN, YELLOW, ORANGE, RED };

int main() {
    const int screenWidth  = 800;
    const int screenHeight = 600;

    InitWindow(screenWidth, screenHeight, "Eden Treaty Pandemonium C++");

    // Window/taskbar/dock icon (shown while the game is running)
    Image windowIcon = LoadImage(AssetPath("etp_cpp.png").c_str());
    if (windowIcon.data != nullptr) {
        ImageFormat(&windowIcon, PIXELFORMAT_UNCOMPRESSED_R8G8B8A8);
        SetWindowIcon(windowIcon);
        UnloadImage(windowIcon);
    }

    SetTargetFPS(60);

    // Braces ensure Game and textures are destroyed before CloseWindow()
    {
    Texture2D background  = LoadTexture(AssetPath("background-trinity.png").c_str());
    Texture2D bgSelect    = LoadTexture(AssetPath("background-select.png").c_str());
    Texture2D titleScreen = LoadTexture(AssetPath("titlescreen.png").c_str());
    Font font = LoadFontEx(AssetPath("PressStart2P-Regular.ttf").c_str(), 20, nullptr, 0);
    Game game;

    GameScreen currentScreen = LOGO;
    int framesCounter = 0;
    int selectedDiff = 1;  // default: NORMAL

    while (!WindowShouldClose()) {
        float dt = GetFrameTime();

        switch (currentScreen) {
            case LOGO:
                framesCounter++;
                if (framesCounter > 0) currentScreen = TITLE;
                break;

            case TITLE:
                if (IsKeyPressed(KEY_ENTER)) currentScreen = DIFFICULTY_SELECT;
                break;

            case DIFFICULTY_SELECT:
                if (IsKeyPressed(KEY_UP))   selectedDiff = (selectedDiff + 3) % 4;
                if (IsKeyPressed(KEY_DOWN)) selectedDiff = (selectedDiff + 1) % 4;
                if (IsKeyPressed(KEY_ENTER)) {
                    game.Reset(DIFF_LIST[selectedDiff]);
                    currentScreen = GAMEPLAY;
                }
                break;

            case GAMEPLAY:
                game.Update(dt);
                if (game.IsGameOver()) currentScreen = ENDING;
                if (game.IsGameWon())  currentScreen = WIN;
                break;

            case ENDING:
                if (IsKeyPressed(KEY_ENTER)) {
                    game.Reset();
                    currentScreen = TITLE;
                }
                break;

            case WIN:
                if (IsKeyPressed(KEY_ENTER)) {
                    game.Reset();
                    currentScreen = TITLE;
                }
                break;
        }

        BeginDrawing();
        ClearBackground(BLACK);

        switch (currentScreen) {
            case TITLE:
                DrawTexturePro(background,
                    Rectangle{0, 0, (float)background.width, (float)background.height},
                    Rectangle{0, 0, (float)screenWidth, (float)screenHeight},  // background fills screen
                    Vector2{0, 0}, 0.0f, WHITE);
                DrawTexturePro(titleScreen,
                    Rectangle{0, 0, (float)titleScreen.width, (float)titleScreen.height},
                    Rectangle{150, 100, 500, 200},  // change these to resize/move the logo
                    Vector2{0, 0}, 0.0f, WHITE);
                DrawTextEx(font, "PRESS ENTER TO PLAY", Vector2{200, 400}, 20, 1, WHITE);
                break;

            case DIFFICULTY_SELECT: {
                DrawTexturePro(bgSelect,
                    Rectangle{0, 0, (float)bgSelect.width, (float)bgSelect.height},
                    Rectangle{0, 0, (float)screenWidth, (float)screenHeight},
                    Vector2{0, 0}, 0.0f, WHITE);
                Vector2 titleSz = MeasureTextEx(font, "SELECT DIFFICULTY", 20, 1);
                DrawTextEx(font, "SELECT DIFFICULTY", Vector2{(screenWidth - titleSz.x) / 2, 100}, 20, 1, WHITE);
                for (int i = 0; i < 4; i++) {
                    Color c = (i == selectedDiff) ? YELLOW : DIFF_COLORS[i];
                    Vector2 sz = MeasureTextEx(font, DIFF_NAMES[i], 20, 1);
                    float tx = (screenWidth - sz.x) / 2;
                    float ty = 210 + i * 70;
                    if (i == selectedDiff)
                        DrawTextEx(font, ">", Vector2{tx - 30, ty}, 20, 1, YELLOW);
                    DrawTextEx(font, DIFF_NAMES[i], Vector2{tx, ty}, 20, 1, c);
                }
                Vector2 hintSz = MeasureTextEx(font, "UP/DOWN   ENTER TO CONFIRM", 12, 1);
                DrawTextEx(font, "UP/DOWN   ENTER TO CONFIRM", Vector2{(screenWidth - hintSz.x) / 2, 510}, 12, 1, GRAY);
            } break;

            case GAMEPLAY:
                game.Draw();
                break;

            case ENDING: {
                DrawTexturePro(bgSelect,
                    Rectangle{0, 0, (float)bgSelect.width, (float)bgSelect.height},
                    Rectangle{0, 0, (float)screenWidth, (float)screenHeight},
                    Vector2{0, 0}, 0.0f, WHITE);
                Vector2 s1 = MeasureTextEx(font, "GAME OVER!", 30, 1);
                Vector2 s2 = MeasureTextEx(font, "PRESS ENTER TO RETURN TO TITLE", 14, 1);
                DrawTextEx(font, "GAME OVER!", Vector2{(screenWidth - s1.x) / 2, 230}, 30, 1, RED);
                DrawTextEx(font, "PRESS ENTER TO RETURN TO TITLE", Vector2{(screenWidth - s2.x) / 2, 320}, 14, 1, WHITE);
            } break;

            case WIN: {
                DrawTexturePro(bgSelect,
                    Rectangle{0, 0, (float)bgSelect.width, (float)bgSelect.height},
                    Rectangle{0, 0, (float)screenWidth, (float)screenHeight},
                    Vector2{0, 0}, 0.0f, WHITE);
                Vector2 w1 = MeasureTextEx(font, "YOU WIN!", 30, 1);
                Vector2 w2 = MeasureTextEx(font, "ALL ENEMIES WERE DEFEATED!", 20, 1);
                Vector2 w3 = MeasureTextEx(font, "PRESS ENTER TO RETURN TO TITLE", 14, 1);
                DrawTextEx(font, "YOU WIN!", Vector2{(screenWidth - w1.x) / 2, 200}, 30, 1, GREEN);
                DrawTextEx(font, "ALL ENEMIES WERE DEFEATED!", Vector2{(screenWidth - w2.x) / 2, 270}, 20, 1, WHITE);
                DrawTextEx(font, "PRESS ENTER TO RETURN TO TITLE", Vector2{(screenWidth - w3.x) / 2, 350}, 14, 1, WHITE);
            } break;
        }

        EndDrawing();
    }

    UnloadTexture(background);
    UnloadTexture(bgSelect);
    UnloadTexture(titleScreen);
    UnloadFont(font);
    } // Game destructor runs here — before CloseWindow

    CloseWindow();
    return 0;
}
