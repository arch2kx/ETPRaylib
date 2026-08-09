#pragma once
#include "raylib.h"
#include "bullet.hpp"
#include <vector>

class Sniper {
public:
    Sniper(float x, Texture2D tex, Texture2D bulletTex, int hp = 3, float shotCooldown = 2.5f, bool spray = false);

    void Update(float dt, std::vector<Bullet>& enemyBullets, float playerX, float playerY);
    void Draw() const;
    bool IsActive() const;
    void SetInactive();
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

    bool  sprayEnabled;  // Hard+: fires an unaimed 3-round spray between aimed shots
    float sprayTimer;
    float sprayInterval;

    void Fire(std::vector<Bullet>& enemyBullets);
    void FireSpray(std::vector<Bullet>& enemyBullets);
};
