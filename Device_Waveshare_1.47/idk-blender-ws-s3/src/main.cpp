#include <Arduino.h>
#include <SPI.h>
#include <SD_MMC.h>
#include <FS.h>
#include <vector>
#include <math.h>
#include <algorithm>

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

// --- 3D ENGINE ---
struct Vec3 {
    float x, y, z;
    Vec3(float _x=0, float _y=0, float _z=0) : x(_x), y(_y), z(_z) {}
    Vec3 operator+(const Vec3& o) const { return Vec3(x+o.x, y+o.y, z+o.z); }
    Vec3 operator-(const Vec3& o) const { return Vec3(x-o.x, y-o.y, z-o.z); }
    float dot(const Vec3& o) const { return x*o.x + y*o.y + z*o.z; }
    Vec3 cross(const Vec3& o) const { return Vec3(y*o.z-z*o.y, z*o.x-x*o.z, x*o.y-y*o.x); }
    Vec3 normalize() const { float l = sqrt(x*x+y*y+z*z); return (l>0) ? Vec3(x/l, y/l, z/l) : *this; }
};

struct Face {
    Vec3 p[4];
    uint16_t color;
    float depth;
    bool visible;
    bool is_wire;
};

// Grid setup
#define GRID_SIZE 64
uint8_t* grid; 

inline uint8_t getGrid(int x, int y, int z) {
    return grid[(x * GRID_SIZE + y) * GRID_SIZE + z];
}

inline void setGrid(int x, int y, int z, uint8_t val) {
    grid[(x * GRID_SIZE + y) * GRID_SIZE + z] = val;
}

Vec3 cursor(GRID_SIZE/2, 0, GRID_SIZE/2);
uint8_t current_shape = 1;
uint8_t current_color = 0; // 0=White, 1=Red, 2=Green, 3=Blue, 4=Yellow

// Camera
float cam_yaw = 0.785f; // 45 deg
float cam_pitch = 0.5f; // ~30 deg
float cam_dist = 80.0f; // Zoomed out for 64x64

enum Mode {
    MODE_PLACE_XZ,
    MODE_PLACE_Y,
    MODE_ORBIT,
    MODE_ZOOM,
    MODE_MENU
};
Mode current_mode = MODE_PLACE_XZ;
int menu_sel = 0;
bool sd_ok = false;

// 3D Projection
Vec3 project(Vec3 p, Vec3 cam_pos, Vec3 forward, Vec3 right, Vec3 up) {
    Vec3 v = p - cam_pos;
    float x = v.dot(right);
    float y = v.dot(up);
    float z = v.dot(forward);
    
    if (z < 0.1f) z = 0.1f;
    float fov = 150.0f;
    float px = (x / z) * fov + SCREEN_W/2.0f;
    float py = -(y / z) * fov + SCREEN_H/2.0f;
    return Vec3(px, py, z);
}

void saveModel() {
    if(!sd_ok) return;
    File file = SD_MMC.open("/blender_save_v64.bin", FILE_WRITE);
    if(file) {
        file.write((uint8_t*)grid, GRID_SIZE*GRID_SIZE*GRID_SIZE);
        file.close();
    }
}

void loadModel() {
    if(!sd_ok) return;
    File file = SD_MMC.open("/blender_save_v64.bin", FILE_READ);
    if(file) {
        file.read((uint8_t*)grid, GRID_SIZE*GRID_SIZE*GRID_SIZE);
        file.close();
    }
}

uint16_t getBaseColor(uint8_t c) {
    switch(c) {
        case 1: return display.color565(255, 50, 50);
        case 2: return display.color565(50, 255, 50);
        case 3: return display.color565(50, 100, 255);
        case 4: return display.color565(255, 255, 50);
        default: return display.color565(200, 200, 200);
    }
}

void renderScene() {
    canvas.fillSprite(display.color565(20, 20, 40));
    
    Vec3 center(GRID_SIZE/2.0f, 0, GRID_SIZE/2.0f);
    Vec3 cam_pos(
        center.x + cam_dist * cos(cam_pitch) * sin(cam_yaw),
        center.y + cam_dist * sin(cam_pitch),
        center.z + cam_dist * cos(cam_pitch) * cos(cam_yaw)
    );
    
    Vec3 forward = (center - cam_pos).normalize();
    Vec3 world_up(0,1,0);
    Vec3 right = forward.cross(world_up).normalize();
    Vec3 up = right.cross(forward).normalize();
    
    Vec3 light_dir(0.5f, 1.0f, 0.3f);
    light_dir = light_dir.normalize();
    
    std::vector<Face> faces;
    
    // Base platform wireframe (Draw every 4 blocks to avoid clutter)
    for(int i=0; i<=GRID_SIZE; i+=4) {
        Vec3 p1 = project(Vec3(i, 0, 0), cam_pos, forward, right, up);
        Vec3 p2 = project(Vec3(i, 0, GRID_SIZE), cam_pos, forward, right, up);
        if(p1.z>0.1f && p2.z>0.1f) canvas.drawLine(p1.x, p1.y, p2.x, p2.y, display.color565(60,60,80));
        
        p1 = project(Vec3(0, 0, i), cam_pos, forward, right, up);
        p2 = project(Vec3(GRID_SIZE, 0, i), cam_pos, forward, right, up);
        if(p1.z>0.1f && p2.z>0.1f) canvas.drawLine(p1.x, p1.y, p2.x, p2.y, display.color565(60,60,80));
    }
    
    // Render boundaries
    Vec3 p1 = project(Vec3(GRID_SIZE, 0, 0), cam_pos, forward, right, up);
    Vec3 p2 = project(Vec3(GRID_SIZE, 0, GRID_SIZE), cam_pos, forward, right, up);
    if(p1.z>0.1f && p2.z>0.1f) canvas.drawLine(p1.x, p1.y, p2.x, p2.y, display.color565(100,100,150));
    p1 = project(Vec3(0, 0, GRID_SIZE), cam_pos, forward, right, up);
    if(p1.z>0.1f && p2.z>0.1f) canvas.drawLine(p1.x, p1.y, p2.x, p2.y, display.color565(100,100,150));
    
    // Bounds checking optimization (only process objects within frustum)
    for(int x=0; x<GRID_SIZE; x++) {
        for(int y=0; y<GRID_SIZE; y++) {
            for(int z=0; z<GRID_SIZE; z++) {
                uint8_t val = getGrid(x, y, z);
                uint8_t shape = val & 0x0F;
                uint8_t col = (val >> 4) & 0x0F;
                
                bool is_cursor = (cursor.x==x && cursor.y==y && cursor.z==z);
                if (!shape && !is_cursor) continue;
                
                if (is_cursor && !shape) {
                    shape = current_shape;
                    col = current_color;
                }
                
                // Fast culling (don't process if behind camera)
                Vec3 obj_pos(x+0.5f, y+0.5f, z+0.5f);
                if ((obj_pos - cam_pos).dot(forward) < 0) continue;
                
                std::vector<std::vector<Vec3>> face_verts;
                
                if (shape == 1) { // Cube
                    Vec3 p[8] = {
                        Vec3(x,y,z), Vec3(x+1,y,z), Vec3(x+1,y+1,z), Vec3(x,y+1,z),
                        Vec3(x,y,z+1), Vec3(x+1,y,z+1), Vec3(x+1,y+1,z+1), Vec3(x,y+1,z+1)
                    };
                    int f_idx[6][4] = {{0,3,2,1}, {4,5,6,7}, {0,1,5,4}, {3,7,6,2}, {0,4,7,3}, {1,2,6,5}};
                    for(int f=0; f<6; f++) face_verts.push_back({p[f_idx[f][0]], p[f_idx[f][1]], p[f_idx[f][2]], p[f_idx[f][3]]});
                } else if (shape == 2) { // Pyramid
                    Vec3 b0(x,y,z), b1(x+1,y,z), b2(x+1,y,z+1), b3(x,y,z+1);
                    Vec3 top(x+0.5f, y+1.0f, z+0.5f);
                    face_verts.push_back({b0, b1, b2, b3}); // Bottom
                    face_verts.push_back({b0, top, b1, b1}); // Front
                    face_verts.push_back({b1, top, b2, b2}); // Right
                    face_verts.push_back({b2, top, b3, b3}); // Back
                    face_verts.push_back({b3, top, b0, b0}); // Left
                } else if (shape == 3) { // Wedge
                    Vec3 b0(x,y,z), b1(x+1,y,z), b2(x+1,y,z+1), b3(x,y,z+1);
                    Vec3 t0(x,y+1,z), t1(x+1,y+1,z);
                    face_verts.push_back({b0, b1, b2, b3}); // Bottom
                    face_verts.push_back({b0, t0, t1, b1}); // Back wall
                    face_verts.push_back({b0, b3, t0, t0}); // Left side
                    face_verts.push_back({b1, t1, b2, b2}); // Right side
                    face_verts.push_back({b3, b2, t1, t0}); // Slanted face
                } else if (shape == 4) { // Diamond
                    Vec3 top(x+0.5f, y+1.0f, z+0.5f);
                    Vec3 bot(x+0.5f, y, z+0.5f);
                    Vec3 m0(x, y+0.5f, z+0.5f);
                    Vec3 m1(x+0.5f, y+0.5f, z);
                    Vec3 m2(x+1.0f, y+0.5f, z+0.5f);
                    Vec3 m3(x+0.5f, y+0.5f, z+1.0f);
                    face_verts.push_back({m1, top, m0, m0});
                    face_verts.push_back({m2, top, m1, m1});
                    face_verts.push_back({m3, top, m2, m2});
                    face_verts.push_back({m0, top, m3, m3});
                    face_verts.push_back({m0, bot, m1, m1});
                    face_verts.push_back({m1, bot, m2, m2});
                    face_verts.push_back({m2, bot, m3, m3});
                    face_verts.push_back({m3, bot, m0, m0});
                }
                
                uint16_t base_color = getBaseColor(col);
                if (is_cursor && !(val & 0x0F)) base_color = display.color565(255, 0, 255); 
                else if (is_cursor) base_color = display.color565(255, 150, 150); 
                
                for(auto& fv : face_verts) {
                    Face face;
                    Vec3 fc(0,0,0);
                    for(int i=0; i<4; i++) {
                        face.p[i] = fv[i];
                        fc = fc + face.p[i];
                    }
                    fc.x/=4; fc.y/=4; fc.z/=4;
                    
                    Vec3 v1 = face.p[1] - face.p[0];
                    Vec3 v2 = face.p[2] - face.p[0];
                    Vec3 normal = v1.cross(v2).normalize();
                    
                    Vec3 view_dir = (fc - cam_pos).normalize();
                    if(normal.dot(view_dir) >= 0) continue;
                    
                    float light = max(0.2f, normal.dot(light_dir));
                    
                    uint8_t r = (base_color >> 11) & 0x1F;
                    uint8_t g = (base_color >> 5) & 0x3F;
                    uint8_t b = base_color & 0x1F;
                    r = (uint8_t)(r * light);
                    g = (uint8_t)(g * light);
                    b = (uint8_t)(b * light);
                    face.color = (r << 11) | (g << 5) | b;
                    
                    face.is_wire = (is_cursor && !(val & 0x0F));
                    
                    for(int i=0; i<4; i++) face.p[i] = project(face.p[i], cam_pos, forward, right, up);
                    
                    face.depth = fc.dot(forward);
                    face.visible = true;
                    for(int i=0; i<4; i++) if(face.p[i].z < 0.1f) face.visible = false;
                    
                    if (face.visible) faces.push_back(face);
                }
            }
        }
    }
    
    std::sort(faces.begin(), faces.end(), [](const Face& a, const Face& b) {
        return a.depth > b.depth;
    });
    
    for(auto& f : faces) {
        if (f.is_wire) {
            for(int i=0; i<4; i++) {
                if (f.p[i].x != f.p[(i+1)%4].x || f.p[i].y != f.p[(i+1)%4].y)
                    canvas.drawLine(f.p[i].x, f.p[i].y, f.p[(i+1)%4].x, f.p[(i+1)%4].y, f.color);
            }
        } else {
            canvas.fillTriangle(f.p[0].x, f.p[0].y, f.p[1].x, f.p[1].y, f.p[2].x, f.p[2].y, f.color);
            canvas.fillTriangle(f.p[0].x, f.p[0].y, f.p[2].x, f.p[2].y, f.p[3].x, f.p[3].y, f.color);
            for(int i=0; i<4; i++) {
                if (f.p[i].x != f.p[(i+1)%4].x || f.p[i].y != f.p[(i+1)%4].y)
                    canvas.drawLine(f.p[i].x, f.p[i].y, f.p[(i+1)%4].x, f.p[(i+1)%4].y, display.color565(30,30,40));
            }
        }
    }
    
    // UI Overlay
    canvas.setTextColor(TFT_WHITE);
    canvas.setTextSize(1.0f);
    
    if (current_mode == MODE_MENU) {
        canvas.fillRoundRect(40, 30, 240, 120, 10, display.color565(20,50,80));
        canvas.setTextSize(1.5f);
        canvas.setCursor(50, 40); canvas.print("--- BLENDER MENU ---");
        
        const char* menus[] = {
            "Move X/Z", "Elevate Y", "Orbit Camera", "Zoom Camera",
            "Shape: Cube", "Shape: Pyramid", "Shape: Wedge", "Shape: Diamond",
            "Color: White", "Color: Red", "Color: Green", "Color: Blue", "Color: Yellow",
            "Save Model", "Load Model"
        };
        
        for(int i=0; i<6; i++) {
            int m_idx = menu_sel - 2 + i;
            if (m_idx < 0) m_idx += 15;
            if (m_idx >= 15) m_idx -= 15;
            
            if (i == 2) canvas.setTextColor(TFT_YELLOW);
            else canvas.setTextColor(TFT_LIGHTGREY);
            
            canvas.setCursor(50, 65 + i*12);
            canvas.printf("%c %s", (i==2)?'>':' ', menus[m_idx]);
        }
    } else {
        canvas.setCursor(5, 5);
        if (current_mode == MODE_PLACE_XZ) canvas.print("MODE: X/Z");
        if (current_mode == MODE_PLACE_Y) canvas.print("MODE: Y");
        if (current_mode == MODE_ORBIT) canvas.print("MODE: ORBIT");
        if (current_mode == MODE_ZOOM) canvas.print("MODE: ZOOM");
        
        canvas.setCursor(5, 155);
        canvas.printf("Cur:%.0f,%.0f,%.0f Shp:%d", cursor.x, cursor.y, cursor.z, current_shape);
    }
    
    if(!sd_ok) {
        canvas.setTextColor(TFT_RED);
        canvas.setCursor(SCREEN_W - 50, 5);
        canvas.print("NO SD");
    }
    
    canvas.pushSprite(0, 0);
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
    
    SD_MMC.setPins(14, 15, 16);
    if (SD_MMC.begin("/sdcard", true)) {
        sd_ok = true;
    }
    
    // Allocate 262KB dynamically in PSRAM
    grid = (uint8_t*)ps_malloc(GRID_SIZE * GRID_SIZE * GRID_SIZE);
    if (!grid) {
        canvas.fillSprite(TFT_RED);
        canvas.setTextColor(TFT_WHITE);
        canvas.setCursor(10, 50);
        canvas.print("PSRAM ALLOC FAILED!");
        canvas.pushSprite(0, 0);
        while(1) delay(100);
    }
    memset(grid, 0, GRID_SIZE * GRID_SIZE * GRID_SIZE);
}

unsigned long last_press = 0;
unsigned long center_down_time = 0;
bool center_handled = false;

void loop() {
    unsigned long now = millis();
    bool changed = false;
    
    bool u = btnDown(BTN_UP);
    bool d = btnDown(BTN_DOWN);
    bool l = btnDown(BTN_LEFT);
    bool r = btnDown(BTN_RIGHT);
    bool c = btnDown(BTN_CENTER);
    
    // Handle Center Long Press for Menu
    if (c) {
        if (center_down_time == 0) {
            center_down_time = now;
            center_handled = false;
        } else if (!center_handled && now - center_down_time > 600) {
            // Long press! Toggle Menu
            if (current_mode != MODE_MENU) current_mode = MODE_MENU;
            else current_mode = MODE_PLACE_XZ;
            center_handled = true;
            changed = true;
        }
    } else {
        if (center_down_time > 0 && !center_handled && now - center_down_time > 20) {
            // Short press
            if (current_mode == MODE_MENU) {
                if (menu_sel == 0) current_mode = MODE_PLACE_XZ;
                if (menu_sel == 1) current_mode = MODE_PLACE_Y;
                if (menu_sel == 2) current_mode = MODE_ORBIT;
                if (menu_sel == 3) current_mode = MODE_ZOOM;
                if (menu_sel == 4) current_shape = 1;
                if (menu_sel == 5) current_shape = 2;
                if (menu_sel == 6) current_shape = 3;
                if (menu_sel == 7) current_shape = 4;
                if (menu_sel == 8) current_color = 0;
                if (menu_sel == 9) current_color = 1;
                if (menu_sel == 10) current_color = 2;
                if (menu_sel == 11) current_color = 3;
                if (menu_sel == 12) current_color = 4;
                if (menu_sel == 13) { saveModel(); current_mode = MODE_PLACE_XZ; }
                if (menu_sel == 14) { loadModel(); current_mode = MODE_PLACE_XZ; }
            } else {
                int cx = (int)cursor.x;
                int cy = (int)cursor.y;
                int cz = (int)cursor.z;
                if (getGrid(cx, cy, cz) == 0) {
                    setGrid(cx, cy, cz, (current_color << 4) | current_shape);
                } else {
                    setGrid(cx, cy, cz, 0); // Remove
                }
            }
            changed = true;
        }
        center_down_time = 0;
        center_handled = false;
    }
    
    // D-Pad (Fast scrolling for 64x64)
    if (now - last_press > 50) { 
        if (current_mode == MODE_MENU) {
            if (now - last_press > 150) { // Keep menu scroll slow
                if (d) { menu_sel = (menu_sel + 1) % 15; last_press = now; changed = true; }
                if (u) { menu_sel = (menu_sel + 14) % 15; last_press = now; changed = true; }
            }
        }
        else if (current_mode == MODE_PLACE_XZ) {
            if (u && cursor.z > 0) { cursor.z--; last_press = now; changed = true; }
            if (d && cursor.z < GRID_SIZE-1) { cursor.z++; last_press = now; changed = true; }
            if (l && cursor.x > 0) { cursor.x--; last_press = now; changed = true; }
            if (r && cursor.x < GRID_SIZE-1) { cursor.x++; last_press = now; changed = true; }
        }
        else if (current_mode == MODE_PLACE_Y) {
            if (u && cursor.y < GRID_SIZE-1) { cursor.y++; last_press = now; changed = true; }
            if (d && cursor.y > 0) { cursor.y--; last_press = now; changed = true; }
        }
        else if (current_mode == MODE_ORBIT) {
            if (l) { cam_yaw -= 0.05f; changed = true; last_press = now; }
            if (r) { cam_yaw += 0.05f; changed = true; last_press = now; }
            if (u) { cam_pitch = min(cam_pitch + 0.05f, 1.5f); changed = true; last_press = now; }
            if (d) { cam_pitch = max(cam_pitch - 0.05f, -0.5f); changed = true; last_press = now; }
        }
        else if (current_mode == MODE_ZOOM) {
            if (u) { cam_dist = max(cam_dist - 2.0f, 5.0f); changed = true; last_press = now; }
            if (d) { cam_dist = min(cam_dist + 2.0f, 150.0f); changed = true; last_press = now; }
        }
    }
    
    static unsigned long last_render = 0;
    if (changed || now - last_render > 33) {
        renderScene();
        last_render = now;
    }
}
