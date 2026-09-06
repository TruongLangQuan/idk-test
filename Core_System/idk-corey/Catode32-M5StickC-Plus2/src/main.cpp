// main.cpp - M5Catode entry point for M5StickC Plus2
#include <M5Unified.h>
#include "config.h"
#include "Renderer.h"
#include "Input.h"
#include "GameContext.h"
#include "SceneManager.h"
#include "assets/boot_img_assets.h"

// ── Global singletons ──────────────────────────────────────────────────────────
static Renderer     gRenderer;
static InputHandler gInput;
static GameContext  gContext;
static SceneManager* gSceneManager = nullptr;

// ── Boot screen ────────────────────────────────────────────────────────────────
static void showBootScreen() {
    gRenderer.clear(COLOR_BLACK);

    int bx = (DISPLAY_WIDTH  - STRETCH_CAT1.width  * SPRITE_SCALE) / 2;
    int by = (DISPLAY_HEIGHT - STRETCH_CAT1.height * SPRITE_SCALE) / 2 - 8;
    if (bx < 0) bx = 0;
    if (by < 0) by = 0;

    gRenderer.drawSpriteObj(&STRETCH_CAT1, bx, by, false, 0);

    gRenderer.drawText("M5Catode", (DISPLAY_WIDTH - 8*8)/2, DISPLAY_HEIGHT/2 + 28,
                       COLOR_UI_SELECT, COLOR_BLACK, 1);
    gRenderer.drawText("loading...", (DISPLAY_WIDTH - 10*6)/2, DISPLAY_HEIGHT/2 + 40,
                       COLOR_UI_DIM, COLOR_BLACK, 1);
    gRenderer.show();
    delay(1400);
}

// ── Arduino setup ──────────────────────────────────────────────────────────────
void setup() {
    auto cfg = M5.config();
    M5.begin(cfg);

    // Display orientation: landscape, USB connector on left
    // Display setup and double-buffer canvas are initialized in gRenderer.begin()
    gRenderer.begin();
    gContext.loadEnv();
    gContext.loadPetStats();
    gSceneManager = new SceneManager(&gContext, &gRenderer, &gInput);

    showBootScreen();

    gSceneManager->begin();
}

// ── Arduino loop ───────────────────────────────────────────────────────────────
void loop() {
    static uint32_t lastMs = 0;
    uint32_t now = millis();
    float dt = (now - lastMs) / 1000.0f;
    if (dt > 0.1f) dt = 0.1f;
    lastMs = now;

    M5.update();

    gInput.update();
    gSceneManager->update(dt);
    gSceneManager->draw();
    gRenderer.show();

    // Periodic pet stat save (every 60 s)
    static float sSaveTimer = 0.0f;
    sSaveTimer += dt;
    if (sSaveTimer >= 60.0f) { sSaveTimer = 0.0f; gContext.savePetStats(); }

    uint32_t elapsed = millis() - now;
    if (elapsed < (uint32_t)FRAME_TIME_MS) delay(FRAME_TIME_MS - elapsed);
}
