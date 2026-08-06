#include "game.hpp"
#include "paths.hpp"
#include <algorithm>
#include <cstdlib>
#include <ctime>
#include <cmath>

Game::Game(DifficultySettings settings, bool endless)
    : score(0), killCount(0), loopCount(0), endlessMode(endless),
      difficulty(1.0f), spawnTimer(0.0f),
      spawnDelay(1.5f * settings.spawnDelayMult),
      bossSpawned(false), gameOver(false), gameWon(false),
      phase2Active(false), phase3Active(false), missedEnemyCount(0), settings(settings) {
    srand((unsigned)time(nullptr));

    bgTexture         = LoadTexture(AssetPath("backgroundGehenna.png").c_str());
    playerTex         = LoadTexture(AssetPath("mikaPlayer.png").c_str());
    enemyTex          = LoadTexture(AssetPath("gehennaMobChanEnemy.png").c_str());
    bossTex           = LoadTexture(AssetPath("makotoBoss.png").c_str());
    friendlyBulletTex = LoadTexture(AssetPath("bulletFriendly.png").c_str());
    enemyBulletTex    = LoadTexture(AssetPath("bulletEnemy.png").c_str());
    sniperTex         = LoadTexture(AssetPath("harunaSniper.png").c_str());
    arTex             = LoadTexture(AssetPath("junkoAR.png").c_str());
    font              = LoadFontEx(AssetPath("PressStart2P-Regular.ttf").c_str(), 14, nullptr, 0);

    player = Player(playerTex, friendlyBulletTex, settings.playerSpeedMult);
}

Game::~Game() {
    UnloadTexture(bgTexture);
    UnloadTexture(playerTex);
    UnloadTexture(enemyTex);
    UnloadTexture(arTex);
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

    if (phase3Active) {
        Rectangle pr = player.GetRect();
        float pcx = pr.x + pr.width  / 2.0f;
        float pcy = pr.y + pr.height / 2.0f;
        for (auto& s : ar) s.Update(dt, enemyBullets, pcx, pcy);
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
        if (snipers.empty()) {
            phase2Active = false;
            if (settings.hasPhase3) {
                SpawnARs();
                phase3Active = true;
            } else {
                EndCycleOrLoop();
            }
        }
    }

    if (phase3Active) {
        ar.erase(std::remove_if(ar.begin(), ar.end(),
            [](const AR& s){ return !s.IsActive(); }),
            ar.end());
        if (ar.empty()) {
            phase3Active = false;
            EndCycleOrLoop();
        }
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
                killCount++;
            }
        }

        if (bullet.IsActive() && boss && CheckCollisionRecs(br, boss->GetRect())) {
            bullet.SetInactive();
            boss->health -= 5;
            score += 10;
            if (boss->IsDead()) {
                score += 500;
                killCount++;
                boss.reset();
                if (settings.hasPhase2) {
                    SpawnSnipers();
                    phase2Active = true;
                } else if (settings.hasPhase3) {
                    SpawnARs();
                    phase3Active = true;
                } else {
                    EndCycleOrLoop();
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
                        killCount++;
                    }
                }
            }
        }

        // Player bullets vs AR
        if (bullet.IsActive()) {
            for (auto& gun : ar) {
                if (!bullet.IsActive()) break;
                if (gun.IsActive() && CheckCollisionRecs(br, gun.GetRect())) {
                    bullet.SetInactive();
                    gun.health--;
                    score += 10;
                    if (gun.health <= 0) {
                        gun.SetInactive();
                        score += 50;
                        killCount++;
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
    // 3 Harunas spawn on top
    snipers.emplace_back(150.0f, sniperTex, enemyBulletTex);
    snipers.emplace_back(370.0f, sniperTex, enemyBulletTex);
    snipers.emplace_back(590.0f, sniperTex, enemyBulletTex);
}

void Game::SpawnARs() {
    // 5 junkos spawn on top
    ar.emplace_back(118.0f, arTex, enemyBulletTex);
    ar.emplace_back(252.0f, arTex, enemyBulletTex);
    ar.emplace_back(370.0f, arTex, enemyBulletTex);
    ar.emplace_back(504.0f, arTex, enemyBulletTex);
    ar.emplace_back(590.0f, arTex, enemyBulletTex);
}

void Game::EndCycleOrLoop() {
    if (!endlessMode) {
        gameWon = true;
        return;
    }

    // Endless: scale stats up and loop back into another boss wave instead
    // of ending the run. Score/kills carry over — only the run state resets.
    loopCount++;
    settings.bossHP           = (int)(settings.bossHP * 1.15f);
    settings.bossBulletSpeed *= 1.05f;
    settings.enemySpeedMult  *= 1.05f;
    settings.difficultyRamp  *= 1.05f;
    settings.spawnDelayMult   = fmaxf(0.3f, settings.spawnDelayMult * 0.95f);
    settings.bossScoreThresh += 300;  // require more score before the next boss triggers

    bossSpawned = false;
    difficulty  = 1.0f;
    spawnTimer  = 0.0f;
    spawnDelay  = 1.5f * settings.spawnDelayMult;
}

void Game::Draw() const {
    DrawTexturePro(bgTexture,
        Rectangle{0, 0, (float)bgTexture.width, (float)bgTexture.height},
        Rectangle{0, 0, 800, 600}, Vector2{0, 0}, 0.0f, WHITE);

    for (const auto& e : enemies)       e.Draw();
    for (const auto& s : snipers)       s.Draw();
    for (const auto& g : ar)            g.Draw();
    if (boss) { boss->Draw(); boss->DrawHealthBar(); }
    for (const auto& b : playerBullets) b.Draw();
    for (const auto& b : enemyBullets)  b.Draw();
    player.Draw();
    DrawScore();
    player.DrawHealthBar();
}

void Game::DrawScore() const {
    DrawTextEx(font, TextFormat("SCORE: %d", score),
               Vector2{10, 10}, 14, 1, WHITE);
    DrawTextEx(font, TextFormat("G*HENNANS ELIMINATED: %d", killCount),
               Vector2{10, 30}, 14, 1, WHITE);
    if (endlessMode) {
        DrawTextEx(font, TextFormat("WAVE: %d", loopCount + 1),
                   Vector2{10, 50}, 14, 1, WHITE);
    }
}

bool Game::IsGameOver() const { return gameOver; }
bool Game::IsGameWon()  const { return gameWon; }
int  Game::GetScore()   const { return score; }

void Game::Reset(DifficultySettings newSettings, bool endless) {
    enemies.clear();
    playerBullets.clear();
    enemyBullets.clear();
    boss.reset();
    score       = 0;
    killCount   = 0;
    loopCount   = 0;
    endlessMode = endless;
    difficulty  = 1.0f;
    spawnTimer  = 0.0f;
    spawnDelay  = 1.5f * newSettings.spawnDelayMult;
    bossSpawned      = false;
    gameOver         = false;
    gameWon          = false;
    phase2Active     = false;
    phase3Active     = false;
    missedEnemyCount = 0;
    snipers.clear();
    ar.clear();
    settings         = newSettings;
    player      = Player(playerTex, friendlyBulletTex, newSettings.playerSpeedMult);
}
