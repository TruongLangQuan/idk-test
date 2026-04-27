#include <M5Unified.h>

namespace {

// Assumption: this 5-way module is wired as active-low switches with a shared GND.
// Default map for M5StickC Plus2 using exposed pins from the top header + HY2.0-4:
// - Top header: G26, G36/G25, G0
// - HY2.0-4 (Grove): G32, G33
// Change these GPIOs to match your actual wiring.
static constexpr int kPinUp = 32;      // HY2.0-4 / Grove
static constexpr int kPinDown = 33;    // HY2.0-4 / Grove
static constexpr int kPinLeft = 25;    // top header label G36/G25
static constexpr int kPinRight = 26;   // top header
static constexpr int kPinCenter = 0;   // top header

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

static constexpr uint32_t kPollDelayMs = 15;
static uint32_t g_last_summary_ms = 0;

bool readPressed(int pin) {
  return digitalRead(pin) == LOW;
}

void drawState() {
  M5.Display.fillScreen(TFT_BLACK);
  M5.Display.setTextColor(TFT_WHITE, TFT_BLACK);
  M5.Display.setTextSize(1);
  M5.Display.setCursor(4, 4);
  M5.Display.print("idk-5way-test");
  M5.Display.setTextColor(TFT_DARKGREY, TFT_BLACK);
  M5.Display.setCursor(4, 16);
  M5.Display.print("5-way tactile switch");

  for (size_t i = 0; i < (sizeof(g_buttons) / sizeof(g_buttons[0])); ++i) {
    const int y = 36 + static_cast<int>(i) * 18;
    const bool on = g_buttons[i].pressed;
    M5.Display.fillRoundRect(8, y, 224, 14, 4, on ? TFT_GREEN : TFT_DARKGREY);
    M5.Display.setTextColor(on ? TFT_BLACK : TFT_WHITE, on ? TFT_GREEN : TFT_BLACK);
    M5.Display.setCursor(14, y + 3);
    M5.Display.printf("%-6s : %s", g_buttons[i].name, on ? "PRESSED" : "released");
  }

  M5.Display.setTextColor(TFT_YELLOW, TFT_BLACK);
  M5.Display.setCursor(4, 126);
  M5.Display.print("Edit GPIOs in src/main.cpp if needed");
}

void printWiringHint() {
  Serial.println();
  Serial.println("idk-5way-test");
  Serial.println("Assumption: VCC -> 3V3, GND -> GND, each signal -> one GPIO");
  Serial.println("Assumption: switch outputs are active LOW with INPUT_PULLUP");
  Serial.println("Default wiring uses top header GPIO + HY2.0-4 GPIO");
  Serial.printf("UP=%d DOWN=%d LEFT=%d RIGHT=%d CENTER=%d\n", kPinUp, kPinDown, kPinLeft, kPinRight,
                kPinCenter);
  Serial.println("Press any direction or center to see events.");
  Serial.println();
}

void printSummary() {
  Serial.print("[STATE] ");
  for (size_t i = 0; i < (sizeof(g_buttons) / sizeof(g_buttons[0])); ++i) {
    Serial.print(g_buttons[i].name);
    Serial.print('=');
    Serial.print(g_buttons[i].pressed ? "ON" : "OFF");
    if (i + 1 < (sizeof(g_buttons) / sizeof(g_buttons[0]))) {
      Serial.print(' ');
    }
  }
  Serial.println();
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

  printWiringHint();
  printSummary();
  drawState();
}

void loop() {
  M5.update();
  bool changed = false;

  for (auto& button : g_buttons) {
    const bool now_pressed = readPressed(button.pin);
    if (now_pressed == button.pressed) continue;

    button.pressed = now_pressed;
    changed = true;
    Serial.printf("[EVENT] %-6s %s\n", button.name, button.pressed ? "PRESSED" : "RELEASED");
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
