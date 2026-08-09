#pragma once
#include "raylib.h"
#include "bullet.hpp"
#include "difficulty.hpp"
#include <vector>

class Boss {
public:
    Boss(Texture2D tex, Texture2D bulletTex, const DifficultySettings& diff);

    void Update(float dt, std::vector<Bullet>& enemyBullets,
                float playerX, float playerY);
    void Draw() const;
    void DrawHealthBar() const;
    bool IsDead() const;
    Rectangle GetRect() const;

    int health;
    int maxHealth;

private:
    Texture2D texture;
    Texture2D bulletTexture;
    float x, y;
    float speed;
    int   direction;
    float shootCooldown;
    float shootTimer;
    float bulletSpeed;
    bool  aimedShot;
    bool  burst;
    float burstTimer;
    bool  spiralEnabled;
    float spiralAngle;  // rotating start-offset for the ring pattern, degrees

    void Fire(std::vector<Bullet>& enemyBullets,
              float playerX, float playerY);
    void FireSpiral(std::vector<Bullet>& enemyBullets);
};
