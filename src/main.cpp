#include "raylib.h"
#include <cmath>
#include "game.hpp"
#include "difficulty.hpp"
#include "paths.hpp"

typedef enum GameScreen { LOGO = 0, TITLE, DIFFICULTY_SELECT, GAMEPLAY, ENDING, WIN } GameScreen;

// Draws a block of lines, vertically centered around blockCenterY and each
// horizontally centered, with consistent spacing between lines derived from
// each line's own font size + a fixed gap. Keeps every multi-line screen
// (title prompt, win, lose) following the same layout rule instead of each
// using separately eyeballed pixel offsets.
struct TextLine { const char* text; float fontSize; Color color; };

static void DrawCenteredTextBlock(Font font, const TextLine* lines, int count,
                                   float screenW, float blockCenterY, float lineGap) {
    Vector2 sizes[8];
    float totalHeight = 0.0f;
    for (int i = 0; i < count; i++) {
        sizes[i] = MeasureTextEx(font, lines[i].text, lines[i].fontSize, 1);
        totalHeight += sizes[i].y;
        if (i < count - 1) totalHeight += lineGap;
    }

    float y = blockCenterY - totalHeight / 2.0f;
    for (int i = 0; i < count; i++) {
        float x = (screenW - sizes[i].x) / 2.0f;
        DrawTextEx(font, lines[i].text, Vector2{x, y}, lines[i].fontSize, 1, lines[i].color);
        y += sizes[i].y + lineGap;
    }
}

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
constexpr Color CUSTOM_BLUE = { 50, 189, 240, 255 };
constexpr Color CUSTOM_TEXT_BLUE = { 37, 42, 74, 255 };

const Color DIFF_COLORS[]             = { CUSTOM_GREEN, CUSTOM_YELLOW, CUSTOM_ORANGE, CUSTOM_RED, CUSTOM_PURPLE, CUSTOM_PINK };

int main() {
    const int screenWidth  = 800;
    const int screenHeight = 600;

    SetConfigFlags(FLAG_WINDOW_RESIZABLE);
    InitWindow(screenWidth, screenHeight, "Eden Treaty Pandemonium C++");

    InitAudioDevice();  // Initialize audio device

    // Window/taskbar/dock icon (shown while the game is running)
    Image windowIcon = LoadImage(AssetPath("mikaIcon.png").c_str());
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
    Texture2D bgGameOver = LoadTexture(AssetPath("backgroundGehennaMidnight.png").c_str());
    Texture2D bgEnding = LoadTexture(AssetPath("backgroundGehennaNight.png").c_str());
    Texture2D bgSelect    = LoadTexture(AssetPath("backgroundGehennaNight.png").c_str());
    Texture2D titleScreen = LoadTexture(AssetPath("titlescreen.png").c_str());

    // These are painted/illustrated art, not pixel art.
    // Bilinear keeps them smooth when scaled instead of the jagged look point filtering gives
    // non-pixel-art images. titleScreen is pixel art like the font/sprites,
    // so it stays point-filtered for crisp edges instead of joining these.
    SetTextureFilter(background,  TEXTURE_FILTER_BILINEAR);
    SetTextureFilter(bgGameOver,  TEXTURE_FILTER_BILINEAR);
    SetTextureFilter(bgEnding,    TEXTURE_FILTER_BILINEAR);
    SetTextureFilter(bgSelect,    TEXTURE_FILTER_BILINEAR);
    Font font = LoadFontEx(AssetPath("PressStart2P-Regular.ttf").c_str(), 34, nullptr, 0);
    Music bgm = LoadMusicStream(AssetPath("unwelcomeSchoolFami.mp3").c_str());
    SetMusicVolume(bgm, 0.4f);  // From 0.0 to 1.0 in raylib.
    Music victoryBgm = LoadMusicStream(AssetPath("reAoharuFami.mp3").c_str());
    SetMusicVolume(victoryBgm, 0.4f);
    Music titleBgm = LoadMusicStream(AssetPath("hifumiDaisukiFami.mp3").c_str());
    SetMusicVolume(titleBgm, 0.4f);

    // LoadSound failed to decode this file's MP3 encoding (0 Hz, 0 channels,
    // raylib logs a WAVE decode warning but doesn't error out loudly).
    // LoadMusicStream uses a different decode path that handles it fine, so
    // it's used here too even though this is conceptually a one-shot SE,
    // not a loop, looping is turned off below.
    Music gameOverSE = LoadMusicStream(AssetPath("gameOverSE.mp3").c_str());
    SetMusicVolume(gameOverSE, 0.5f);
    gameOverSE.looping = false;
    SetTextureFilter(font.texture, TEXTURE_FILTER_POINT);  // Crisp pixel text, no bilinear blur
    Game game;

    // Everything is drawn at a fixed 800x600 internal resolution into this
    // render texture, then scaled/letterboxed to whatever the real window
    // size is (fullscreen, resized, any monitor aspect ratio). Game code
    // never needs to know the real window size — it always draws at 800x600.
    RenderTexture2D target = LoadRenderTexture(screenWidth, screenHeight);
    SetTextureFilter(target.texture, TEXTURE_FILTER_POINT);  // keep pixel art crisp when scaled up

    GameScreen currentScreen = LOGO;
    GameScreen previousScreen = LOGO;  // tracks the last frame's screen, to detect zone changes
    int framesCounter = 0;
    int selectedDiff = 1;  // default: NORMAL

    // Groups screens into music "zones", so moving between TITLE and
    // DIFFICULTY_SELECT (same zone) doesn't restart the title track, only
    // an actual zone change swaps what's playing.
    auto musicZoneOf = [](GameScreen s) -> int {
        if (s == TITLE || s == DIFFICULTY_SELECT) return 1;
        if (s == GAMEPLAY) return 2;
        if (s == ENDING) return 3;
        if (s == WIN) return 4;
        return 0;
    };

    while (!WindowShouldClose()) {
        float dt = GetFrameTime();

        if (musicZoneOf(currentScreen) != musicZoneOf(previousScreen)) {
            StopMusicStream(bgm);
            StopMusicStream(titleBgm);
            StopMusicStream(gameOverSE);
            StopMusicStream(victoryBgm);
            switch (musicZoneOf(currentScreen)) {
                case 1: PlayMusicStream(titleBgm);   break;
                case 2: PlayMusicStream(bgm);        break;
                case 3: PlayMusicStream(gameOverSE); break;  // Plays once, looping is off
                case 4: PlayMusicStream(victoryBgm); break;
            }
        }
        UpdateMusicStream(bgm);
        UpdateMusicStream(titleBgm);
        UpdateMusicStream(gameOverSE);
        UpdateMusicStream(victoryBgm);
        previousScreen = currentScreen;

        if (IsKeyPressed(KEY_F11)) ToggleFullscreen();

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
                // Store the player's high score in a .txt file for persistent scoring.
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

        BeginTextureMode(target);
        ClearBackground(BLACK);

        //
        // Game graphics get drawn here
        // 
        switch (currentScreen) {
            case TITLE: {
                DrawTexturePro(background,
                    Rectangle{0, 0, (float)background.width, (float)background.height},
                    Rectangle{0, 0, (float)screenWidth, (float)screenHeight},  // Background fills screen
                    Vector2{0, 0}, 0.0f, WHITE);
                DrawTexturePro(titleScreen,
                    Rectangle{0, 0, (float)titleScreen.width, (float)titleScreen.height},
                    Rectangle{210, 70, 400, 280},  // Titlescreen position and size
                    Vector2{0, 0}, 0.0f, WHITE);
                TextLine lines[] = {
                    { "PRESS [ENTER] TO PLAY", 22, CUSTOM_TEXT_BLUE},
                    { TextFormat("HIGH SCORE: %d", highScore), 16, CUSTOM_TEXT_BLUE},
                };
                DrawCenteredTextBlock(font, lines, 2, (float)screenWidth, 400.0f, 16.0f);
            } break;

            case DIFFICULTY_SELECT: {
                DrawTexturePro(bgSelect,
                    Rectangle{0, 0, (float)bgSelect.width, (float)bgSelect.height},
                    Rectangle{0, 0, (float)screenWidth, (float)screenHeight},
                    Vector2{0, 0}, 0.0f, WHITE);
                Vector2 titleSz = MeasureTextEx(font, "SELECT DIFFICULTY", 24, 1);
                DrawTextEx(font, "SELECT DIFFICULTY", Vector2{(screenWidth - titleSz.x) / 2, 100}, 24, 1, WHITE);
                for (int i = 0; i < DIFF_COUNT; i++) {
                    Color c = (i == selectedDiff) ? WHITE : DIFF_COLORS[i];
                    Vector2 sz = MeasureTextEx(font, DIFF_NAMES[i], 22, 1);
                    float tx = (screenWidth - sz.x) / 2;
                    float ty = 190 + i * 55;
                    if (i == selectedDiff)
                        DrawTextEx(font, ">", Vector2{tx - 30, ty}, 22, 1, WHITE);
                    DrawTextEx(font, DIFF_NAMES[i], Vector2{tx, ty}, 22, 1, c);
                }
                Vector2 hintSz = MeasureTextEx(font, "[UP]/[DOWN]   [ENTER] TO CONFIRM", 14, 1);
                DrawTextEx(font, "[UP]/[DOWN] [ENTER] TO CONFIRM", Vector2{(screenWidth - hintSz.x) / 2, 510}, 14, 1, WHITE);
            } break;

            case GAMEPLAY:
                game.Draw();
                break;

            case ENDING: {
                DrawTexturePro(bgGameOver,
                    Rectangle{0, 0, (float)bgGameOver.width, (float)bgGameOver.height},
                    Rectangle{0, 0, (float)screenWidth, (float)screenHeight},
                    Vector2{0, 0}, 0.0f, WHITE);
                TextLine lines[] = {
                    { "DEFEAT!", 34, CUSTOM_RED},
                    { "PRESS [ENTER] FOR TITLE, [R] TO RETRY", 16, WHITE },
                };
                DrawCenteredTextBlock(font, lines, 2, (float)screenWidth, 280.0f, 24.0f);
            } break;

            case WIN: {
                DrawTexturePro(bgEnding,
                    Rectangle{0, 0, (float)bgEnding.width, (float)bgEnding.height},
                    Rectangle{0, 0, (float)screenWidth, (float)screenHeight},
                    Vector2{0, 0}, 0.0f, WHITE);
                TextLine lines[] = {
                    { "VICTORY!", 34, CUSTOM_BLUE },
                    { "ALL ENEMIES WERE DEFEATED!", 22, WHITE },
                    { "PRESS [ENTER] FOR TITLE, [R] TO RETRY", 16, WHITE },
                };
                DrawCenteredTextBlock(font, lines, 3, (float)screenWidth, 280.0f, 24.0f);
            } break;
        }

        EndTextureMode();

        // Scale the fixed-resolution render onto the real window, preserving
        // aspect ratio and letterboxing (bars) instead of stretching.
        float scale = fminf((float)GetScreenWidth() / screenWidth,
                             (float)GetScreenHeight() / screenHeight);

        BeginDrawing();
        ClearBackground(CUSTOM_TEXT_BLUE);
        DrawTexturePro(target.texture,
            Rectangle{ 0.0f, 0.0f, (float)target.texture.width, -(float)target.texture.height },  // negative height: render textures are stored flipped vs the screen
            Rectangle{
                (GetScreenWidth()  - screenWidth  * scale) * 0.5f,
                (GetScreenHeight() - screenHeight * scale) * 0.5f,
                screenWidth  * scale,
                screenHeight * scale
            },
            Vector2{0, 0}, 0.0f, WHITE);
        EndDrawing();
    }

    // Unload the Texture2Ds on user leaving
    UnloadTexture(background);
    UnloadTexture(bgGameOver);
    UnloadTexture(bgEnding);
    UnloadTexture(bgSelect);
    UnloadTexture(titleScreen);
    UnloadFont(font);
    UnloadRenderTexture(target);
    UnloadMusicStream(bgm);
    UnloadMusicStream(titleBgm);
    UnloadMusicStream(gameOverSE);
    UnloadMusicStream(victoryBgm);
    CloseAudioDevice();
    }

    CloseWindow();
    return 0;
}
