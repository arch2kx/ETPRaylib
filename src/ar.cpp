#include "ar.hpp"
#include <cmath>

AR::AR(float x, Texture2D tex, Texture2D bulletTex)
    : texture(tex), bulletTexture(bulletTex),
      x(x), y(60.0f),
      health(3),
      driftSpeed(40.0f), driftDir(1),
      aimTimer(0.0f), aimDelay(1.2f),
      cooldownTimer(0.0f), cooldown(2.5f),
      active(true), aiming(false),
      aimTargetX(0.0f), aimTargetY(0.0f) {}

void AR::Update(float dt, std::vector<Bullet>& enemyBullets,
                    float playerX, float playerY) {
    // Slow horizontal drift, bounce off screen edges
    x += driftDir * driftSpeed * dt;
    if (x + texture.width > 750) driftDir = -1;
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
}

void AR::Fire(std::vector<Bullet>& enemyBullets) {
    float cx = x + texture.width  / 2.0f;
    float cy = y + texture.height / 2.0f;
    float dx = aimTargetX - cx;
    float dy = aimTargetY - cy;
    float dist = sqrtf(dx * dx + dy * dy);
    if (dist < 1.0f) dist = 1.0f;
    float dir = atan2f(dy, dx);
    // Fast single bullet — the AR's signature
    enemyBullets.emplace_back(cx, cy, dir, 500.0f, BulletType::ENEMY, bulletTexture);
}

void AR::Draw() const {
    DrawTexture(texture, (int)x, (int)y, WHITE);

    // Draw red aim line while aiming — gives the player time to dodge
    if (aiming) {
        float cx = x + texture.width  / 2.0f;
        float cy = y + texture.height / 2.0f;
        // Fade from transparent to solid as the shot gets closer
        unsigned char alpha = (unsigned char)(200 * (aimTimer / 1.2f));
        DrawLineEx(Vector2{cx, cy}, Vector2{aimTargetX, aimTargetY},
                   2.0f, Color{255, 50, 50, alpha});
    }
}

bool AR::IsActive()  const { return active; }
void AR::SetInactive()     { active = false; }

Rectangle AR::GetRect() const {
    return Rectangle{ x, y, (float)texture.width, (float)texture.height };
}
