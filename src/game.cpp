#include "game.hpp"
#include <cstring>
#include "paths.hpp"
#include <algorithm>
#include <cstdlib>
#include <ctime>
#include <cmath>

Game::Game(DifficultySettings settings, bool endless, uint64_t seed)
    : rng(seed), simTime(0.0f), score(0), killCount(0), loopCount(0), endlessMode(endless),
      difficulty(1.0f), spawnTimer(0.0f),
      spawnDelay(1.5f * settings.spawnDelayMult),
      bossSpawned(false), gameOver(false), gameWon(false),
      phase2Active(false), phase3Active(false), missedEnemyCount(0), settings(settings) {

    bgTexture         = LoadTexture(AssetPath("backgroundGehenna.png").c_str());
    playerTex         = LoadTexture(AssetPath("mikaPlayer.png").c_str());
    enemyTex          = LoadTexture(AssetPath("gehennaMobChanEnemy.png").c_str());
    bossTex           = LoadTexture(AssetPath("makotoBoss.png").c_str());
    friendlyBulletTex = LoadTexture(AssetPath("bulletFriendly.png").c_str());
    enemyBulletTex    = LoadTexture(AssetPath("bulletEnemy.png").c_str());
    sniperTex         = LoadTexture(AssetPath("harunaSniper.png").c_str());
    arTex             = LoadTexture(AssetPath("junkoAR.png").c_str());
    healthBarTex      = LoadTexture(AssetPath("mikaBowHP.png").c_str());
    font              = LoadFontEx(AssetPath("PressStart2P-Regular.ttf").c_str(), 18, nullptr, 0);

    player = Player(playerTex, friendlyBulletTex, healthBarTex, settings.playerSpeedMult, settings.playerBulletCount);
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
    UnloadTexture(healthBarTex);
    UnloadFont(font);
}

// Custom color(s)
constexpr Color CUSTOM_TEXT_BLUE = { 37, 42, 74, 255 };

void Game::Update(float dt, const InputState& in) {
    if (gameOver || gameWon) return;

    simTime += dt;
    player.Update(dt, in, playerBullets);

    // Spawn enemies until boss appears
    if (!bossSpawned) {
        spawnTimer += dt;
        if (spawnTimer >= spawnDelay) {
            SpawnEnemy();
            spawnTimer = 0.0f;
            difficulty += settings.difficultyRamp;
            spawnDelay  = fmaxf(0.3f, 1.5f * settings.spawnDelayMult - difficulty * 0.1f);

            if (score >= settings.bossScoreThresh || difficulty >= 5.0f) {
                SpawnSnipers();
                phase2Active = true;
                bossSpawned = true;
            }
        }
    }

    for (auto& e : enemies) e.Update(dt, simTime, enemyBullets);

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

        // Extreme: once only one Junko unit is left standing, it fires
        // much faster, a tension spike instead of the phase just diminishing.
        int aliveCount = 0;
        for (auto& s : ar) if (s.IsActive()) aliveCount++;
        for (auto& s : ar) {
            s.SetRage(settings.arRageEnabled && aliveCount == 1 && s.IsActive());
            s.Update(dt, enemyBullets, pcx, pcy);
        }
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
            boss = std::make_unique<Boss>(bossTex, enemyBulletTex, settings);
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
                EndCycleOrLoop();
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
                    score += 14;
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


static inline void HashF(uint64_t& h, float v) {
    uint32_t bits;
    std::memcpy(&bits, &v, sizeof(bits));
    for (int i = 0; i < 4; i++) {
        h ^= (uint8_t)(bits >> (i * 8));
        h *= 1099511628211ULL;
    }
}

static inline void HashI(uint64_t& h, long long v) {
    for (int i = 0; i < 8; i++) {
        h ^= (uint8_t)(v >> (i * 8));
        h *= 1099511628211ULL;
    }
}

uint64_t Game::StateHash() const {
    uint64_t h = 1469598103934665603ULL;
    HashI(h, score);
    HashI(h, killCount);
    HashI(h, loopCount);
    HashI(h, missedEnemyCount);
    HashI(h, player.health);
    HashI(h, gameOver ? 1 : 0);
    HashI(h, gameWon ? 1 : 0);
    HashI(h, bossSpawned ? 1 : 0);
    HashF(h, difficulty);
    HashF(h, spawnTimer);
    HashF(h, simTime);
    Rectangle pr = player.GetRect();
    HashF(h, pr.x); HashF(h, pr.y);
    HashI(h, (long long)enemies.size());
    for (const auto& e : enemies) { Rectangle r = e.GetRect(); HashF(h, r.x); HashF(h, r.y); }
    HashI(h, (long long)playerBullets.size());
    for (const auto& b : playerBullets) { Rectangle r = b.GetRect(); HashF(h, r.x); HashF(h, r.y); }
    HashI(h, (long long)enemyBullets.size());
    for (const auto& b : enemyBullets) { Rectangle r = b.GetRect(); HashF(h, r.x); HashF(h, r.y); }
    HashI(h, (long long)snipers.size());
    HashI(h, (long long)ar.size());
    HashI(h, (long long)rng.State());
    return h;
}

void Game::SpawnEnemy() {
    float x = 50.0f + (float)rng.Range(700);
    enemies.emplace_back(x, (int)difficulty, settings.enemySpeedMult, enemyTex, enemyBulletTex, rng);
}

void Game::SpawnSnipers() {
    // Count/HP/fire-rate/spray all now come from DifficultySettings, evenly
    // spaced across the playfield instead of fixed hardcoded positions.
    int count = settings.sniperCount;
    const float margin = 100.0f, usable = 800.0f - 2.0f * margin;
    for (int i = 0; i < count; i++) {
        float px = (count == 1) ? 400.0f : margin + usable * ((float)i / (count - 1));
        snipers.emplace_back(px, sniperTex, enemyBulletTex, settings.sniperHP,
                              settings.sniperCooldown, settings.sniperSpray);
    }
}

void Game::SpawnARs() {
    int count = settings.arCount;
    const float margin = 100.0f, usable = 800.0f - 2.0f * margin;
    for (int i = 0; i < count; i++) {
        float px = (count == 1) ? 400.0f : margin + usable * ((float)i / (count - 1));
        ar.emplace_back(px, arTex, enemyBulletTex, settings.arHP, settings.arCooldown);
    }
}

void Game::EndCycleOrLoop() {
    if (!endlessMode) {
        gameWon = true;
        return;
    }

    // Endless: scale stats up and loop back into another boss wave instead
    // of ending the run. Score/kills carry over, only the run state resets.
    loopCount++;
    settings.bossHP           = (int)(settings.bossHP * 1.15f);
    settings.bossBulletSpeed *= 1.05f;
    settings.enemySpeedMult  *= 1.05f;
    settings.difficultyRamp  *= 1.05f;
    settings.spawnDelayMult   = fmaxf(0.3f, settings.spawnDelayMult * 0.95f);
    settings.bossScoreThresh += 300;  // require more score before the next boss triggers
    settings.sniperHP        += 1;    // Haruna/Junko keep pace with the boss across loops too
    settings.arHP             += 1;
    settings.sniperCooldown   = fmaxf(0.6f, settings.sniperCooldown * 0.95f);
    settings.arCooldown       = fmaxf(0.6f, settings.arCooldown * 0.95f);
    settings.bossShootCooldown = fmaxf(0.3f, settings.bossShootCooldown * 1.4f);

    // Content progression: Unlock boss attack patterns progressively as loops go.
    // Avoids the issue of repetitive gameplay, who knows?
    // 
    // I'm might be such a Touhou larper :)
    if (loopCount >= 2) settings.bossAimedShot     = true;
    if (loopCount >= 4) settings.bossBurst         = true;
    if (loopCount >= 3) settings.sniperSpray       = true;
    if (loopCount >= 3) settings.arRageEnabled     = true;
    if (loopCount >= 5) settings.bossSpiralEnabled = true;  // unlocks going into WAVE 6

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

// Draw the score in gameplay section
void Game::DrawScore() const {
    DrawTextEx(font, TextFormat("SCORE: %d", score),
               Vector2{10, 10}, 18, 1, CUSTOM_TEXT_BLUE);
    DrawTextEx(font, TextFormat("G*HENNANS ELIMINATED: %d", killCount),
               Vector2{10, 34}, 18, 1, CUSTOM_TEXT_BLUE);
    if (endlessMode) {
        DrawTextEx(font, TextFormat("WAVE: %d", loopCount + 1),
                   Vector2{10, 58}, 18, 1, CUSTOM_TEXT_BLUE);
    }
}

bool Game::IsGameOver() const { return gameOver; }
bool Game::IsGameWon()  const { return gameWon; }
int  Game::GetScore()   const { return score; }

void Game::Reset(DifficultySettings newSettings, bool endless, uint64_t seed) {
    rng.Seed(seed);
    simTime     = 0.0f;
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
    player      = Player(playerTex, friendlyBulletTex, healthBarTex, newSettings.playerSpeedMult, newSettings.playerBulletCount);
}
