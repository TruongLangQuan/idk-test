#include <M5Unified.h>
#include <math.h>
#include <algorithm>
#include "engine/ASCIIRenderer.h"
#include "styles/StyleManager.h"
#include "world/WorldGenerator.h"

using namespace idk;

// ─── Constants & Pins ──────────────────────────────────────────
#if defined(STICKS3)
static constexpr int kPinUp     = 1;
static constexpr int kPinDown   = 2;
static constexpr int kPinLeft   = 3;
static constexpr int kPinRight  = 8;
static constexpr int kPinCenter = 43;
#elif defined(PCBFUN)
static constexpr int kPinUp     = 1;
static constexpr int kPinDown   = 2;
static constexpr int kPinLeft   = 3;
static constexpr int kPinRight  = 4;
static constexpr int kPinCenter = 5;
#else
static constexpr int kPinUp     = 32;
static constexpr int kPinDown   = 33;
static constexpr int kPinLeft   = 25;
static constexpr int kPinRight  = 26;
static constexpr int kPinCenter = 0;
#endif

// ─── Global State ──────────────────────────────────────────────
ASCIIRenderer renderer;
WorldGenerator world;
int currentStyleIdx = 0;

float camX = 0, camZ = 0;
float camY = 10.0f;
float yaw = 0;

bool readPressed(int pin) {
    return digitalRead(pin) == LOW;
}

// ─── Input Handling ────────────────────────────────────────────
void handleInput() {
    static uint32_t lastMove = 0;
    if (millis() - lastMove < 33) return; // ~30Hz input polling
    lastMove = millis();

    const float moveSpeed = 0.45f;
    const float turnSpeed = 0.08f;

    if (readPressed(kPinUp)) {
        camX += cosf(yaw) * moveSpeed;
        camZ += sinf(yaw) * moveSpeed;
    }
    if (readPressed(kPinDown)) {
        camX -= cosf(yaw) * moveSpeed;
        camZ -= sinf(yaw) * moveSpeed;
    }
    if (readPressed(kPinLeft)) {
        yaw += turnSpeed;
    }
    if (readPressed(kPinRight)) {
        yaw -= turnSpeed;
    }

    static bool centerPressed = false;
    if (readPressed(kPinCenter)) {
        if (!centerPressed) {
            currentStyleIdx = (currentStyleIdx + 1) % kStyleCount;
            centerPressed = true;
        }
    } else {
        centerPressed = false;
    }

    if (M5.BtnA.isPressed()) camY += 0.2f;
    if (M5.BtnB.isPressed()) camY -= 0.2f;
    if (camY < 2.0f) camY = 2.0f;
    if (camY > 24.0f) camY = 24.0f;
}

// ─── Main Logic ───────────────────────────────────────────────
void setup() {
    pinMode(14, OUTPUT);
    digitalWrite(14, HIGH);
    delay(100);

    auto cfg = M5.config();
    cfg.internal_mic = false; // Disable MIC (G0 conflicts with SD SCK)
    cfg.internal_spk = false; // Disable SPK
    M5.begin(cfg);
#if defined(STICKS3)
    M5.Display.setRotation(1);
#else
    M5.Display.setRotation(3);
#endif
    
    pinMode(kPinUp, INPUT_PULLUP);
    pinMode(kPinDown, INPUT_PULLUP);
    pinMode(kPinLeft, INPUT_PULLUP);
    pinMode(kPinRight, INPUT_PULLUP);
    pinMode(kPinCenter, INPUT_PULLUP);

    renderer.begin();
    world.randomize();
}

void loop() {
    M5.update();
    handleInput();

    // ─── FPP Raycasting Engine (Graphical Doom Style) ───────────
    static constexpr float kFOV = 1.047f; // 60 deg
    static constexpr float kMaxDist = 30.0f;
    static constexpr float kStep = 0.2f;
    static constexpr int kScreenW = 240;
    static constexpr int kScreenH = 135;
    static constexpr int kHalfH = 67;

    M5.Display.startWrite();
    
    // Draw Ceiling and Floor
    M5.Display.fillRect(0, 0, kScreenW, kHalfH, M5.Display.color565(20, 20, 20));
    M5.Display.fillRect(0, kHalfH, kScreenW, kHalfH, M5.Display.color565(40, 40, 40));

    for (int x = 0; x < kScreenW; ++x) {
        float rayAngle = (yaw - kFOV/2.0f) + (float)x / kScreenW * kFOV;
        float rx = cosf(rayAngle);
        float rz = sinf(rayAngle);
        
        bool hit = false;
        float dist = 0.1f;
        
        while (!hit && dist < kMaxDist) {
            dist += kStep;
            float wx = camX + rx * dist;
            float wz = camZ + rz * dist;
            
            float h = world.getHeight(wx, wz);
            if (h > camY * 0.22f) hit = true; 
        }

        // Draw Vertical Slice
        int lineHeight = (int)(kScreenH / (dist * 0.3f));
        int drawStart = -lineHeight / 2 + kHalfH;
        if (drawStart < 0) drawStart = 0;
        int drawEnd = lineHeight / 2 + kHalfH;
        if (drawEnd >= kScreenH) drawEnd = kScreenH - 1;
        
        if (hit) {
            // Distance fading for Doom aesthetic
            int fade = std::max(0, 255 - (int)(dist * 8));
            uint16_t color = M5.Display.color565(fade/2, fade/2, fade/3);
            M5.Display.drawFastVLine(x, drawStart, drawEnd - drawStart, color);
        }
    }

    M5.Display.endWrite();
    
    M5.Display.setTextColor(TFT_YELLOW, M5.Display.color565(20, 20, 20));
    M5.Display.setCursor(2, 2);
    M5.Display.printf("HP:--  POS:%.1f,%.1f Y:%.0f", camX, camZ, camY);
    M5.Display.setTextColor(TFT_LIGHTGREY, TFT_BLACK);
    M5.Display.setCursor(4, 120);
    M5.Display.print("5way:move/turn A/B:height C:style");

    const int mx = 205;
    const int my = 8;
    M5.Display.drawRect(mx - 1, my - 1, 30, 30, TFT_DARKGREY);
    M5.Display.fillRect(mx + 14, my + 14, 2, 2, TFT_GREEN);
    M5.Display.drawLine(mx + 15, my + 15, mx + 15 + (int)(cosf(yaw) * 10), my + 15 + (int)(sinf(yaw) * 10), TFT_YELLOW);
}
