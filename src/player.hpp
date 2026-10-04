#pragma once
#include "raylib.h"
#include "bullet.hpp"
#include "input_state.hpp"
#include <vector>

class Player {
public:
    Player();
    Player(Texture2D tex, Texture2D bulletTex, Texture2D healthTex, float speedMult = 1.0f, int bulletCount = 1);

    void Update(float dt, const InputState& in, std::vector<Bullet>& playerBullets);
    void Draw() const;
    void DrawHealthBar() const;
    Rectangle GetRect() const;
    void TakeDamage(int amount);
    bool IsDead() const;

    int health;
    int maxHealth;

private:
    Texture2D texture;
    Texture2D bulletTexture;
    Texture2D healthBarTex;
    float x, y;
    float speed;
    float shootTimer;
    float shootCooldown;
    int   bulletCount; // Number of bullets fired per shot, fanned out if >1 (from DifficultySettings)
};
