#include <M5Unified.h>
#include <math.h>

namespace {

// ─── ASCII Modes ───────────────────────────────────────────────
enum AsciiMode {
  DETAILED,
  STANDARD,
  MINIMAL,
  BLOCK,
  DOTS,
  MODE_COUNT,
};

const char* kModeNames[] = {"detailed", "standard", "minimal", "block", "dots"};
const char* kDetailedChars = "▓▒░█■◆●○◉◎◈∎▰▱▲△▼▽";
const char* kStandardChars = "#@%*+-=~`!*|.+-/\\O";
const char* kMinimalChars = "#.-|+/\\O";
const char* kBlockChars = "#□■▪▫█░";
const char* kDotsChars = ".:;'\"^v<>";

// ─── 5-way tactile switch GPIO mapping ──────────────────────────
static constexpr int kPinUp = 32;
static constexpr int kPinDown = 33;
static constexpr int kPinLeft = 25;
static constexpr int kPinRight = 26;
static constexpr int kPinCenter = 0;

struct Button {
  int pin;
  bool pressed;
  uint32_t lastRepeatMs;
};

static Button g_buttons[] = {
    {kPinUp, false, 0},
    {kPinDown, false, 0},
    {kPinLeft, false, 0},
    {kPinRight, false, 0},
    {kPinCenter, false, 0},
};

static constexpr uint32_t kRepeatMs = 150;

// ─── State ─────────────────────────────────────────────────────
static AsciiMode g_mode = STANDARD;
static uint32_t g_frame = 0;
static int g_speed = 1;  // 1-5 animation speed
static bool g_paused = false;
static uint32_t g_last_update = 0;

// ─── Helpers ───────────────────────────────────────────────────

bool readPressed(int pin) {
  return digitalRead(pin) == LOW;
}

char getAsciiForDensity(float value, AsciiMode mode) {
  // value: 0.0 - 1.0 (brightness/density)
  const char* charset;
  int charCount;

  switch (mode) {
    case DETAILED:
      charset = kDetailedChars;
      charCount = strlen(charset);
      break;
    case STANDARD:
      charset = kStandardChars;
      charCount = strlen(charset);
      break;
    case MINIMAL:
      charset = kMinimalChars;
      charCount = strlen(charset);
      break;
    case BLOCK:
      charset = kBlockChars;
      charCount = strlen(charset);
      break;
    case DOTS:
      charset = kDotsChars;
      charCount = strlen(charset);
      break;
    default:
      charset = kStandardChars;
      charCount = strlen(charset);
  }

  int idx = (int)(value * (charCount - 1));
  idx = std::max(0, std::min(charCount - 1, idx));
  return charset[idx];
}

void drawAsciiScene() {
  M5.Display.fillScreen(TFT_BLACK);
  M5.Display.setTextSize(1);
  M5.Display.setTextColor(TFT_GREEN, TFT_BLACK);

  const int W = 30;
  const int H = 8;
  const float time = g_frame * 0.1f;

  for (int y = 0; y < H; ++y) {
    M5.Display.setCursor(4, 20 + y * 12);
    for (int x = 0; x < W; ++x) {
      float px = (x - W / 2.0f) * 0.1f;
      float py = (y - H / 2.0f) * 0.1f;

      // 3D rotation: spin around multiple axes
      float angleX = time * 0.5f;
      float angleY = time * 0.3f;
      float angleZ = time * 0.2f;

      // Simple 3D projection
      float rx = px;
      float ry = py * cosf(angleX) - 0.5f * sinf(angleX);
      float rz = py * sinf(angleX) + 0.5f * cosf(angleX);

      float sx = rx * cosf(angleY) + rz * sinf(angleY);
      float sz = -rx * sinf(angleY) + rz * cosf(angleY);

      float tx = sx * cosf(angleZ) - ry * sinf(angleZ);
      float ty = sx * sinf(angleZ) + ry * cosf(angleZ);

      // Compute depth-based density
      float depth = 2.0f + sz;
      float density = (sinf(tx * 5.0f) * cosf(ty * 5.0f) + 1.0f) * 0.5f;
      density *= std::max(0.0f, std::min(1.0f, depth / 3.0f));

      char c = getAsciiForDensity(density, g_mode);
      M5.Display.print(c);
    }
  }

  // Draw mode and speed info
  M5.Display.setTextColor(TFT_YELLOW, TFT_BLACK);
  M5.Display.setCursor(4, 4);
  M5.Display.printf("%s [%d/5] %s", kModeNames[g_mode], g_speed, g_paused ? "PAUSED" : "");
}

void handleInput() {
  const uint32_t now = millis();

  auto handleDir = [&](Button& btn, int action) {
    const bool down = readPressed(btn.pin);
    if (!down) {
      btn.pressed = false;
      return;
    }

    bool trigger = false;
    if (!btn.pressed) {
      btn.pressed = true;
      btn.lastRepeatMs = now;
      trigger = true;
    } else if (now - btn.lastRepeatMs > kRepeatMs) {
      btn.lastRepeatMs = now;
      trigger = true;
    }

    if (trigger) {
      if (action == 0) {  // UP - speed up
        if (g_speed < 5) g_speed++;
      } else if (action == 1) {  // DOWN - speed down
        if (g_speed > 1) g_speed--;
      } else if (action == 2) {  // LEFT - prev mode
        g_mode = (AsciiMode)((g_mode - 1 + MODE_COUNT) % MODE_COUNT);
      } else if (action == 3) {  // RIGHT - next mode
        g_mode = (AsciiMode)((g_mode + 1) % MODE_COUNT);
      }
    }
  };

  handleDir(g_buttons[0], 0);  // UP
  handleDir(g_buttons[1], 1);  // DOWN
  handleDir(g_buttons[2], 2);  // LEFT
  handleDir(g_buttons[3], 3);  // RIGHT

  // CENTER - pause/resume
  if (readPressed(kPinCenter)) {
    static uint32_t lastCenter = 0;
    if (now - lastCenter > 300) {
      g_paused = !g_paused;
      lastCenter = now;
    }
  }

  // PWR - reset
  if (M5.BtnPWR.wasPressed()) {
    g_frame = 0;
    g_speed = 1;
    g_mode = STANDARD;
    g_paused = false;
  }
}

}  // namespace

void setup() {
  auto cfg = M5.config();
  M5.begin(cfg);
  M5.Display.setRotation(3);
  M5.Display.setBrightness(180);

  Serial.begin(115200);
  delay(200);
  Serial.println("idk-motion-ascii starting...");

  // Initialize 5-way pins
  for (auto& btn : g_buttons) {
    pinMode(btn.pin, INPUT_PULLUP);
  }

  g_last_update = millis();
}

void loop() {
  M5.update();
  handleInput();

  // Update animation based on speed
  uint32_t now = millis();
  if (!g_paused && (now - g_last_update) > (100 / g_speed)) {
    g_frame++;
    g_last_update = now;
  }

  drawAsciiScene();
  delay(5);
}
