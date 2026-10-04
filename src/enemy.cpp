#include "enemy.hpp"
#include "visuals.hpp"
#include "det.hpp"
#include <cmath>
#include <cstdlib>

Enemy::Enemy(float x, int difficulty, float speedMult, Texture2D tex, Texture2D bulletTex, Rng& rng)
    : texture(tex), bulletTexture(bulletTex),
      x(x), y(-50.0f),
      speed((5.0f + difficulty * 0.8f) * 60.0f * speedMult),
      difficulty(difficulty),
      fireDelay(fmaxf(1.5f - difficulty * 0.2f, 0.3f)),
      fireTimer(0.0f), active(true) {

    if (difficulty >= 3) {
        int r = rng.Range(3);
        if      (r == 0) movementPattern = MovementPattern::STRAIGHT;
        else if (r == 1) movementPattern = MovementPattern::ZIGZAG;
        else             movementPattern = MovementPattern::CIRCLE;
    } else {
        movementPattern = MovementPattern::STRAIGHT;
    }
}

void Enemy::Update(float dt, float simTime, std::vector<Bullet>& enemyBullets) {
    y += speed * dt;

    switch (movementPattern) {
        case MovementPattern::STRAIGHT:
            break;
        case MovementPattern::ZIGZAG:
            x += 5.0f * det::Sin(simTime * 5.0f) * dt * 60.0f;
            break;
        case MovementPattern::CIRCLE:
            x += 10.0f * det::Sin(simTime * 10.0f) * dt * 60.0f;
            break;
    }

    fireTimer += dt;
    if (fireTimer >= fireDelay) {
        Fire(simTime, enemyBullets);
        fireTimer = 0.0f;
    }
}

void Enemy::Fire(float simTime, std::vector<Bullet>& enemyBullets) {
    float cx  = x + SPRITE_SIZE / 2.0f;
    float cy  = y + SPRITE_SIZE;
    const float spd = 250.0f;

    // DEG2RAD is defined by raylib.h
    if (difficulty == 1) {
        enemyBullets.emplace_back(cx, cy, 90.0f * DEG2RAD, spd,
                                  BulletType::ENEMY, bulletTexture);
    } else if (difficulty == 2) {
        for (int angle : {80, 90, 100})
            enemyBullets.emplace_back(cx, cy, angle * DEG2RAD, spd,
                                      BulletType::ENEMY, bulletTexture);
    } else if (difficulty == 3) {
        for (int angle : {70, 80, 90, 100, 110})
            enemyBullets.emplace_back(cx, cy, angle * DEG2RAD, spd,
                                      BulletType::ENEMY, bulletTexture);
    } else {
        // Spiral burst: base angle rotates over time
        float baseAngle = det::Fmod(simTime * 100.0f, 360.0f);
        for (int i = 0; i < 8; i++) {
            float angle = (baseAngle + i * 45.0f) * DEG2RAD;
            enemyBullets.emplace_back(cx, cy, angle, spd,
                                      BulletType::ENEMY, bulletTexture);
        }
    }
}

void Enemy::Draw() const {
    Rectangle src  = { 0, 0, (float)texture.width, (float)texture.height };
    Rectangle dest = { x, y, SPRITE_SIZE, SPRITE_SIZE };
    DrawTexturePro(texture, src, dest, Vector2{0, 0}, 0.0f, WHITE);
}

bool Enemy::IsOffscreen() const { return y > 600; }
bool Enemy::IsActive()    const { return active; }
void Enemy::SetInactive()       { active = false; }

Rectangle Enemy::GetRect() const {
    return Rectangle{ x, y, SPRITE_SIZE, SPRITE_SIZE };
}
