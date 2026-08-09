#include "player.hpp"
#include "visuals.hpp"

Player::Player()
    : texture{}, bulletTexture{}, healthBarTex{},
      x(0.0f), y(520.0f), speed(360.0f),
      shootTimer(0.0f), shootCooldown(0.2f), bulletCount(1),
      health(100), maxHealth(100) {}

Player::Player(Texture2D tex, Texture2D bulletTex, Texture2D healthTex, float speedMult, int bulletCount)
    : texture(tex), bulletTexture(bulletTex), healthBarTex(healthTex),
    x(400.0f - SPRITE_SIZE / 2.0f), y(520.0f),
    speed(360.0f * speedMult), shootTimer(0.0f), shootCooldown(0.2f), bulletCount(bulletCount),
    health(100), maxHealth(100) {}

void Player::Update(float dt, std::vector<Bullet>& playerBullets) {
    if (IsKeyDown(KEY_LEFT)  && x > 0)                         x -= speed * dt;
    if (IsKeyDown(KEY_RIGHT) && x + SPRITE_SIZE < 800)         x += speed * dt;
    if (IsKeyDown(KEY_UP)    && y > 0)                         y -= speed * dt;
    if (IsKeyDown(KEY_DOWN)  && y + SPRITE_SIZE < 600)         y += speed * dt;

    shootTimer += dt;
    if (IsKeyDown(KEY_SPACE) && shootTimer >= shootCooldown) {
        float bx = x + SPRITE_SIZE / 2.0f - bulletTexture.width  / 2.0f;
        float by = y;
        if (bulletCount > 1) {
            // Fan bulletCount shots evenly across a fixed cone width, wider
            // fans for more bullets so density stays roughly consistent.
            const float spreadRad = (6.0f * bulletCount) * (PI / 180.0f);
            for (int i = 0; i < bulletCount; i++) {
                float t   = (float)i / (bulletCount - 1) - 0.5f;  // -0.5..0.5 across the fan
                float dir = -(PI / 2.0f) + t * spreadRad;         // centered straight up
                playerBullets.emplace_back(bx, by, dir, 600.0f,
                                           BulletType::FRIENDLY, bulletTexture);
            }
        } else {
            playerBullets.emplace_back(bx, by, -(PI / 2.0f), 600.0f,
                                       BulletType::FRIENDLY, bulletTexture);
        }
        shootTimer = 0.0f;
    }
}

void Player::Draw() const {
    Rectangle src  = { 0, 0, (float)texture.width, (float)texture.height };
    Rectangle dest = { x, y, SPRITE_SIZE, SPRITE_SIZE };
    DrawTexturePro(texture, src, dest, Vector2{0, 0}, 0.0f, WHITE);
}

void Player::DrawHealthBar() const {
    // Discrete icon-count health display using the bow sprite: every damage
    // source in the game deals exactly 25 flat damage, so at maxHealth=100
    // that's always exactly 4 hits to die, one icon lost per hit taken.
    const int   iconSize = 45;
    const int   gap      = 8;
    const int   bx       = 15;
    const int   by       = 545;
    int icons = health / 25;

    Rectangle src = { 0, 0, (float)healthBarTex.width, (float)healthBarTex.height };
    for (int i = 0; i < icons; i++) {
        float ix = bx + i * (iconSize + gap);
        DrawTexturePro(healthBarTex, src,
            Rectangle{ ix, (float)by, (float)iconSize, (float)iconSize },
            Vector2{0, 0}, 0.0f, WHITE);
    }
}

Rectangle Player::GetRect() const {
    return Rectangle{ x, y, SPRITE_SIZE, SPRITE_SIZE };
}

void Player::TakeDamage(int amount) {
    health -= amount;
    if (health < 0) health = 0;
}

bool Player::IsDead() const { return health <= 0; }
