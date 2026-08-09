#pragma once
#include "raylib.h"

enum class BulletType { FRIENDLY, ENEMY };

class Bullet {
public:
    Bullet(float x, float y, float direction, float speed,
           BulletType type, Texture2D tex);

    void Update(float dt);
    void Draw() const;
    bool IsOffscreen() const;
    bool IsActive() const;
    void SetInactive();
    Rectangle GetRect() const;

private:
    Texture2D texture;   // handle only, the game owns the GPU resources.
    float x, y;
    float direction;     // radians; 0 = right, -PI/2 = up, PI/2 = down
    float speed;         // pixels per second
    BulletType type;
    bool active;
};
