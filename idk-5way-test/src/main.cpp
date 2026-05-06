#include <M5Unified.h>

namespace {

// Assumption: this 5-way module is wired as active-low switches with a shared GND.
// Default map for M5StickC Plus2 using exposed pins from the top header + HY2.0-4:
// - Top header: G26, G36/G25, G0
// - HY2.0-4 (Grove): G32, G33
static constexpr int kPinUp = 32;      // HY2.0-4 / Grove
static constexpr int kPinDown = 33;    // HY2.0-4 / Grove
static constexpr int kPinLeft = 25;    // top header label G36/G25
static constexpr int kPinRight = 26;   // top header
static constexpr int kPinCenter = 0;   // top header

// Joystick Module Mapping (Top Header 5-pin alignment):
// 1. GND -> GND
// 2. 5V  -> 5V
// 3. VRx -> G26 (Analog)
// 4. VRy -> G36 (Analog)
// 5. SW  -> G0  (Digital)
static constexpr int kPinJoyX = 26;
static constexpr int kPinJoyY = 36;
static constexpr int kPinJoySW = 0;

struct Button {
  const char* name;
  int pin;
  bool pressed;
};

Button g_buttons[] = {
    {"UP", kPinUp, false},
    {"DOWN", kPinDown, false},
    {"LEFT", kPinLeft, false},
    {"RIGHT", kPinRight, false},
    {"CENTER", kPinCenter, false},
};

struct Joystick {
  int x;
  int y;
  bool sw;
};

Joystick g_joy = {2048, 2048, false};

static constexpr uint32_t kPollDelayMs = 20;
static uint32_t g_last_summary_ms = 0;

bool readPressed(int pin) {
  return digitalRead(pin) == LOW;
}

void drawState() {
  M5.Display.startWrite();
  M5.Display.fillScreen(TFT_BLACK);
  M5.Display.setTextColor(TFT_WHITE, TFT_BLACK);
  M5.Display.setTextSize(1);
  M5.Display.setCursor(4, 4);
  M5.Display.print("idk-5way-test & joystick");
  
  // 5-Way Switch Section
  M5.Display.setTextColor(TFT_CYAN, TFT_BLACK);
  M5.Display.setCursor(4, 16);
  M5.Display.print("5-Way Switch (Digital)");

  for (size_t i = 0; i < (sizeof(g_buttons) / sizeof(g_buttons[0])); ++i) {
    const int y = 28 + static_cast<int>(i) * 14;
    const bool on = g_buttons[i].pressed;
    M5.Display.fillRoundRect(8, y, 100, 12, 2, on ? TFT_GREEN : TFT_DARKGREY);
    M5.Display.setTextColor(on ? TFT_BLACK : TFT_WHITE, on ? TFT_GREEN : TFT_BLACK);
    M5.Display.setCursor(12, y + 2);
    M5.Display.printf("%-6s:%s", g_buttons[i].name, on ? "PRSD" : "rel ");
  }

  // Joystick Section
  M5.Display.setTextColor(TFT_YELLOW, TFT_BLACK);
  M5.Display.setCursor(120, 16);
  M5.Display.print("Joystick (Analog)");

  const int jx = 120;
  const int jy = 28;
  M5.Display.setTextColor(TFT_WHITE, TFT_BLACK);
  M5.Display.setCursor(jx, jy);
  M5.Display.printf("X: %4d", g_joy.x);
  M5.Display.setCursor(jx, jy + 14);
  M5.Display.printf("Y: %4d", g_joy.y);
  M5.Display.setCursor(jx, jy + 28);
  M5.Display.printf("SW: %s", g_joy.sw ? "PRESSED" : "released");

  // Visual Joystick Box
  const int boxSize = 50;
  const int boxX = 140;
  const int boxY = 75;
  M5.Display.drawRect(boxX, boxY, boxSize, boxSize, TFT_WHITE);
  int curX = boxX + (g_joy.x * boxSize / 4096);
  int curY = boxY + (g_joy.y * boxSize / 4096);
  M5.Display.fillCircle(curX, curY, 3, TFT_RED);

  M5.Display.setTextColor(TFT_DARKGREY, TFT_BLACK);
  M5.Display.setCursor(4, 128);
  M5.Display.print("Pins: X=G26, Y=G36, SW=G0");
  M5.Display.endWrite();
}

void printWiringHint() {
  Serial.println("\n--- idk-5way-test & joystick ---");
  Serial.println("5-Way: UP=32, DOWN=33, LEFT=25, RIGHT=26, CENTER=0");
  Serial.println("Joystick: GND=GND, 5V=5V, VRx=G26, VRy=G36, SW=G0");
}

void printSummary() {
  Serial.printf("[STATE] 5W:");
  for (size_t i = 0; i < (sizeof(g_buttons) / sizeof(g_buttons[0])); ++i) {
    Serial.printf("%s=%d ", g_buttons[i].name, g_buttons[i].pressed);
  }
  Serial.printf("| JOY: X=%d Y=%d SW=%d\n", g_joy.x, g_joy.y, g_joy.sw);
}

}  // namespace

void setup() {
  auto cfg = M5.config();
  M5.begin(cfg);
  M5.Display.setRotation(3);
  M5.Display.setBrightness(180);
  Serial.begin(115200);
  delay(200);

  for (auto& button : g_buttons) {
    pinMode(button.pin, INPUT_PULLUP);
    button.pressed = readPressed(button.pin);
  }

  pinMode(kPinJoySW, INPUT_PULLUP);
  analogReadResolution(12);

  printWiringHint();
  printSummary();
  drawState();
}

void loop() {
  M5.update();
  bool changed = false;

  // Poll 5-way
  for (auto& button : g_buttons) {
    const bool now_pressed = readPressed(button.pin);
    if (now_pressed != button.pressed) {
      button.pressed = now_pressed;
      changed = true;
    }
  }

  // Poll Joystick
  int nx = analogRead(kPinJoyX);
  int ny = analogRead(kPinJoyY);
  bool nsw = (digitalRead(kPinJoySW) == LOW);

  if (abs(nx - g_joy.x) > 20 || abs(ny - g_joy.y) > 20 || nsw != g_joy.sw) {
    g_joy.x = nx;
    g_joy.y = ny;
    g_joy.sw = nsw;
    changed = true;
  }

  const uint32_t now = millis();
  if (changed) {
    drawState();
  }
  if (changed || now - g_last_summary_ms >= 1000) {
    g_last_summary_ms = now;
    printSummary();
  }

  delay(kPollDelayMs);
}
