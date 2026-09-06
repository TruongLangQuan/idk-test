#include <Arduino.h>
#include <SPI.h>
#include <vector>
#include <cmath>
#include <cstdlib>

#define LGFX_USE_V1
#include <LovyanGFX.hpp>

// Display Pins
#define TFT_BL 46
#define TFT_RST 39
#define TFT_DC 41
#define TFT_MOSI 45
#define TFT_MISO 18
#define TFT_SCLK 40
#define TFT_CS 42

class LGFX_Custom : public lgfx::LGFX_Device {
    lgfx::Panel_ST7789 _panel_instance;
    lgfx::Bus_SPI _bus_instance;
    lgfx::Light_PWM _light_instance;
public:
    LGFX_Custom(void) {
        {
            auto cfg = _bus_instance.config();
            cfg.spi_host = SPI2_HOST;
            cfg.spi_mode = 0;
            cfg.freq_write = 40000000;
            cfg.freq_read = 16000000;
            cfg.spi_3wire = true;
            cfg.use_lock = true;
            cfg.dma_channel = SPI_DMA_CH_AUTO;
            cfg.pin_sclk = TFT_SCLK;
            cfg.pin_mosi = TFT_MOSI;
            cfg.pin_miso = TFT_MISO;
            cfg.pin_dc = TFT_DC;
            _bus_instance.config(cfg);
            _panel_instance.setBus(&_bus_instance);
        }
        {
            auto cfg = _panel_instance.config();
            cfg.pin_cs = TFT_CS;
            cfg.pin_rst = TFT_RST;
            cfg.pin_busy = -1;
            cfg.panel_width = 172;
            cfg.panel_height = 320;
            cfg.offset_x = 34;
            cfg.offset_y = 0;
            cfg.offset_rotation = 0;
            cfg.dummy_read_pixel = 8;
            cfg.dummy_read_bits = 1;
            cfg.readable = true;
            cfg.invert = true;
            cfg.rgb_order = false;
            cfg.dlen_16bit = false;
            cfg.bus_shared = true;
            _panel_instance.config(cfg);
        }
        {
            auto cfg = _light_instance.config();
            cfg.pin_bl = TFT_BL;
            cfg.invert = false;
            cfg.freq = 44100;
            cfg.pwm_channel = 7;
            _light_instance.config(cfg);
            _panel_instance.setLight(&_light_instance);
        }
        setPanel(&_panel_instance);
    }
};

LGFX_Custom display;
LGFX_Sprite canvas(&display);

#define SCREEN_W 320
#define SCREEN_H 172

// Joystick Pins
#define BTN_UP 0
#define BTN_DOWN 2
#define BTN_LEFT 3
#define BTN_RIGHT 4
#define BTN_CENTER 5

bool btnDown(int p) { return digitalRead(p) == LOW; }

#define NUM_PARTICLES 3000
#define NUM_STARS 800
#define R_BH 35.0f
#define R_INNER 45.0f
#define R_OUTER 140.0f

struct Particle {
    float angle;
    float radius;
    float speed;
    uint16_t color;
    float y_offset;
};

struct Star {
    float x;
    float y;
    uint16_t color;
};

Particle disk[NUM_PARTICLES];
Star stars[NUM_STARS];

float tilt_angle = 1.3f; // radians (about 75 degrees)
float cam_x = 0;
float cam_y = 0;

uint16_t getColor(float radius) {
    // Hot (white/blue) at R_INNER, cool (red/orange) at R_OUTER
    float t = (radius - R_INNER) / (R_OUTER - R_INNER);
    if (t < 0) t = 0;
    if (t > 1) t = 1;
    
    int r, g, b;
    if (t < 0.2f) {
        // White to Light Blue
        float nt = t / 0.2f;
        r = 255 - nt * 100;
        g = 255 - nt * 50;
        b = 255;
    } else if (t < 0.5f) {
        // Light Blue to Orange
        float nt = (t - 0.2f) / 0.3f;
        r = 155 + nt * 100;
        g = 205 - nt * 100;
        b = 255 - nt * 200;
    } else {
        // Orange to Dark Red
        float nt = (t - 0.5f) / 0.5f;
        r = 255 - nt * 150;
        g = 105 - nt * 105;
        b = 55 - nt * 55;
    }
    
    // Add some random dimming
    float dim = 0.5f + (rand() % 50) / 100.0f;
    r *= dim; g *= dim; b *= dim;
    if(r>255) r=255; if(g>255) g=255; if(b>255) b=255;
    
    return display.color565(r, g, b);
}

void setup() {
    Serial.begin(115200);
    pinMode(BTN_UP, INPUT_PULLUP);
    pinMode(BTN_DOWN, INPUT_PULLUP);
    pinMode(BTN_LEFT, INPUT_PULLUP);
    pinMode(BTN_RIGHT, INPUT_PULLUP);
    pinMode(BTN_CENTER, INPUT_PULLUP);
    
    display.init();
    display.setRotation(1);
    display.setBrightness(255);
    canvas.createSprite(SCREEN_W, SCREEN_H);
    
    // Init Disk
    for(int i=0; i<NUM_PARTICLES; i++) {
        disk[i].angle = (rand() % 3600) / 10.0f * M_PI / 180.0f;
        
        // Non-linear distribution, more dense near inner edge
        float t = pow((rand() % 1000) / 1000.0f, 2.0f); 
        disk[i].radius = R_INNER + t * (R_OUTER - R_INNER);
        
        // Speed relates to Keplerian orbit (v ~ 1/sqrt(r))
        disk[i].speed = 15.0f / sqrt(disk[i].radius);
        
        // Thickness of the disk
        disk[i].y_offset = ((rand() % 200) - 100) / 100.0f * (disk[i].radius * 0.05f);
        
        disk[i].color = getColor(disk[i].radius);
    }
    
    // Init Stars
    for(int i=0; i<NUM_STARS; i++) {
        stars[i].x = (rand() % 1000) - 500;
        stars[i].y = (rand() % 600) - 300;
        int c = 100 + (rand() % 155);
        stars[i].color = display.color565(c, c, c);
    }
}

// Function to apply gravitational lensing
void applyLensing(float u, float v, float z, float& out_u, float& out_v) {
    if (z > 0) {
        // In front: slightly magnifying
        float r = sqrt(u*u + v*v);
        if (r < 1.0f) r = 1.0f;
        out_u = u;
        out_v = v;
        return;
    }
    
    // Behind: strong lensing
    float r = sqrt(u*u + v*v);
    if (r < 0.1f) r = 0.1f;
    
    float Rs = R_BH * 1.5f; // Effective lensing radius
    
    // Approximation of primary Einstein ring image
    float r_app = (r + sqrt(r*r + 4.0f * Rs * Rs)) / 2.0f;
    
    out_u = u * (r_app / r);
    out_v = v * (r_app / r);
}

void loop() {
    // Controls
    if (btnDown(BTN_UP)) tilt_angle -= 0.05f;
    if (btnDown(BTN_DOWN)) tilt_angle += 0.05f;
    if (tilt_angle < 0) tilt_angle = 0;
    if (tilt_angle > M_PI/2.0f) tilt_angle = M_PI/2.0f;
    
    if (btnDown(BTN_LEFT)) cam_x -= 3.0f;
    if (btnDown(BTN_RIGHT)) cam_x += 3.0f;
    if (btnDown(BTN_CENTER)) {
        cam_x = 0;
        cam_y = 0;
        tilt_angle = 1.3f;
    }
    
    canvas.fillSprite(TFT_BLACK);
    
    float cx = SCREEN_W / 2;
    float cy = SCREEN_H / 2;
    
    // Draw Stars
    for(int i=0; i<NUM_STARS; i++) {
        float u = stars[i].x - cam_x;
        float v = stars[i].y - cam_y;
        
        float lu, lv;
        applyLensing(u, v, -100.0f, lu, lv);
        
        float r_app = sqrt(lu*lu + lv*lv);
        if (r_app >= R_BH) {
            int sx = cx + lu;
            int sy = cy + lv;
            if (sx >= 0 && sx < SCREEN_W && sy >= 0 && sy < SCREEN_H) {
                canvas.drawPixel(sx, sy, stars[i].color);
            }
        }
    }
    
    // Draw Disk Particles
    // We separate them into back (behind BH) and front (in front of BH)
    // To handle drawing order (Back disk -> Black Hole -> Front disk)
    
    std::vector<int> back_particles;
    std::vector<int> front_particles;
    back_particles.reserve(NUM_PARTICLES);
    front_particles.reserve(NUM_PARTICLES);
    
    float cos_t = cos(tilt_angle);
    float sin_t = sin(tilt_angle);
    
    for(int i=0; i<NUM_PARTICLES; i++) {
        disk[i].angle -= disk[i].speed * 0.016f; // Update position
        if (disk[i].angle < 0) disk[i].angle += M_PI * 2;
        
        float px = cos(disk[i].angle) * disk[i].radius;
        float pz = sin(disk[i].angle) * disk[i].radius;
        float py = disk[i].y_offset;
        
        // Rotate around X axis for tilt
        float rot_y = py * cos_t - pz * sin_t;
        float rot_z = py * sin_t + pz * cos_t;
        
        if (rot_z < 0) back_particles.push_back(i);
        else front_particles.push_back(i);
    }
    
    // Draw Back Particles (Lensed)
    for(int idx : back_particles) {
        float px = cos(disk[idx].angle) * disk[idx].radius;
        float pz = sin(disk[idx].angle) * disk[idx].radius;
        float py = disk[idx].y_offset;
        
        float rot_y = py * cos_t - pz * sin_t;
        float rot_z = py * sin_t + pz * cos_t;
        
        float lu, lv;
        applyLensing(px - cam_x, rot_y - cam_y, rot_z, lu, lv);
        
        float r_app = sqrt(lu*lu + lv*lv);
        if (r_app >= R_BH) {
            int sx = cx + lu;
            int sy = cy + lv;
            if (sx >= 0 && sx < SCREEN_W && sy >= 0 && sy < SCREEN_H) {
                canvas.drawPixel(sx, sy, disk[idx].color);
                // Optional: draw slight bloom
                if (r_app < R_BH + 15) {
                    canvas.drawPixel(sx+1, sy, disk[idx].color);
                    canvas.drawPixel(sx, sy+1, disk[idx].color);
                }
            }
        }
    }
    
    // Draw Black Hole Event Horizon
    canvas.fillCircle(cx - cam_x, cy - cam_y, R_BH, TFT_BLACK);
    
    // Draw Front Particles
    for(int idx : front_particles) {
        float px = cos(disk[idx].angle) * disk[idx].radius;
        float pz = sin(disk[idx].angle) * disk[idx].radius;
        float py = disk[idx].y_offset;
        
        float rot_y = py * cos_t - pz * sin_t;
        
        float u = px - cam_x;
        float v = rot_y - cam_y;
        
        int sx = cx + u;
        int sy = cy + v;
        if (sx >= 0 && sx < SCREEN_W && sy >= 0 && sy < SCREEN_H) {
            canvas.drawPixel(sx, sy, disk[idx].color);
            // Optional: draw slight bloom
            float r_app = sqrt(u*u + v*v);
            if (r_app < R_BH + 20) {
                canvas.drawPixel(sx+1, sy, disk[idx].color);
                canvas.drawPixel(sx, sy+1, disk[idx].color);
            }
        }
    }
    
    // UI Overlay
    canvas.setTextColor(TFT_WHITE);
    canvas.setTextSize(1.0f);
    canvas.setCursor(2, 2);
    canvas.print("Interstellar Gargantua Simulation");
    canvas.setCursor(2, 12);
    canvas.print("SW1: Pan (L/R) | Tilt (U/D) | Reset (C)");
    
    canvas.pushSprite(0, 0);
}
