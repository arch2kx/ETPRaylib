#include "game.hpp"
#include "paths.hpp"
#include <algorithm>
#include <cstdlib>
#include <ctime>
#include <cmath>

Game::Game(DifficultySettings settings)
    : score(0), difficulty(1.0f), spawnTimer(0.0f),
      spawnDelay(1.5f * settings.spawnDelayMult),
      bossSpawned(false), gameOver(false), gameWon(false),
      phase2Active(false), missedEnemyCount(0), settings(settings) {
    srand((unsigned)time(nullptr));

    bgTexture         = LoadTexture(AssetPath("background-trinity.png").c_str());
    playerTex         = LoadTexture(AssetPath("mika-player.png").c_str());
    enemyTex          = LoadTexture(AssetPath("gehenna-mob-chan-enemy.png").c_str());
    bossTex           = LoadTexture(AssetPath("makoto_boss.png").c_str());
    friendlyBulletTex = LoadTexture(AssetPath("bullet-friendly.png").c_str());
    enemyBulletTex    = LoadTexture(AssetPath("bullet-enemy.png").c_str());
    sniperTex         = LoadTexture(AssetPath("junko_sniper.png").c_str());
    font              = LoadFontEx(AssetPath("PressStart2P-Regular.ttf").c_str(), 14, nullptr, 0);

    player = Player(playerTex, friendlyBulletTex, settings.playerSpeedMult);
}

Game::~Game() {
    UnloadTexture(bgTexture);
    UnloadTexture(playerTex);
    UnloadTexture(enemyTex);
    UnloadTexture(bossTex);
    UnloadTexture(friendlyBulletTex);
    UnloadTexture(enemyBulletTex);
    UnloadTexture(sniperTex);
    UnloadFont(font);
}

void Game::Update(float dt) {
    if (gameOver || gameWon) return;

    player.Update(dt, playerBullets);

    // Spawn enemies until boss appears
    if (!bossSpawned) {
        spawnTimer += dt;
        if (spawnTimer >= spawnDelay) {
            SpawnEnemy();
            spawnTimer = 0.0f;
            difficulty += settings.difficultyRamp;
            spawnDelay  = fmaxf(0.3f, 1.5f * settings.spawnDelayMult - difficulty * 0.1f);

            if (score >= settings.bossScoreThresh || difficulty >= 5.0f) {
                boss = std::make_unique<Boss>(bossTex, enemyBulletTex, settings);
                bossSpawned = true;
            }
        }
    }

    for (auto& e : enemies) e.Update(dt, enemyBullets);

    if (boss) {
        Rectangle pr = player.GetRect();
        float pcx = pr.x + pr.width  / 2.0f;
        float pcy = pr.y + pr.height / 2.0f;
        boss->Update(dt, enemyBullets, pcx, pcy);
    }

    if (phase2Active) {
        Rectangle pr = player.GetRect();
        float pcx = pr.x + pr.width  / 2.0f;
        float pcy = pr.y + pr.height / 2.0f;
        for (auto& s : snipers) s.Update(dt, enemyBullets, pcx, pcy);
    }

    for (auto& b : playerBullets) b.Update(dt);
    for (auto& b : enemyBullets)  b.Update(dt);

    CheckCollisions();

    // Erase dead/offscreen bullets and enemies
    auto deadBullet = [](const Bullet& b){ return !b.IsActive() || b.IsOffscreen(); };
    playerBullets.erase(std::remove_if(playerBullets.begin(), playerBullets.end(), deadBullet),
                        playerBullets.end());
    enemyBullets.erase( std::remove_if(enemyBullets.begin(),  enemyBullets.end(),  deadBullet),
                        enemyBullets.end());
    for (const auto& e : enemies) {
        if (e.IsOffscreen() && e.IsActive()) {
            missedEnemyCount++;
            if (missedEnemyCount > settings.missPenaltyGrace) {
                player.TakeDamage(25);
                if (player.IsDead()) { gameOver = true; return; }
            }
        }
    }
    enemies.erase(std::remove_if(enemies.begin(), enemies.end(),
        [](const Enemy& e){ return !e.IsActive() || e.IsOffscreen(); }),
        enemies.end());

    if (phase2Active) {
        snipers.erase(std::remove_if(snipers.begin(), snipers.end(),
            [](const Sniper& s){ return !s.IsActive(); }),
            snipers.end());
        if (snipers.empty()) gameWon = true;
    }
}

void Game::CheckCollisions() {
    // Player bullets vs enemies and boss
    for (auto& bullet : playerBullets) {
        if (!bullet.IsActive()) continue;
        Rectangle br = bullet.GetRect();

        for (auto& enemy : enemies) {
            if (!bullet.IsActive()) break;  // already hit something this frame
            if (enemy.IsActive() && CheckCollisionRecs(br, enemy.GetRect())) {
                bullet.SetInactive();
                enemy.SetInactive();
                score += 10;
            }
        }

        if (bullet.IsActive() && boss && CheckCollisionRecs(br, boss->GetRect())) {
            bullet.SetInactive();
            boss->health -= 5;
            score += 10;
            if (boss->IsDead()) {
                score += 500;
                boss.reset();
                if (settings.hasPhase2) {
                    SpawnSnipers();
                    phase2Active = true;
                } else {
                    gameWon = true;
                }
            }
        }

        // Player bullets vs snipers
        if (bullet.IsActive()) {
            for (auto& sniper : snipers) {
                if (!bullet.IsActive()) break;
                if (sniper.IsActive() && CheckCollisionRecs(br, sniper.GetRect())) {
                    bullet.SetInactive();
                    sniper.health--;
                    score += 10;
                    if (sniper.health <= 0) {
                        sniper.SetInactive();
                        score += 50;
                    }
                }
            }
        }
    }

    // Enemy bullets vs player
    Rectangle playerRect = player.GetRect();
    for (auto& bullet : enemyBullets) {
        if (bullet.IsActive() && CheckCollisionRecs(bullet.GetRect(), playerRect)) {
            bullet.SetInactive();
            player.TakeDamage(25);
            if (player.IsDead()) { gameOver = true; return; }
        }
    }
}

void Game::SpawnEnemy() {
    float x = 50.0f + (float)(rand() % 700);
    enemies.emplace_back(x, (int)difficulty, settings.enemySpeedMult, enemyTex, enemyBulletTex);
}

void Game::SpawnSnipers() {
    // 3 Junkos spread across the top
    snipers.emplace_back(150.0f, sniperTex, enemyBulletTex);
    snipers.emplace_back(370.0f, sniperTex, enemyBulletTex);
    snipers.emplace_back(590.0f, sniperTex, enemyBulletTex);
}

void Game::Draw() const {
    DrawTexturePro(bgTexture,
        Rectangle{0, 0, (float)bgTexture.width, (float)bgTexture.height},
        Rectangle{0, 0, 800, 600}, Vector2{0, 0}, 0.0f, WHITE);

    for (const auto& e : enemies)       e.Draw();
    for (const auto& s : snipers)       s.Draw();
    if (boss) { boss->Draw(); boss->DrawHealthBar(); }
    for (const auto& b : playerBullets) b.Draw();
    for (const auto& b : enemyBullets)  b.Draw();
    player.Draw();
    DrawScore();
    player.DrawHealthBar();
}

void Game::DrawScore() const {
    DrawTextEx(font, TextFormat("G*HENNANS ELIMINATED: %d", score),
               Vector2{10, 10}, 14, 1, WHITE);
}

bool Game::IsGameOver() const { return gameOver; }
bool Game::IsGameWon()  const { return gameWon; }

void Game::Reset(DifficultySettings newSettings) {
    enemies.clear();
    playerBullets.clear();
    enemyBullets.clear();
    boss.reset();
    score       = 0;
    difficulty  = 1.0f;
    spawnTimer  = 0.0f;
    spawnDelay  = 1.5f * newSettings.spawnDelayMult;
    bossSpawned      = false;
    gameOver         = false;
    gameWon          = false;
    phase2Active     = false;
    missedEnemyCount = 0;
    snipers.clear();
    settings         = newSettings;
    player      = Player(playerTex, friendlyBulletTex, newSettings.playerSpeedMult);
}
