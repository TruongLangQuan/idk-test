#include <M5Unified.h>
#include "badapple_video.h"

// Frame buffer to hold binarized pixels (120x68 = 8160 bytes)
static uint8_t frame_buffer[BADAPPLE_WIDTH * BADAPPLE_HEIGHT];

// Modes: 
// 0: Real Video (Standard Black & White)
// 1: Real Video (Cyberpunk - Neon Pink on Dark Purple)
// 2: ASCII Art (Standard Black & White)
// 3: ASCII Art (Matrix Green)
static int current_mode = 0;
static constexpr int MAX_MODES = 4;

// Display canvas for double buffering
static M5Canvas canvas(&M5.Display);

// Timing variables
static uint32_t last_frame_time = 0;
static uint32_t hud_trigger_time = 0;
static bool hud_visible = true;

static int frame_index = 0;
static bool is_playing = true;

// Custom colors (RGB565 format)
static constexpr uint16_t COLOR_CYBER_BG = 0x080412; // Very dark indigo
static constexpr uint16_t COLOR_CYBER_FG = 0xF81F;   // Neon Pink/Magenta
static constexpr uint16_t COLOR_MATRIX_BG = 0x0000;  // Deep Black
static constexpr uint16_t COLOR_MATRIX_FG = 0x37E2;  // High-contrast Neon/Matrix Green

// Decode RLE compressed frame into 1-bit frame_buffer
void decode_frame(int idx) {
    if (idx < 0 || idx >= BADAPPLE_FRAMES) return;
    
    // Get byte offset for this frame
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
        color = 1 - color; // Toggle color
    }
}

// Render functions
void render_real_video(uint16_t bg_color, uint16_t fg_color) {
    canvas.fillSprite(bg_color);
    for (int y = 0; y < BADAPPLE_HEIGHT; y++) {
        for (int x = 0; x < BADAPPLE_WIDTH; x++) {
            if (frame_buffer[y * BADAPPLE_WIDTH + x] == 1) {
                // Scale 2x to fill 240x136 perfectly
                canvas.fillRect(x * 2, y * 2, 2, 2, fg_color);
            }
        }
    }
}

void render_ascii_video(uint16_t bg_color, uint16_t fg_color) {
    canvas.fillSprite(bg_color);
    canvas.setTextColor(fg_color, bg_color);
    canvas.setFont(&fonts::Font0);
    canvas.setTextSize(1);
    
    const char* ascii_table = " .:-=+*#%@";
    
    // Downsample 120x68 frame to 40x16 character grid
    for (int r = 0; r < 16; r++) {
        for (int c = 0; c < 40; c++) {
            int white_count = 0;
            // Sum pixels in a 3x4 block
            for (int dy = 0; dy < 4; dy++) {
                for (int dx = 0; dx < 3; dx++) {
                    int px = c * 3 + dx;
                    int py = r * 4 + dy;
                    if (frame_buffer[py * BADAPPLE_WIDTH + px] == 1) {
                        white_count++;
                    }
                }
            }
            // Scale white_count (0-12) to density index (0-9)
            int density_idx = (white_count * 9) / 12;
            char ch = ascii_table[density_idx];
            canvas.drawChar(ch, c * 6, r * 8 + 3); // Center vertically with +3 offset
        }
    }
}

void draw_hud() {
    uint32_t now = millis();
    if (!hud_visible) return;
    
    // Fade out HUD after 2.5 seconds
    if (now - hud_trigger_time > 2500) {
        hud_visible = false;
        return;
    }
    
    // Draw a premium HUD overlay at the bottom
    int hud_h = 24;
    int hud_y = 135 - hud_h;
    
    // Draw translucent background (dark grey/blue)
    canvas.fillRect(0, hud_y, 240, hud_h, 0x18E3); // 50% opacity greyish blue
    canvas.drawFastHLine(0, hud_y, 240, 0x07FF);   // Cyberpunk Cyan border
    
    // Text labels
    canvas.setTextColor(0x07FF, 0x18E3);
    canvas.setFont(&fonts::Font0);
    canvas.setTextSize(1);
    
    String mode_name = "";
    switch (current_mode) {
        case 0: mode_name = "RAW B&W"; break;
        case 1: mode_name = "CYBERPUNK"; break;
        case 2: mode_name = "ASCII MONO"; break;
        case 3: mode_name = "ASCII MATRIX"; break;
    }
    
    canvas.drawString("MODE: " + mode_name, 6, hud_y + 4);
    
    // Frame count & FPS
    int elapsed_seconds = frame_index / BADAPPLE_FPS;
    int total_seconds = BADAPPLE_FRAMES / BADAPPLE_FPS;
    char time_str[32];
    sprintf(time_str, "%02d:%02d/%02d:%02d", elapsed_seconds / 60, elapsed_seconds % 60, total_seconds / 60, total_seconds % 60);
    canvas.drawRightString(time_str, 234, hud_y + 4);
    
    // Sleek progress bar
    int bar_w = 228;
    int bar_h = 3;
    int bar_x = 6;
    int bar_y = hud_y + 15;
    
    canvas.fillRect(bar_x, bar_y, bar_w, bar_h, 0x39E7); // Dark track
    int progress_w = (frame_index * bar_w) / BADAPPLE_FRAMES;
    canvas.fillRect(bar_x, bar_y, progress_w, bar_h, 0xF800); // Neon Pink bar
}

void setup() {
    auto cfg = M5.config();
    cfg.internal_imu = false;
    cfg.internal_rtc = false;
    cfg.internal_mic = false;
    cfg.internal_spk = false;
    M5.begin(cfg);
    
    M5.Display.setBaseColor(TFT_BLACK);
    // 240x135 widescreen landscape
    M5.Display.setRotation(1); 
    M5.Display.setBrightness(128);
    
    canvas.setPsram(true);
    canvas.setColorDepth(8); // 8-bit color for gorgeous rendering
    canvas.createSprite(240, 135);
    
    // Show splash screen
    canvas.fillSprite(TFT_BLACK);
    canvas.setTextColor(0x07FF);
    canvas.setFont(&fonts::Font0);
    canvas.setTextSize(2);
    canvas.drawCenterString("BAD APPLE", 120, 35);
    canvas.setTextSize(1);
    canvas.setTextColor(TFT_WHITE);
    canvas.drawCenterString("ESP32-S3 High-Speed RLE", 120, 65);
    canvas.setTextColor(0x39FF14);
    canvas.drawCenterString("Locked 30 FPS", 120, 85);
    canvas.pushSprite(0, 0);
    
    delay(2000);
    
    last_frame_time = millis();
    hud_trigger_time = millis();
    hud_visible = true;
}

void loop() {
    M5.update();
    
    // Check buttons
    // BtnA toggles mode
    if (M5.BtnA.wasPressed()) {
        current_mode = (current_mode + 1) % MAX_MODES;
        hud_trigger_time = millis();
        hud_visible = true;
    }
    
    // BtnB pauses/plays
    if (M5.BtnB.wasPressed()) {
        is_playing = !is_playing;
        hud_trigger_time = millis();
        hud_visible = true;
    }
    
    uint32_t now = millis();
    if (is_playing && (now - last_frame_time >= (1000 / BADAPPLE_FPS))) {
        last_frame_time = now;
        
        // Decode next frame
        decode_frame(frame_index);
        
        // Render according to mode
        switch (current_mode) {
            case 0:
                render_real_video(TFT_BLACK, TFT_WHITE);
                break;
            case 1:
                render_real_video(COLOR_CYBER_BG, COLOR_CYBER_FG);
                break;
            case 2:
                render_ascii_video(TFT_BLACK, TFT_WHITE);
                break;
            case 3:
                render_ascii_video(COLOR_MATRIX_BG, COLOR_MATRIX_FG);
                break;
        }
        
        // Draw HUD overlay
        draw_hud();
        
        // Push double-buffered canvas to display
        canvas.pushSprite(0, 0);
        
        // Advance frame
        frame_index++;
        if (frame_index >= BADAPPLE_FRAMES) {
            frame_index = 0; // Loop back
        }
    }
    
    // Small delay to prevent CPU choking
    delay(1);
}
