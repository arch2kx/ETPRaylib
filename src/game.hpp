#pragma once
#include "raylib.h"
#include "player.hpp"
#include "enemy.hpp"
#include "boss.hpp"
#include "sniper.hpp"
#include "ar.hpp"
#include "bullet.hpp"
#include "difficulty.hpp"
#include "input_state.hpp"
#include "rng.hpp"
#include <vector>
#include <memory>

class Game {
public:
    Game(DifficultySettings settings = DIFF_NORMAL, bool endless = false, uint64_t seed = 0);
    ~Game();

    void Update(float dt, const InputState& in);
    void Draw() const;
    bool IsGameOver() const;
    bool IsGameWon() const;
    int  GetScore() const;
    uint64_t StateHash() const;
    void Reset(DifficultySettings settings = DIFF_NORMAL, bool endless = false, uint64_t seed = 0);

private:
    // Textures loaded once here, passed by value to entities (raylib textures are GPU handles)
    Texture2D bgTexture;
    Texture2D playerTex;
    Texture2D enemyTex;
    Texture2D bossTex;
    Texture2D friendlyBulletTex;
    Texture2D enemyBulletTex;
    Texture2D sniperTex;
    Texture2D arTex;
    Texture2D healthBarTex;
    Font      font;

    Player              player;
    std::vector<Enemy>  enemies;
    std::vector<Bullet> playerBullets;
    std::vector<Bullet> enemyBullets;
    std::unique_ptr<Boss>    boss;
    std::vector<Sniper>      snipers;
    std::vector<AR>          ar;
    bool                     phase2Active;
    bool                     phase3Active;

    Rng   rng;
    float simTime;
    int   score;
    int   killCount;
    int   loopCount;      // Endless mode: how many full boss->sniper->AR cycles cleared
    bool  endlessMode;
    float difficulty;
    float spawnTimer;
    float spawnDelay;
    bool  bossSpawned;
    bool  gameOver;
    bool  gameWon;
    int   missedEnemyCount;
    DifficultySettings settings;

    void SpawnEnemy();
    void SpawnSnipers();
    void SpawnARs();
    void CheckCollisions();
    void EndCycleOrLoop();  // Called when boss->sniper->AR cycle fully clears
    void DrawScore() const;
};
