#ifndef PLAYER_H
#define PLAYER_H

#include "raylib.h"

class Player {
public:
    Player(): tex(LoadTexture("assets/player.png")), jumpSfx(LoadSound("assets/jump.wav")) {}

    void init();
    void update(float dt);
    void draw();
    void unload();

    Vector2& getPos() { return pos; }
    Rectangle getCollisionRec() { return { pos.x, pos.y, tex.width * scale, tex.height * scale }; }

    float getWidth() { return tex.width * scale; }
    float getHeight() { return tex.height * scale; }

    bool isAlive() { return alive; }
    void setAlive(bool isAlive) { alive = isAlive; }
private:
    Texture2D tex;
    Sound jumpSfx;
    Vector2 pos{};
    float velocityY;
    bool alive;
    const float scale{5.0f};
    const float gravity{10.0f};
};

#endif