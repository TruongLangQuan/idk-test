import re

with open("src/main.cpp", "r") as f:
    code = f.read()

# Replace headers
code = code.replace("#include <M5Unified.h>", """#define LGFX_USE_V1
#include <LovyanGFX.hpp>
#include <SD_MMC.h>
""")
code = code.replace("#include <SD.h>", "")

# Insert LGFX class and button class right after includes
lgfx_class = """
class LGFX : public lgfx::LGFX_Device {
  lgfx::Panel_ST7789 _panel_instance;
  lgfx::Bus_SPI      _bus_instance;
  lgfx::Light_PWM    _light_instance;
public:
  LGFX(void) {
    auto bus_cfg = _bus_instance.config();
    bus_cfg.spi_host   = SPI2_HOST;
    bus_cfg.spi_mode   = 0;
    bus_cfg.freq_write = 40000000;
    bus_cfg.freq_read  = 16000000;
    bus_cfg.pin_sclk   = 40;
    bus_cfg.pin_mosi   = 45;
    bus_cfg.pin_miso   = -1;
    bus_cfg.pin_dc     = 41;
    _bus_instance.config(bus_cfg);
    _panel_instance.setBus(&_bus_instance);

    auto panel_cfg = _panel_instance.config();
    panel_cfg.pin_cs           = 42;
    panel_cfg.pin_rst          = 39;
    panel_cfg.pin_busy         = -1;
    panel_cfg.panel_width      = 172;
    panel_cfg.panel_height     = 320;
    panel_cfg.offset_x         = 34;
    panel_cfg.offset_y         = 0;
    panel_cfg.offset_rotation  = 0;
    panel_cfg.dummy_read_bits  = 8;
    panel_cfg.readable         = false;
    panel_cfg.invert           = true;
    panel_cfg.rgb_order        = false;
    _panel_instance.config(panel_cfg);

    auto light_cfg = _light_instance.config();
    light_cfg.pin_bl = 46;
    light_cfg.invert = false;
    light_cfg.freq   = 12000;
    light_cfg.pwm_channel = 7;
    _light_instance.config(light_cfg);
    _panel_instance.setLight(&_light_instance);

    setPanel(&_panel_instance);
  }
};
LGFX display_lcd;

class Button {
  int pin;
  bool state;
  bool lastState;
  uint32_t pressTime;
  uint32_t lastDebounce;
  bool _wasPressed;
public:
  Button(int p) : pin(p), state(false), lastState(false), pressTime(0), lastDebounce(0), _wasPressed(false) {
    pinMode(pin, INPUT_PULLUP);
  }
  void update() {
    _wasPressed = false;
    bool reading = (digitalRead(pin) == LOW);
    if (reading != lastState) lastDebounce = millis();
    if ((millis() - lastDebounce) > 50) {
      if (reading != state) {
        state = reading;
        if (state) { pressTime = millis(); _wasPressed = true; }
      }
    }
    lastState = reading;
  }
  bool isPressed() { return state; }
  bool wasPressed() { return _wasPressed; }
  bool pressedFor(uint32_t ms) { return state && (millis() - pressTime > ms); }
};

Button btnUp(0);
Button btnDown(2);
Button btnLeft(3);
Button btnRight(4);
Button btnPress(5);

constexpr int kPinSD_CLK    = 14;
constexpr int kPinSD_CMD    = 15;
constexpr int kPinSD_D0     = 16;
constexpr int kPinSD_D1     = 18;
constexpr int kPinSD_D2     = 17;
constexpr int kPinSD_D3     = 21;
"""
code = code.replace("static const uint32_t kFrameDelayMs = 100;", lgfx_class + "\nstatic const uint32_t kFrameDelayMs = 100;")

# Replace M5.Display calls
code = code.replace("M5.Display.", "display_lcd.")

# Remove SD pin definition block
code = re.sub(r'// SD pins aligned.*?(?=\nenum class)', '', code, flags=re.DOTALL)

# Replace ensureSdReady
sd_ready_func = """static bool ensureSdReady() {
  if (g_sd_ready) return true;
  SD_MMC.end();
  delay(10);
  if (!SD_MMC.setPins(kPinSD_CLK, kPinSD_CMD, kPinSD_D0, kPinSD_D1, kPinSD_D2, kPinSD_D3)) {
    return false;
  }
  if (!SD_MMC.begin("/sdcard", false, false, 20000000)) {
    if (!SD_MMC.begin("/sdcard", true, false, 20000000)) {
      return false;
    }
  }
  g_sd_ready = true;
  return g_sd_ready;
}"""
code = re.sub(r'static bool ensureSdReady\(\) \{.*?(?=\nstatic String joinPath)', sd_ready_func, code, flags=re.DOTALL)

# Replace battery function
bat_func = """static void drawBattery() {
  int w = display_lcd.width();
  int x = w - 42;
  int y = 6;
  int bw = 32;
  int bh = 10;
  display_lcd.drawRoundRect(x, y, bw, bh, 2, kFgColor);
  int raw_adc = analogRead(1);
  float voltage = (raw_adc > 0) ? (raw_adc / 4095.0f) * 3.3f * 2.0f : 3.85f;
  if (voltage > 4.2f) voltage = 4.2f;
  if (voltage < 3.2f) voltage = 3.2f;
  int level = (int)(((voltage - 3.2f) / 1.0f) * 100.0f);
  level = clampInt(level, 0, 100);
  int fill = (bw - 4) * level / 100;
  if (fill > 0) {
    display_lcd.fillRoundRect(x + 2, y + 2, fill, bh - 4, 2, kFgColor);
  }
}"""
code = re.sub(r'static void drawBattery\(\) \{.*?(?=\nstatic void drawFrame)', bat_func, code, flags=re.DOTALL)

# Change g_fs pointer type from fs::FS to FS
code = code.replace("static fs::FS *g_fs = nullptr;", "static fs::FS *g_fs = nullptr;")
code = code.replace("g_fs = &SD;", "g_fs = &SD_MMC;")

# Change setup
setup_func = """void setup() {
  Serial.begin(115200);
  display_lcd.init();
  display_lcd.setRotation(1);
  display_lcd.setBrightness(180);
  g_vi_font_loaded = idk_vi_font::load(display_lcd);

  WiFi.mode(WIFI_AP);
  WiFi.softAP(kApSsid, kApPass);
  g_udp.begin(kSubtitlePort);

  g_frame_buf = static_cast<uint8_t *>(ps_malloc(kFrameBufSize));
  if (!g_frame_buf) {
    g_frame_buf = static_cast<uint8_t *>(malloc(kFrameBufSize));
  }
  if (!g_frame_buf) {
    display_lcd.fillScreen(TFT_BLACK);
    display_lcd.setTextColor(TFT_RED, TFT_BLACK);
    display_lcd.setCursor(4, 4);
    display_lcd.print("Frame buffer alloc failed");
    return;
  }

  TJpgDec.setCallback(tft_output);
  TJpgDec.setSwapBytes(true);
  TJpgDec.setJpgScale(1);

  g_state = AppState::FILE_SELECT;
  g_status = "Scanning...";
  scanDir(g_dir);
  drawFileMenu();
}"""
code = re.sub(r'void setup\(\) \{.*?(?=\nvoid loop)', setup_func, code, flags=re.DOTALL)

# Button mappings
# M5.update() -> update all our buttons
# M5.BtnPWR -> btnLeft (Back/Esc)
# M5.BtnA -> btnPress (Select)
# M5.BtnB -> btnRight (Next/Down)
code = code.replace("M5.update();", "btnUp.update(); btnDown.update(); btnLeft.update(); btnRight.update(); btnPress.update();")
code = code.replace("M5.BtnPWR.pressedFor(", "btnLeft.pressedFor(")
code = code.replace("M5.BtnPWR.wasPressed()", "btnLeft.wasPressed()")
code = code.replace("M5.BtnPWR.isPressed()", "btnLeft.isPressed()")
code = code.replace("M5.BtnA.wasPressed()", "btnPress.wasPressed()")
code = code.replace("M5.BtnA.isPressed()", "btnPress.isPressed()")
code = code.replace("M5.BtnB.wasPressed()", "btnRight.wasPressed()")

# In loop for FILE_SELECT
code = code.replace("if (btnLeft.wasPressed() && !btnPress.isPressed() && g_entry_count > 0)", "if (btnLeft.wasPressed() && g_entry_count > 0)")
# Let's map Up and Down appropriately in FILE_SELECT since we have 5-way!
file_select_patch = """
    if (btnRight.wasPressed() && g_entry_count > 0) {
      g_entry_index = (g_entry_index + 1) % g_entry_count;
      drawFileMenu();
    }
    if (btnLeft.wasPressed() && g_entry_count > 0) {
      g_entry_index = (g_entry_index + g_entry_count - 1) % g_entry_count;
      drawFileMenu();
    }
"""
new_file_select = """
    if ((btnRight.wasPressed() || btnDown.wasPressed()) && g_entry_count > 0) {
      g_entry_index = (g_entry_index + 1) % g_entry_count;
      drawFileMenu();
    }
    if ((btnLeft.wasPressed() || btnUp.wasPressed()) && g_entry_count > 0) {
      g_entry_index = (g_entry_index + g_entry_count - 1) % g_entry_count;
      drawFileMenu();
    }
"""
code = code.replace(file_select_patch, new_file_select)

with open("src/main.cpp", "w") as f:
    f.write(code)
