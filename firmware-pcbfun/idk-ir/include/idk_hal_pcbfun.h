#pragma once
#ifdef __cplusplus

#define LGFX_USE_V1
#include <LovyanGFX.hpp>

// Custom LGFX device class for Waveshare/PCBFUN ESP32-S3-LCD-1.47
class LGFX : public lgfx::LGFX_Device {
  lgfx::Panel_ST7789 _panel_instance;
  lgfx::Bus_SPI      _bus_instance;

public:
  LGFX(void) {
    {
      auto bus_cfg = _bus_instance.config();
      bus_cfg.spi_host = SPI2_HOST;
      bus_cfg.spi_mode = 0;
      bus_cfg.freq_write = 40000000;
      bus_cfg.freq_read  = 16000000;
      bus_cfg.pin_sclk = 40;
      bus_cfg.pin_mosi = 45;
      bus_cfg.pin_miso = -1;
      bus_cfg.pin_dc   = 41;
      _bus_instance.config(bus_cfg);
      _panel_instance.setBus(&_bus_instance);
    }

    {
      auto panel_cfg = _panel_instance.config();
      panel_cfg.pin_cs           = 42;
      panel_cfg.pin_rst          = 39;
      panel_cfg.pin_busy         = -1;
      panel_cfg.panel_width      = 172;
      panel_cfg.panel_height     = 320;
      panel_cfg.offset_x         = 34; // Centering offset
      panel_cfg.offset_y         = 0;
      panel_cfg.offset_rotation  = 0;
      panel_cfg.dummy_read_bits  = 8;
      panel_cfg.readable         = false;
      panel_cfg.invert           = true;
      panel_cfg.rgb_order        = false; // BGR order
      _panel_instance.config(panel_cfg);
      setPanel(&_panel_instance);
    }
  }
};

class MockButton {
  int _pin;
  bool _last_state;
  bool _curr_state;
  bool _was_pressed;
  bool _was_released;
  uint32_t _press_time;
  uint32_t _release_time;

public:
  MockButton(int pin) : _pin(pin), _last_state(false), _curr_state(false), _was_pressed(false), _was_released(false), _press_time(0), _release_time(0) {}

  void begin() {
    if (_pin >= 0) {
      pinMode(_pin, INPUT_PULLUP);
    }
  }

  void update(bool is_low) {
    _last_state = _curr_state;
    _curr_state = is_low;
    
    _was_pressed = false;
    _was_released = false;
    
    if (_curr_state && !_last_state) {
      _was_pressed = true;
      _press_time = millis();
    } else if (!_curr_state && _last_state) {
      _was_released = true;
      _release_time = millis();
    }
  }

  bool isPressed() const { return _curr_state; }
  bool isReleased() const { return !_curr_state; }
  bool wasPressed() const { return _was_pressed; }
  bool wasReleased() const { return _was_released; }
  bool wasHold() const { return pressedFor(500); }
  bool wasClicked() const { return wasPressed(); }
  
  bool pressedFor(uint32_t ms) const {
    return _curr_state && (millis() - _press_time >= ms);
  }
  
  bool releasedFor(uint32_t ms) const {
    return !_curr_state && (millis() - _release_time >= ms);
  }
};

class M5UnifiedMock {
public:
  LGFX Display;
  MockButton BtnA;
  MockButton BtnB;
  MockButton BtnPWR;
  MockButton BtnC;
  
  struct ImuMock {
    bool isEnabled() { return false; }
    bool getAccel(float* x, float* y, float* z) {
      *x = 0.0f;
      *y = 0.0f;
      *z = 1.0f;
      return true;
    }
  } Imu;

  struct SpeakerMock {
    void playRaw(const int16_t* data, size_t len, uint32_t sample_rate, bool is_stereo, int repeat, int channel) {}
    void playRaw(const uint8_t* data, size_t len, uint32_t sample_rate, bool is_stereo, int repeat, int channel) {}
    void setVolume(uint8_t volume) {}
  } Speaker;

  struct PowerMock {
    int getBatteryLevel() { return 100; }
    float getBatteryVoltage() { return 4.2f; }
  } Power;

  struct RtcMock {
    bool isEnabled() { return false; }
    void setSystemTimeFromRtc() {}
    template <typename T>
    void setDateTime(const T&) {}
  } Rtc;

  M5UnifiedMock() : BtnA(6), BtnB(7), BtnPWR(-1), BtnC(-1) {}

  static M5UnifiedMock& getInstance() {
    static M5UnifiedMock instance;
    return instance;
  }

  void begin() {
    Display.init();
    Display.setRotation(1); // Landscape
    pinMode(48, OUTPUT);
    digitalWrite(48, HIGH); // Backlight
    
    BtnA.begin();
    BtnB.begin();
    BtnPWR.begin();
    BtnC.begin();
  }

  template <typename T>
  void begin(const T&) {
    begin();
  }

  void update() {
    BtnA.update(digitalRead(6) == LOW);
    BtnB.update(digitalRead(7) == LOW);
    BtnPWR.update(false);
    BtnC.update(false);
  }
  
  struct config_t {
    bool external_rtc = false;
    bool internal_imu = true;
    bool internal_rtc = true;
    bool internal_mic = true;
    bool internal_spk = true;
    bool external_imu = false;
    bool external_mic = false;
    bool external_spk = false;
    bool clear_display = true;
    bool output_power = true;
  };
  
  static config_t config() {
    return config_t();
  }
};

#define M5 (M5UnifiedMock::getInstance())
// Redefining standard M5Unified types
typedef LGFX M5GFX;
typedef LGFX_Sprite M5Canvas;

#include <SD.h>
#include <SD_MMC.h>

class SDMockClass {
public:
  bool begin(int cs = -1, SPIClass &spi = SPI, uint32_t freq = 4000000) {
    SD_MMC.setPins(14, 15, 16, 18, 17, 21);
    return SD_MMC.begin("/sdcard", false, false, 20000000);
  }
  bool begin(int cs, SPIClass &spi, uint32_t freq, const char* mount, uint8_t max_files = 5, bool format = false) {
    SD_MMC.setPins(14, 15, 16, 18, 17, 21);
    return SD_MMC.begin(mount, false, format, 20000000);
  }
  
  fs::File open(const char* path, const char* mode = FILE_READ) { return SD_MMC.open(path, mode); }
  fs::File open(const String& path, const char* mode = FILE_READ) { return SD_MMC.open(path, mode); }
  bool exists(const char* path) { return SD_MMC.exists(path); }
  bool exists(const String& path) { return SD_MMC.exists(path); }
  
  fs::FS* operator&() { return &SD_MMC; }
  operator fs::FS&() { return SD_MMC; }
};

inline SDMockClass SDMock;
#define SD SDMock

#endif

