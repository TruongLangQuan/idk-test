import re

with open("idk-ir-ws-1.47/src/main.cpp", "r") as f:
    code = f.read()

# Replace M5Unified with LovyanGFX
lgfx_code = """#define LGFX_USE_V1
#include <LovyanGFX.hpp>

class LGFX : public lgfx::LGFX_Device {
  lgfx::Panel_ST7789 _panel_instance;
  lgfx::Bus_SPI      _bus_instance;
  lgfx::Light_PWM    _light_instance;
public:
  LGFX(void) {
    auto bus_cfg = _bus_instance.config();
    bus_cfg.spi_host   = SPI2_HOST;
    bus_cfg.dma_channel = SPI_DMA_CH_AUTO;
    bus_cfg.spi_mode   = 0;
    bus_cfg.freq_write = 80000000;
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

LGFX M5Display;

"""

code = code.replace("#include <M5Unified.h>", lgfx_code)

code = code.replace("static constexpr uint8_t IR_PIN = 19;", "static constexpr uint8_t IR_PIN = 9;")
code = code.replace("static constexpr uint8_t IR_PIN_EXT = 26;", "static constexpr uint8_t IR_PIN_EXT = 9;")

code = code.replace("M5.Display.", "M5Display.")
code = code.replace("M5.Display", "M5Display")

# Setup modification
setup_replacement = """
  pinMode(0, INPUT_PULLUP);
  pinMode(2, INPUT_PULLUP);
  pinMode(3, INPUT_PULLUP);
  pinMode(4, INPUT_PULLUP);
  pinMode(5, INPUT_PULLUP);
  M5Display.init();
  M5Display.setRotation(1);
"""

code = re.sub(r"auto cfg = M5\.config\(\);\s*M5\.begin\(cfg\);\s*#if defined\(STICKS3\)\s*M5\.Display\.setRotation\(1\);\s*#else\s*M5\.Display\.setRotation\(3\);\s*#endif", setup_replacement, code, flags=re.MULTILINE)

# Loop modification
# Replace M5 button calls with joystick reads.
# M5.BtnA.wasPressed() -> select
# M5.BtnB.wasPressed() -> right / next
# M5.BtnPWR.wasPressed() -> left / prev
# M5.BtnA.isPressed() -> btn_press
# M5.BtnB.isPressed() -> btn_right
# M5.BtnPWR.isPressed() -> btn_left

# We need a proper button debouncing for 5-way joystick.
btn_code = """
  static bool btn_press_last = false;
  static bool btn_right_last = false;
  static bool btn_left_last = false;
  static bool btn_up_last = false;
  static bool btn_down_last = false;
  
  bool btn_press_raw = (digitalRead(5) == LOW);
  bool btn_right_raw = (digitalRead(4) == LOW);
  bool btn_left_raw = (digitalRead(3) == LOW);
  bool btn_up_raw = (digitalRead(0) == LOW);
  bool btn_down_raw = (digitalRead(2) == LOW);
  
  bool btn_press = btn_press_raw;
  bool btn_right = btn_right_raw || btn_down_raw;
  bool btn_left = btn_left_raw || btn_up_raw;
  
  bool select = btn_press && !btn_press_last;
  bool right_pressed = btn_right && !btn_right_last;
  bool left_pressed = btn_left && !btn_left_last;
  
  btn_press_last = btn_press;
  btn_right_last = btn_right;
  btn_left_last = btn_left;
"""

code = code.replace("M5.update();", btn_code)

code = code.replace("const bool select = M5.BtnA.wasPressed();", "")
code = code.replace("M5.BtnB.wasPressed()", "right_pressed")
code = code.replace("M5.BtnB.isPressed()", "btn_right")
code = code.replace("M5.BtnPWR.wasPressed()", "left_pressed")
code = code.replace("M5.BtnPWR.isPressed()", "btn_left")
code = code.replace("M5.BtnA.isPressed()", "btn_press")

# Extra fix for BtnPWR.pressedFor(700)
code = re.sub(r"M5\.BtnPWR\.pressedFor\(700\)", "btn_left && (millis() - prevPressTime > 700)", code)

with open("idk-ir-ws-1.47/src/main.cpp", "w") as f:
    f.write(code)

