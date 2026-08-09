#include "sniper.hpp"
#include "visuals.hpp"
#include <cmath>

Sniper::Sniper(float x, Texture2D tex, Texture2D bulletTex, int hp, float shotCooldown, bool spray)
    : texture(tex), bulletTexture(bulletTex),
      x(x), y(60.0f),
      health(hp),
      driftSpeed(40.0f), driftDir(1),
      aimTimer(0.0f), aimDelay(1.2f),
      cooldownTimer(0.0f), cooldown(shotCooldown),
      active(true), aiming(false),
      aimTargetX(0.0f), aimTargetY(0.0f),
      sprayEnabled(spray), sprayTimer(0.0f), sprayInterval(3.5f) {}

void Sniper::Update(float dt, std::vector<Bullet>& enemyBullets,
                    float playerX, float playerY) {
    // Slow horizontal drift, bounce off screen edges
    x += driftDir * driftSpeed * dt;
    if (x + SPRITE_SIZE > 750) driftDir = -1;
    if (x < 50)                  driftDir =  1;

    if (!aiming) {
        cooldownTimer += dt;
        if (cooldownTimer >= cooldown) {
            // Lock on to player's current position and start aiming
            aiming      = true;
            aimTimer    = 0.0f;
            aimTargetX  = playerX;
            aimTargetY  = playerY;
            cooldownTimer = 0.0f;
        }
    } else {
        aimTimer += dt;
        if (aimTimer >= aimDelay) {
            Fire(enemyBullets);
            aiming = false;
        }
    }

    // Hard: an unaimed spray runs independently of the aim/shoot cycle
    // above, giving Haruna a second, different threat instead of just a
    // tankier/faster version of the same single-shot pattern.
    if (sprayEnabled) {
        sprayTimer += dt;
        if (sprayTimer >= sprayInterval) {
            FireSpray(enemyBullets);
            sprayTimer = 0.0f;
        }
    }
}

void Sniper::Fire(std::vector<Bullet>& enemyBullets) {
    float cx = x + SPRITE_SIZE / 2.0f;
    float cy = y + SPRITE_SIZE / 2.0f;
    float dx = aimTargetX - cx;
    float dy = aimTargetY - cy;
    float dist = sqrtf(dx * dx + dy * dy);
    if (dist < 1.0f) dist = 1.0f;
    float dir = atan2f(dy, dx);
    // Fast single bullet, part of the sniper.
    enemyBullets.emplace_back(cx, cy, dir, 500.0f, BulletType::ENEMY, bulletTexture);
}

void Sniper::FireSpray(std::vector<Bullet>& enemyBullets) {
    float cx = x + SPRITE_SIZE / 2.0f;
    float cy = y + SPRITE_SIZE / 2.0f;

    // 3 unaimed bullets fanned downward, a threat the player has to
    // account for even while dodging the aimed shot, not just a copy of it.
    const int   bulletCount = 3;
    const float spreadRad   = 30.0f * (PI / 180.0f);
    for (int i = 0; i < bulletCount; i++) {
        float t   = (float)i / (bulletCount - 1) - 0.5f;
        float dir = (PI / 2.0f) + t * spreadRad;  // centered straight down
        enemyBullets.emplace_back(cx, cy, dir, 350.0f, BulletType::ENEMY, bulletTexture);
    }
}

void Sniper::Draw() const {
    Rectangle src  = { 0, 0, (float)texture.width, (float)texture.height };
    Rectangle dest = { x, y, SPRITE_SIZE, SPRITE_SIZE };
    DrawTexturePro(texture, src, dest, Vector2{0, 0}, 0.0f, WHITE);

    // Draw red aim line while aiming, which gives the player time to dodge
    if (aiming) {
        float cx = x + SPRITE_SIZE / 2.0f;
        float cy = y + SPRITE_SIZE / 2.0f;
        // Fade from transparent to solid as the shot gets closer
        unsigned char alpha = (unsigned char)(200 * (aimTimer / 1.2f));
        DrawLineEx(Vector2{cx, cy}, Vector2{aimTargetX, aimTargetY},
                   2.0f, Color{180, 0, 50, alpha});

    }
}

bool Sniper::IsActive()  const { return active; }
void Sniper::SetInactive()     { active = false; }

Rectangle Sniper::GetRect() const {
    return Rectangle{ x, y, SPRITE_SIZE, SPRITE_SIZE };
}
