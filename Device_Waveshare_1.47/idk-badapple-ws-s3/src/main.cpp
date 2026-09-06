#include <Arduino.h>
#include <LovyanGFX.hpp>
#include "badapple_video.h"

// --- Pin Definitions for Waveshare ESP32-S3 1.47" ---
#define TFT_BL 46
#define TFT_RST 39
#define TFT_DC 41
#define TFT_MOSI 45
#define TFT_MISO 18
#define TFT_SCLK 40
#define TFT_CS 42

#define BTN_UP 0
#define BTN_DOWN 2
#define BTN_LEFT 3
#define BTN_RIGHT 4
#define BTN_CENTER 5

// --- LGFX Configuration ---
class LGFX_Custom : public lgfx::LGFX_Device {
  lgfx::Panel_ST7789 _panel_instance;
  lgfx::Bus_SPI      _bus_instance;
public:
  LGFX_Custom(void) {
    auto bus_cfg = _bus_instance.config();
    bus_cfg.spi_host    = SPI2_HOST;
    bus_cfg.spi_mode    = 0;
    bus_cfg.freq_write  = 40000000;
    bus_cfg.freq_read   = 16000000;
    bus_cfg.spi_3wire   = false;
    bus_cfg.use_lock    = true;
    bus_cfg.dma_channel = SPI_DMA_CH_AUTO;
    
    bus_cfg.pin_sclk    = TFT_SCLK;
    bus_cfg.pin_mosi    = TFT_MOSI;
    bus_cfg.pin_miso    = TFT_MISO;
    bus_cfg.pin_dc      = TFT_DC;
    
    _bus_instance.config(bus_cfg);
    _panel_instance.setBus(&_bus_instance);

    auto panel_cfg = _panel_instance.config();
    panel_cfg.pin_cs           = TFT_CS;
    panel_cfg.pin_rst          = TFT_RST;
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
    setPanel(&_panel_instance);
  }
};

LGFX_Custom display;

// Frame buffer to hold binarized pixels (120x68 = 8160 bytes)
static uint8_t frame_buffer[BADAPPLE_WIDTH * BADAPPLE_HEIGHT];

// Modes
static int current_mode = 0;
static constexpr int MAX_MODES = 4;

// Display canvas for double buffering (using 240x136)
static LGFX_Sprite canvas(&display);

// Timing variables
static uint32_t last_frame_time = 0;
static uint32_t hud_trigger_time = 0;
static bool hud_visible = true;

static int frame_index = 0;
static bool is_playing = true;

// Button state tracking
static bool last_btn_center = true;
static bool last_btn_right = true;

// Custom colors (RGB565 format)
static constexpr uint16_t COLOR_CYBER_BG = 0x080412; // Very dark indigo
static constexpr uint16_t COLOR_CYBER_FG = 0xF81F;   // Neon Pink/Magenta
static constexpr uint16_t COLOR_MATRIX_BG = 0x0000;  // Deep Black
static constexpr uint16_t COLOR_MATRIX_FG = 0x37E2;  // High-contrast Neon/Matrix Green

void decode_frame(int idx) {
    if (idx < 0 || idx >= BADAPPLE_FRAMES) return;
    uint32_t start_offset = pgm_read_dword(&badapple_frame_offsets[idx]);
    
    int pixel_idx = 0;
    int total_pixels = BADAPPLE_WIDTH * BADAPPLE_HEIGHT;
    uint8_t color = 0; // Starts with Black
    uint32_t rle_ptr = start_offset;
    
    while (pixel_idx < total_pixels) {
        uint8_t run_len = pgm_read_byte(&badapple_rle[rle_ptr++]);
        for (int i = 0; i < run_len && pixel_idx < total_pixels; i++) {
            frame_buffer[pixel_idx++] = color;
        }
        color = 1 - color;
    }
}

void render_real_video(uint16_t bg_color, uint16_t fg_color) {
    canvas.fillSprite(bg_color);
    for (int y = 0; y < BADAPPLE_HEIGHT; y++) {
        for (int x = 0; x < BADAPPLE_WIDTH; x++) {
            if (frame_buffer[y * BADAPPLE_WIDTH + x] == 1) {
                canvas.fillRect(x * 2, y * 2, 2, 2, fg_color);
            }
        }
    }
}

void render_ascii_video(uint16_t bg_color, uint16_t fg_color) {
    canvas.fillSprite(bg_color);
    canvas.setTextColor(fg_color, bg_color);
    canvas.setTextSize(1);
    
    const char* ascii_table = " .:-=+*#%@";
    
    for (int r = 0; r < 16; r++) {
        for (int c = 0; c < 40; c++) {
            int white_count = 0;
            for (int dy = 0; dy < 4; dy++) {
                for (int dx = 0; dx < 3; dx++) {
                    int px = c * 3 + dx;
                    int py = r * 4 + dy;
                    if (frame_buffer[py * BADAPPLE_WIDTH + px] == 1) {
                        white_count++;
                    }
                }
            }
            int density_idx = (white_count * 9) / 12;
            char ch = ascii_table[density_idx];
            canvas.drawChar(ch, c * 6, r * 8 + 3);
        }
    }
}

void draw_hud() {
    uint32_t now = millis();
    if (!hud_visible) return;
    if (now - hud_trigger_time > 2500) {
        hud_visible = false;
        return;
    }
    
    int hud_h = 24;
    int hud_y = 135 - hud_h;
    
    canvas.fillRect(0, hud_y, 240, hud_h, 0x18E3);
    canvas.drawFastHLine(0, hud_y, 240, 0x07FF);
    
    canvas.setTextColor(0x07FF, 0x18E3);
    canvas.setTextSize(1);
    
    String mode_name = "";
    switch (current_mode) {
        case 0: mode_name = "RAW B&W"; break;
        case 1: mode_name = "CYBERPUNK"; break;
        case 2: mode_name = "ASCII MONO"; break;
        case 3: mode_name = "ASCII MATRIX"; break;
    }
    
    canvas.drawString("MODE: " + mode_name, 6, hud_y + 4);
    
    int elapsed_seconds = frame_index / BADAPPLE_FPS;
    int total_seconds = BADAPPLE_FRAMES / BADAPPLE_FPS;
    char time_str[32];
    sprintf(time_str, "%02d:%02d/%02d:%02d", elapsed_seconds / 60, elapsed_seconds % 60, total_seconds / 60, total_seconds % 60);
    canvas.drawRightString(time_str, 234, hud_y + 4);
    
    int bar_w = 228;
    int bar_h = 3;
    int bar_x = 6;
    int bar_y = hud_y + 15;
    
    canvas.fillRect(bar_x, bar_y, bar_w, bar_h, 0x39E7);
    int progress_w = (frame_index * bar_w) / BADAPPLE_FRAMES;
    canvas.fillRect(bar_x, bar_y, progress_w, bar_h, 0xF800);
}

void setup() {
    Serial.begin(115200);

    // Initialize Buttons
    pinMode(BTN_UP, INPUT_PULLUP);
    pinMode(BTN_DOWN, INPUT_PULLUP);
    pinMode(BTN_LEFT, INPUT_PULLUP);
    pinMode(BTN_RIGHT, INPUT_PULLUP);
    pinMode(BTN_CENTER, INPUT_PULLUP);

    // Initialize Backlight
    pinMode(TFT_BL, OUTPUT);
    digitalWrite(TFT_BL, HIGH);

    // Initialize Display
    display.init();
    display.setRotation(1); // Landscape mode (320x172)
    display.fillScreen(TFT_BLACK);
    
    canvas.setPsram(true);
    canvas.setColorDepth(8); // 8-bit color for fast rendering
    canvas.createSprite(240, 136); // Bad Apple is 120x68, scaled 2x is 240x136
    
    // Show splash screen
    canvas.fillSprite(TFT_BLACK);
    canvas.setTextColor(0x07FF);
    canvas.setTextSize(2);
    canvas.drawCenterString("BAD APPLE", 120, 35);
    canvas.setTextSize(1);
    canvas.setTextColor(TFT_WHITE);
    canvas.drawCenterString("Waveshare 1.47\"", 120, 65);
    canvas.setTextColor(0x39FF14);
    canvas.drawCenterString("Locked 30 FPS", 120, 85);
    
    // Center the 240x136 canvas on the 320x172 display
    int offset_x = (320 - 240) / 2;
    int offset_y = (172 - 136) / 2;
    canvas.pushRotateZoom(160, 86, 0.0f, 320.0f/240.0f, 172.0f/136.0f);
    
    delay(2000);
    
    last_frame_time = millis();
    hud_trigger_time = millis();
    hud_visible = true;
}

void loop() {
    // Check buttons (Active LOW)
    bool btn_center = digitalRead(BTN_CENTER) == LOW;
    bool btn_right = digitalRead(BTN_RIGHT) == LOW;

    // Btn Center toggles mode
    if (btn_center && !last_btn_center) {
        current_mode = (current_mode + 1) % MAX_MODES;
        hud_trigger_time = millis();
        hud_visible = true;
    }
    last_btn_center = btn_center;
    
    // Btn Right pauses/plays
    if (btn_right && !last_btn_right) {
        is_playing = !is_playing;
        hud_trigger_time = millis();
        hud_visible = true;
    }
    last_btn_right = btn_right;
    
    uint32_t now = millis();
    if (is_playing && (now - last_frame_time >= (1000 / BADAPPLE_FPS))) {
        last_frame_time = now;
        
        decode_frame(frame_index);
        
        switch (current_mode) {
            case 0: render_real_video(TFT_BLACK, TFT_WHITE); break;
            case 1: render_real_video(COLOR_CYBER_BG, COLOR_CYBER_FG); break;
            case 2: render_ascii_video(TFT_BLACK, TFT_WHITE); break;
            case 3: render_ascii_video(COLOR_MATRIX_BG, COLOR_MATRIX_FG); break;
        }
        
        draw_hud();
        
        // Push canvas centered on display
        int offset_x = (320 - 240) / 2; // 40
        int offset_y = (172 - 136) / 2; // 18
        canvas.pushRotateZoom(160, 86, 0.0f, 320.0f/240.0f, 172.0f/136.0f);
        
        frame_index++;
        if (frame_index >= BADAPPLE_FRAMES) {
            frame_index = 0;
        }
    }
    
    delay(1);
}
