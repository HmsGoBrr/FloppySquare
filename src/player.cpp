#include "Player.h"

void Player::init() {
    alive = true;
    velocityY = 0;
    pos.x = 80;
    pos.y = GetScreenHeight()/2.0f - tex.height/2.0f * scale;
}

void Player::update(float dt) {
    velocityY += gravity;

    if (IsKeyPressed(KEY_SPACE) || IsMouseButtonPressed(0)) {
        velocityY = -200.0f;
        PlaySound(jumpSfx);
    }

    pos.y += velocityY * dt;
}

void Player::draw() {
    DrawTextureEx(tex, pos, 0.0f, scale, alive ? WHITE : RED);
}

void Player::unload() {
    UnloadTexture(tex);
    UnloadSound(jumpSfx);
}
