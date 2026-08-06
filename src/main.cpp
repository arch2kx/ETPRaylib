#include "raylib.h"
#include <cmath>
#include "game.hpp"
#include "difficulty.hpp"
#include "paths.hpp"

typedef enum GameScreen { LOGO = 0, TITLE, DIFFICULTY_SELECT, GAMEPLAY, ENDING, WIN } GameScreen;

const char* DIFF_NAMES[]              = { "EASY", "NORMAL", "HARD", "EXTREME", "ENDLESS" };
const DifficultySettings DIFF_LIST[]  = { DIFF_EASY, DIFF_NORMAL, DIFF_HARD, DIFF_EXTREME, DIFF_ENDLESS };
constexpr int DIFF_COUNT    = 5;
constexpr int ENDLESS_INDEX = 4;

constexpr Color CUSTOM_RED = { 240, 54, 21, 255 };
constexpr Color CUSTOM_ORANGE = { 247, 95, 30, 255 };
constexpr Color CUSTOM_YELLOW = { 230, 169, 0, 255 };
constexpr Color CUSTOM_GREEN = { 10, 225, 60, 255 };
constexpr Color CUSTOM_PURPLE = { 111, 78, 228, 255 };
constexpr Color CUSTOM_PINK = { 240, 125, 190, 255 };

const Color DIFF_COLORS[]             = { CUSTOM_GREEN, CUSTOM_YELLOW, CUSTOM_ORANGE, CUSTOM_RED, CUSTOM_PURPLE, CUSTOM_PINK };

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

    // Load persisted high score (0 if no save file exists yet)
    int highScore = 0;
    {
        std::string hsPath = SavePath("highscore.txt");
        char* hsText = LoadFileText(hsPath.c_str());
        if (hsText != nullptr) {
            highScore = atoi(hsText);
            UnloadFileText(hsText);
        }
    }

    // Braces ensure Game and textures are destroyed before CloseWindow()
    {
    Texture2D background  = LoadTexture(AssetPath("backgroundGehenna.png").c_str());
    Texture2D bgGameOver = LoadTexture(AssetPath("backgroundGameOver.png").c_str());
    Texture2D bgEnding = LoadTexture(AssetPath("backgroundEnding.png").c_str());
    Texture2D bgSelect    = LoadTexture(AssetPath("backgroundSelect.png").c_str());
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
                if (IsKeyPressed(KEY_UP))   selectedDiff = (selectedDiff + DIFF_COUNT - 1) % DIFF_COUNT;
                if (IsKeyPressed(KEY_DOWN)) selectedDiff = (selectedDiff + 1) % DIFF_COUNT;
                if (IsKeyPressed(KEY_ENTER)) {
                    game.Reset(DIFF_LIST[selectedDiff], selectedDiff == ENDLESS_INDEX);
                    currentScreen = GAMEPLAY;
                }
                break;

            case GAMEPLAY:
                game.Update(dt);
                if (game.IsGameOver() || game.IsGameWon()) {
                    if (game.GetScore() > highScore) {
                        highScore = game.GetScore();
                        std::string hsPath = SavePath("highscore.txt");
                        SaveFileText(hsPath.c_str(), (char*)TextFormat("%d", highScore));
                    }
                }
                if (game.IsGameOver()) currentScreen = ENDING;
                if (game.IsGameWon())  currentScreen = WIN;
                break;

            case ENDING:
                if (IsKeyPressed(KEY_ENTER)) {
                    game.Reset();
                    currentScreen = TITLE;
                }
                if (IsKeyPressed(KEY_R)) {
                    game.Reset(DIFF_LIST[selectedDiff], selectedDiff == ENDLESS_INDEX);
                    currentScreen = GAMEPLAY;
                }
                break;

            case WIN:
                if (IsKeyPressed(KEY_ENTER)) {
                    game.Reset();
                    currentScreen = TITLE;
                }
                if (IsKeyPressed(KEY_R)) {
                    game.Reset(DIFF_LIST[selectedDiff], selectedDiff == ENDLESS_INDEX);
                    currentScreen = GAMEPLAY;
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
                    Rectangle{150, 100, 500, 200},  // Change this to resize/move the logo
                    Vector2{0, 0}, 0.0f, WHITE);
                DrawTextEx(font, "PRESS [ENTER] TO PLAY", Vector2{200, 400}, 20, 1, WHITE);
                DrawTextEx(font, TextFormat("HIGH SCORE: %d", highScore), Vector2{200, 430}, 14, 1, WHITE);
                break;

            case DIFFICULTY_SELECT: {
                DrawTexturePro(bgSelect,
                    Rectangle{0, 0, (float)bgSelect.width, (float)bgSelect.height},
                    Rectangle{0, 0, (float)screenWidth, (float)screenHeight},
                    Vector2{0, 0}, 0.0f, WHITE);
                Vector2 titleSz = MeasureTextEx(font, "SELECT DIFFICULTY", 20, 1);
                DrawTextEx(font, "SELECT DIFFICULTY", Vector2{(screenWidth - titleSz.x) / 2, 100}, 20, 1, WHITE);
                for (int i = 0; i < DIFF_COUNT; i++) {
                    Color c = (i == selectedDiff) ? WHITE : DIFF_COLORS[i];
                    Vector2 sz = MeasureTextEx(font, DIFF_NAMES[i], 20, 1);
                    float tx = (screenWidth - sz.x) / 2;
                    float ty = 190 + i * 55;
                    if (i == selectedDiff)
                        DrawTextEx(font, ">", Vector2{tx - 30, ty}, 20, 1, WHITE);
                    DrawTextEx(font, DIFF_NAMES[i], Vector2{tx, ty}, 20, 1, c);
                }
                Vector2 hintSz = MeasureTextEx(font, "[UP]/[DOWN]   [ENTER] TO CONFIRM", 12, 1);
                DrawTextEx(font, "[UP]/[DOWN] [ENTER] TO CONFIRM", Vector2{(screenWidth - hintSz.x) / 2, 510}, 12, 1, WHITE);
            } break;

            case GAMEPLAY:
                game.Draw();
                break;

            case ENDING: {
                DrawTexturePro(bgGameOver,
                    Rectangle{0, 0, (float)bgGameOver.width, (float)bgGameOver.height},
                    Rectangle{0, 0, (float)screenWidth, (float)screenHeight},
                    Vector2{0, 0}, 0.0f, WHITE);
                Vector2 s1 = MeasureTextEx(font, "YOU LOST!", 30, 1);
                Vector2 s2 = MeasureTextEx(font, "PRESS [ENTER] FOR TITLE, [R] TO RETRY", 14, 1);
                DrawTextEx(font, "YOU LOST!", Vector2{(screenWidth - s1.x) / 2, 230}, 30, 1, CUSTOM_PINK);
                DrawTextEx(font, "PRESS [ENTER] FOR TITLE, [R] TO RETRY", Vector2{(screenWidth - s2.x) / 2, 320}, 14, 1, WHITE);
            } break;

            case WIN: {
                DrawTexturePro(bgEnding,
                    Rectangle{0, 0, (float)bgEnding.width, (float)bgEnding.height},
                    Rectangle{0, 0, (float)screenWidth, (float)screenHeight},
                    Vector2{0, 0}, 0.0f, WHITE);
                Vector2 w1 = MeasureTextEx(font, "YOU AND MIKA WON!", 30, 1);
                Vector2 w2 = MeasureTextEx(font, "ALL ENEMIES WERE DEFEATED!", 20, 1);
                Vector2 w3 = MeasureTextEx(font, "PRESS [ENTER] FOR TITLE, [R] TO RETRY", 14, 1);
                DrawTextEx(font, "YOU WIN!", Vector2{(screenWidth - w1.x) / 2, 200}, 30, 1, CUSTOM_GREEN);
                DrawTextEx(font, "ALL ENEMIES WERE DEFEATED!", Vector2{(screenWidth - w2.x) / 2, 270}, 20, 1, WHITE);
                DrawTextEx(font, "PRESS [ENTER] FOR TITLE, [R] TO RETRY", Vector2{(screenWidth - w3.x) / 2, 350}, 14, 1, WHITE);
            } break;
        }

        EndDrawing();
    }

    // Unload the Texture2Ds on user leaving
    UnloadTexture(background);
    UnloadTexture(bgGameOver);
    UnloadTexture(bgEnding);
    UnloadTexture(bgSelect);
    UnloadTexture(titleScreen);
    UnloadFont(font);
    }

    CloseWindow();
    return 0;
}
