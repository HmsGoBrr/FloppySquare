#include "raylib.h"
#include <math.h>
#include "Player.h"

#if defined(PLATFORM_WEB)
    #include <emscripten/emscripten.h>
    #define powf32 powf
#endif

const int screenWidth = 800;
const int screenHeight = 450;

int volume;

struct Tube {
    Rectangle rec;
    bool active;
};

const int maxTubes = 4;
Tube tubes[maxTubes*2];
Vector2 tubesPos[maxTubes];
float tubeSpeed;

int score;
bool pause;
bool gameStart;

bool scoreBlink;

Camera2D camera;

float shake;

Texture2D playTex;
Texture2D retryTex;
Texture2D pauseTex;
Sound scoreSfx;
Sound selectSfx;
Sound boomSfx;

void init(Player* plr) {
    pause = false;

    scoreBlink = false;

    camera.target = { screenWidth/2.0f, screenHeight/2.0f };
    camera.offset = { screenWidth/2.0f, screenHeight/2.0f };
    camera.rotation = 0.0f;
    camera.zoom = 1.0f;

    shake = 0;

    score = 0;
    tubeSpeed = 1;

    for (int i = 0; i < maxTubes; i++) {
        tubesPos[i].x = screenWidth/2 + 280 * i;
        tubesPos[i].y = -GetRandomValue(0, 120);
    }
    for (int i = 0; i < maxTubes*2; i += 2) {
        tubes[i].rec.x = tubesPos[i/2].x;
        tubes[i].rec.y = tubesPos[i/2].y;
        tubes[i].rec.width = 80;
        tubes[i].rec.height = 255;

        tubes[i+1].rec.x = tubesPos[i/2].x;
        tubes[i+1].rec.y = 615 + tubesPos[i/2].y - 255;
        tubes[i+1].rec.width = 80;
        tubes[i+1].rec.height = 255;

        tubes[i/2].active = true;    
    }

    plr->init();
}

void update(Player* plr, float dt) {
    if (shake != 0) {
        shake -= dt;

        if (shake < 0) shake = 0;

        camera.rotation = 30.0f * powf32(shake, 2.0f) * (GetRandomValue(-100, 100) / 100.0f);
        camera.offset.x = screenWidth/2.0f + 50.0f * powf32(shake, 2.0f) * (GetRandomValue(-100, 100) / 100.0f);
        camera.offset.y = screenHeight/2.0f + 50.0f * powf32(shake, 2.0f) * (GetRandomValue(-100, 100) / 100.0f);
    } else {
        camera.rotation = 0.0f;
        camera.offset.x = screenWidth/2.0f;
        camera.offset.y = screenHeight/2.0f;
    }

    if (!plr->isAlive()) {
        if (IsKeyPressed(KEY_R)) {
            init(plr);
            PlaySound(selectSfx);
        }
        return;
    }
    if (pause) {
        if (IsKeyPressed(KEY_P)) {
            pause = false;
            PlaySound(selectSfx);
        }
        if (IsKeyPressed(KEY_R)) {
            init(plr);
            PlaySound(selectSfx);
        }
        return;
    }
    if (IsKeyPressed(KEY_P)) pause = true;

    plr->update(dt);

    tubeSpeed += dt/10;

    for (int i = 0; i < maxTubes; i++) {
        tubesPos[i].x -= tubeSpeed;
        if (tubesPos[i].x < -80) {
            tubesPos[i].x = screenWidth + 280;
            tubesPos[i].y = -GetRandomValue(0, 120);
            tubes[i*2].active = true;
        }
    }

    for (int i = 0; i < maxTubes*2; i += 2) {
        tubes[i].rec.x = tubesPos[i/2].x;
        tubes[i].rec.y = tubesPos[i/2].y;
        tubes[i+1].rec.x = tubesPos[i/2].x;
        tubes[i+1].rec.y = 615 + tubesPos[i/2].y - 255;
    }

    for (int i = 0; i < maxTubes*2; i++) {
        if (CheckCollisionRecs(plr->getCollisionRec(), tubes[i].rec) ||
            (plr->getPos().y > (screenHeight - plr->getHeight())) ||
            (plr->getPos().y < 0)) {
            plr->setAlive(false);
            pause = false;
            shake = 0.5f;
            PlaySound(boomSfx);
        }else if (((tubesPos[i/2].x+40) < (plr->getPos().x + plr->getWidth()/2)) && tubes[i/2].active) {
            score++;
            tubes[i/2].active = false;
            PlaySound(scoreSfx);
            scoreBlink = true;
        }
    }    
}

void volumeBar() {
    if (IsKeyPressed(KEY_LEFT) && volume > 0) volume--;
    else if (IsKeyPressed(KEY_RIGHT) && volume < 10) volume++;
    SetMasterVolume(volume/10.0f);

    for (int i = 0; i < 10; i++) {
        if (i < volume) DrawRectangle(295 + 21*i, 400, 20, 20, DARKGRAY);
        DrawRectangleLines(295 + 21*i, 400, 20, 20, BLACK);
    }

    DrawText("[LEFT]          VOLUME          [RIGHT]",
        screenWidth/2.0f - MeasureText("[LEFT]          VOL", 10), 430,
        10, RAYWHITE
    );
}

void draw(Player* plr, float dt) {
    BeginDrawing();
    ClearBackground(DARKGRAY);

    BeginMode2D(camera);

    plr->draw();
    
    for (int i = 0; i < maxTubes; i++) {
        DrawRectangle(
            tubes[i*2].rec.x, tubes[i*2].rec.y,
            tubes[i*2].rec.width, tubes[i*2].rec.height,
            GRAY
        );
        DrawRectangle(
            tubes[i*2 + 1].rec.x, tubes[i*2 + 1].rec.y,
            tubes[i*2 + 1].rec.width, tubes[i*2 + 1].rec.height,
            GRAY
        );
    }

    EndMode2D();

    DrawText(TextFormat("Score: %i", score), 5, 5, 20, RAYWHITE);

    if (!plr->isAlive()) {
        DrawRectangle(0, 0, screenWidth, screenHeight, { 130, 130, 130, 130 });
        DrawText("Yuo ded", screenWidth/2 - MeasureText("Yuo ded", 50)/2, 150, 50, RAYWHITE);
        DrawText(TextFormat("Score: %i", score),
            screenWidth/2.0f - MeasureText(TextFormat("Score: %i", score), 20)/2.0f,
            195, 20, RAYWHITE
        );
        DrawTextureEx(
            retryTex,
            { screenWidth/2.0f - retryTex.width * 2.5f, screenHeight/2.0f - retryTex.height*2.5f + 50 },
            0, 5.0f, WHITE
        );
        DrawText("[R]",
            screenWidth/2.0f - MeasureText("[R]", 15)/2.0f,
            screenHeight/2.0f - retryTex.height*2.5f + 100,
            15, RAYWHITE
        );
    }

    if (pause) {
        DrawRectangle(0, 0, screenWidth, screenHeight, { 130, 130, 130, 130 });
        DrawText("Paused", screenWidth/2 - MeasureText("Paused", 50)/2, 150, 50, RAYWHITE);
        DrawTextureEx(
            playTex,
            {(screenWidth/2.0f - playTex.width*5.0f)-50, screenHeight/2.0f - playTex.height*2.5f + 50},
            0, 5, WHITE
        );
        DrawTextureEx(
            retryTex,
            { screenWidth/2.0f + 50, screenHeight/2.0f - retryTex.height*2.5f + 50 },
            0, 5.0f, WHITE
        );
        DrawText("[P]",
            ((screenWidth/2.0f - playTex.width * 2.5f) - 50) - MeasureText("[P]", 15)/2.0f,
            screenHeight/2.0f - playTex.height*2.5f + 100,
            15, RAYWHITE
        );
        DrawText("[R]",
            screenWidth/2.0f + retryTex.width * 2.5f + 50 - MeasureText("[R]", 15)/2.0f,
            screenHeight/2.0f - retryTex.height*2.5f + 100,
            15, RAYWHITE
        );

        volumeBar();
    } else {
        DrawTextureEx(
            pauseTex,
            {(screenWidth - pauseTex.width*5.0f) - 5, 5},
            0, 5, WHITE
        );
        DrawText("[P]",
            (screenWidth - pauseTex.width*2.5f) - 5 - MeasureText("[P]", 15)/2.0f,
            pauseTex.height*5.0f + 10,
            15, RAYWHITE
        );
    }

    EndDrawing();
}

void mainMenu() {
    if (IsKeyPressed(KEY_ENTER)) {
        gameStart = true;
        PlaySound(selectSfx);
    }

    BeginDrawing();
    ClearBackground(DARKGRAY);

    DrawRectangle(0, 0, screenWidth, screenHeight, { 130, 130, 130, 130 });
    DrawText("Floppy Square", screenWidth/2 - MeasureText("Floppy Square", 50)/2, 150, 50, RAYWHITE);
    DrawTextureEx(
        playTex,
        { screenWidth/2.0f - playTex.width * 2.5f, screenHeight/2.0f - playTex.height*2.5f + 50 },
        0, 5.0f, WHITE
    );
    DrawText("[ENTER]",
        screenWidth/2.0f - MeasureText("[ENTER]", 15)/2.0f,
        screenHeight/2.0f - playTex.height*2.5f + 100,
        15, RAYWHITE
    );

    volumeBar();

    EndDrawing();
}

void gameLoop(Player* plr) {
    if (!gameStart) {
        mainMenu();
        return;
    }

    float dt = GetFrameTime();
    
    update(plr, dt);
    draw(plr, dt);
}

int main(void) {
    InitWindow(screenWidth, screenHeight, "Raylib Small Project");

    InitAudioDevice();

    volume = 10;

    Player plr;

    playTex = LoadTexture("assets/play.png");
    retryTex = LoadTexture("assets/retry.png");
    pauseTex = LoadTexture("assets/pause.png");

    scoreSfx = LoadSound("assets/score.wav");
    selectSfx = LoadSound("assets/select.wav");
    boomSfx = LoadSound("assets/boom.wav");

    gameStart = false;

    init(&plr);

#if defined(PLATFORM_WEB)
    emscripten_set_main_loop_arg((em_arg_callback_func)gameLoop, &plr, 60, 1);
#else
    SetTargetFPS(60);

    SetExitKey(0);

    while (!WindowShouldClose()) {
        gameLoop(&plr);
    }
#endif

    plr.unload();

    UnloadTexture(playTex);
    UnloadTexture(retryTex);
    UnloadTexture(pauseTex);
    UnloadSound(scoreSfx);
    UnloadSound(selectSfx);
    UnloadSound(boomSfx);
    CloseAudioDevice();
    CloseWindow();

    return 0;
}