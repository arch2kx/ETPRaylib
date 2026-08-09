#include "bullet.hpp"
#include "visuals.hpp"
#include <cmath>

Bullet::Bullet(float x, float y, float direction, float speed,
               BulletType type, Texture2D tex)
    : x(x), y(y), direction(direction), speed(speed),
      type(type), texture(tex), active(true) {}

void Bullet::Update(float dt) {
    x += std::cos(direction) * speed * dt;
    y += std::sin(direction) * speed * dt;
}

void Bullet::Draw() const {
    Rectangle src  = { 0, 0, (float)texture.width, (float)texture.height };
    Rectangle dest = { x, y, BULLET_SIZE, BULLET_SIZE };
    DrawTexturePro(texture, src, dest, Vector2{0, 0}, 0.0f, WHITE);
}

bool Bullet::IsOffscreen() const {
    return y > 600 || y + BULLET_SIZE < 0 ||
           x > 800 || x + BULLET_SIZE < 0;
}

bool Bullet::IsActive() const  { return active; }
void Bullet::SetInactive()     { active = false; }

Rectangle Bullet::GetRect() const {
    return Rectangle{ x, y, BULLET_SIZE, BULLET_SIZE };
}
