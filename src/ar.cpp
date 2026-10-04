#include "ar.hpp"
#include "visuals.hpp"
#include "det.hpp"
#include <cmath>

AR::AR(float x, Texture2D tex, Texture2D bulletTex, int hp, float shotCooldown)
    : texture(tex), bulletTexture(bulletTex),
      x(x), y(60.0f),
      health(hp),
      driftSpeed(40.0f), driftDir(1),
      aimTimer(0.0f), aimDelay(1.2f),
      cooldownTimer(0.0f), cooldown(shotCooldown),
      active(true), aiming(false),
      aimTargetX(0.0f), aimTargetY(0.0f), rageActive(false) {}

void AR::Update(float dt, std::vector<Bullet>& enemyBullets,
                    float playerX, float playerY) {
    // Slow horizontal drift, bounce off screen edges
    x += driftDir * driftSpeed * dt;
    if (x + SPRITE_SIZE > 750) driftDir = -1;
    if (x < 50)                  driftDir =  1;

    if (!aiming) {
        cooldownTimer += dt;
        // Rage: last unit standing fires much more often than its base cooldown
        float effectiveCooldown = rageActive ? cooldown * 0.35f : cooldown;
        if (cooldownTimer >= effectiveCooldown) {
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
    float cx = x + SPRITE_SIZE / 2.0f;
    float cy = y + SPRITE_SIZE / 2.0f;
    float dx = aimTargetX - cx;
    float dy = aimTargetY - cy;
    float dist = sqrtf(dx * dx + dy * dy);
    if (dist < 1.0f) dist = 1.0f;
    float dir = det::Atan2(dy, dx);

    // AR volley: 5 bullets fanned out around the aimed direction, not one
    // shot, part of the assault rifle's abilities.
    const int   bulletCount = 5;
    const float spreadRad   = 20.0f * (PI / 180.0f);  // total fan width, in radians
    for (int i = 0; i < bulletCount; i++) {
        float t = (bulletCount == 1) ? 0.0f
                : (float)i / (bulletCount - 1) - 0.5f;  // -0.5..0.5 across the fan
        float shotDir = dir + t * spreadRad;
        enemyBullets.emplace_back(cx, cy, shotDir, 500.0f, BulletType::ENEMY, bulletTexture);
    }
}

void AR::Draw() const {
    Rectangle src  = { 0, 0, (float)texture.width, (float)texture.height };
    Rectangle dest = { x, y, SPRITE_SIZE, SPRITE_SIZE };
    DrawTexturePro(texture, src, dest, Vector2{0, 0}, 0.0f, WHITE);

    // Draw red aim line while aiming, gives the player time to dodge
    if (aiming) {
        float cx = x + SPRITE_SIZE / 2.0f;
        float cy = y + SPRITE_SIZE / 2.0f;
        // Fade from transparent to solid as the shot gets closer
        unsigned char alpha = (unsigned char)(200 * (aimTimer / 1.2f));
        DrawLineEx(Vector2{cx, cy}, Vector2{aimTargetX, aimTargetY},
                   2.0f, Color{180, 0, 50, alpha});
    }
}

bool AR::IsActive()  const { return active; }
void AR::SetInactive()     { active = false; }
void AR::SetRage(bool on)  { rageActive = on; }

Rectangle AR::GetRect() const {
    return Rectangle{ x, y, SPRITE_SIZE, SPRITE_SIZE };
}
