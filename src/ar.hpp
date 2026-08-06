#pragma once
#include "raylib.h"
#include "bullet.hpp"
#include <vector>

class AR {
public:
    AR(float x, Texture2D tex, Texture2D bulletTex);

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

    void Fire(std::vector<Bullet>& enemyBullets);
};
