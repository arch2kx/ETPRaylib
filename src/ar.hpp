#pragma once
#include "raylib.h"
#include "bullet.hpp"
#include <vector>

class AR {
public:
    AR(float x, Texture2D tex, Texture2D bulletTex, int hp = 3, float shotCooldown = 2.5f);

    void Update(float dt, std::vector<Bullet>& enemyBullets, float playerX, float playerY);
    void Draw() const;
    bool IsActive() const;
    void SetInactive();
    void SetRage(bool on);  // Extreme+: sharply cuts cooldown while true
    Rectangle GetRect() const;

    int health;

private:
    Texture2D texture;
    Texture2D bulletTexture;
    float x, y;
    float driftSpeed;   // slow horizontal drift
    int   driftDir;

    float aimTimer;     // counts up while aiming
    float aimDelay;     // how long to show aim line before firing
    float cooldownTimer;
    float cooldown;     // time between shots

    bool  active;
    bool  aiming;
    float aimTargetX, aimTargetY;  // where we locked onto when aiming started
    bool  rageActive;   // true when this is the last unit alive and arRageEnabled

    void Fire(std::vector<Bullet>& enemyBullets);
};
