#include <Arduino.h>
#include <SPI.h>
#define LGFX_USE_V1
#include <LovyanGFX.hpp>

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

// ASCII Settings
#define COLS 53
#define ROWS 21
char screenBuf[ROWS][COLS+1]; // +1 for null terminator per line

// Player state
float posX = 2.5, posY = 2.5;  // start in the middle of a 4x4 chunk
float dirX = -1, dirY = 0;
float planeX = 0, planeY = 0.66;
float moveSpeed = 0.15;
float rotSpeed = 0.12;
float wobble = 0;

// Monster state
float monsterX = 8.5, monsterY = 8.5;
bool dead = false;

// Infinite Procedural Map
bool isWall(int x, int y) {
    if (x < 0) x = -x + 10000; 
    if (y < 0) y = -y + 10000;
    
    int cx = x / 4; int cy = y / 4;
    int lx = x % 4; int ly = y % 4;
    
    uint32_t seed = cx * 73856093 ^ cy * 19349663;
    seed = (seed ^ (seed >> 16)) * 2654435761;
    
    if (lx == 0 && ly == 0) return true; // Corner pillar
    if (lx == 0) {
        if (ly == 2 && (seed & 1)) return false; // Door
        return true;
    }
    if (ly == 0) {
        if (lx == 2 && (seed & 2)) return false; // Door
        return true;
    }
    // Random pillars inside room
    if (lx == 2 && ly == 2 && (seed & 4)) return true;
    
    return false;
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
    
    // We draw text directly to a sprite
    canvas.createSprite(SCREEN_W, SCREEN_H);
    canvas.setTextFont(&fonts::Font0); // 6x8 default font
    canvas.setTextSize(1.0f);
}

void renderFrame() {
    canvas.fillSprite(TFT_BLACK);
    
    float depthBuffer[COLS];
    
    for (int x = 0; x < COLS; x++) {
        // calculate ray position and direction
        float cameraX = 2 * x / (float)COLS - 1; // x-coordinate in camera space
        float rayDirX = dirX + planeX * cameraX;
        float rayDirY = dirY + planeY * cameraX;
        
        // which box of the map we're in
        int mapX = int(posX);
        int mapY = int(posY);
        
        // length of ray from current position to next x or y-side
        float sideDistX;
        float sideDistY;
        
        // length of ray from one x or y-side to next x or y-side
        float deltaDistX = (rayDirX == 0) ? 1e30 : abs(1 / rayDirX);
        float deltaDistY = (rayDirY == 0) ? 1e30 : abs(1 / rayDirY);
        float perpWallDist;
        
        // what direction to step in x or y-direction (either +1 or -1)
        int stepX;
        int stepY;
        
        int hit = 0; // was there a wall hit?
        int side; // was a NS or a EW wall hit?
        
        if (rayDirX < 0) {
            stepX = -1;
            sideDistX = (posX - mapX) * deltaDistX;
        } else {
            stepX = 1;
            sideDistX = (mapX + 1.0 - posX) * deltaDistX;
        }
        if (rayDirY < 0) {
            stepY = -1;
            sideDistY = (posY - mapY) * deltaDistY;
        } else {
            stepY = 1;
            sideDistY = (mapY + 1.0 - posY) * deltaDistY;
        }
        
        // perform DDA
        while (hit == 0) {
            if (sideDistX < sideDistY) {
                sideDistX += deltaDistX;
                mapX += stepX;
                side = 0;
            } else {
                sideDistY += deltaDistY;
                mapY += stepY;
                side = 1;
            }
            if (isWall(mapX, mapY)) hit = 1;
        }
        
        if (side == 0) perpWallDist = (sideDistX - deltaDistX);
        else           perpWallDist = (sideDistY - deltaDistY);
        
        depthBuffer[x] = perpWallDist;
        
        int lineHeight = (int)(ROWS / perpWallDist);
        int pitch = int(sin(wobble)*2.0f); // head bobbing
        
        int drawStart = -lineHeight / 2 + ROWS / 2 + pitch;
        if(drawStart < 0) drawStart = 0;
        int drawEnd = lineHeight / 2 + ROWS / 2 + pitch;
        if(drawEnd >= ROWS) drawEnd = ROWS - 1;
        
        // Choose ASCII character for shading based on distance
        // Characters: @ % # * + = - : . 
        const char* shades = "@%#*+=-:. ";
        int shadeIdx = (int)(perpWallDist * 1.2f);
        if (shadeIdx > 9) shadeIdx = 9;
        if (side == 1) shadeIdx = min(9, shadeIdx + 1); // Darker on y-sides
        
        char wallChar = shades[shadeIdx];
        
        // Draw column
        for(int y = 0; y < ROWS; y++) {
            if (y < drawStart) {
                screenBuf[y][x] = ' '; // Ceiling
            } else if (y > drawEnd) {
                // Floor shading
                int floorDist = ROWS - y; // rough proxy for distance
                screenBuf[y][x] = (floorDist < 4) ? '-' : '.';
            } else {
                screenBuf[y][x] = wallChar;
            }
        }
    }
    
    // Draw Monster
    float dx = monsterX - posX;
    float dy = monsterY - posY;
    float invDet = 1.0 / (planeX * dirY - dirX * planeY);
    float transformX = invDet * (dirY * dx - dirX * dy);
    float transformY = invDet * (-planeY * dx + planeX * dy); // Z depth
    
    if (transformY > 0) {
        int spriteScreenX = int((COLS / 2) * (1 + transformX / transformY));
        int spriteHeight = abs(int(ROWS / transformY));
        int pitch = int(sin(wobble)*2.0f);
        int drawStartY = -spriteHeight / 2 + ROWS / 2 + pitch;
        int drawEndY = spriteHeight / 2 + ROWS / 2 + pitch;
        int spriteWidth = abs(int((ROWS/2) / transformY)); // Monster is thin
        int drawStartX = -spriteWidth / 2 + spriteScreenX;
        int drawEndX = spriteWidth / 2 + spriteScreenX;
        
        const char* monsterChars = "XMW";
        int t = (millis() / 100) % 3;
        
        for (int stripe = drawStartX; stripe < drawEndX; stripe++) {
            if (stripe >= 0 && stripe < COLS && transformY < depthBuffer[stripe]) {
                for (int y = max(0, drawStartY); y < min(ROWS, drawEndY); y++) {
                    screenBuf[y][stripe] = monsterChars[t]; // Glitchy monster
                }
            }
        }
    }
    
    // Print ASCII to Canvas
    if (dead) {
        canvas.fillSprite(TFT_BLACK);
        canvas.setTextColor(TFT_RED);
        canvas.setCursor(120, 80);
        canvas.print("IT CAUGHT YOU");
    } else {
        canvas.setTextColor(display.color565(200, 200, 150)); // Sickly yellow-white text!
        for(int y=0; y<ROWS; y++) {
            screenBuf[y][COLS] = '\0';
            canvas.drawString(screenBuf[y], 0, y * 8);
        }
    }
    
    canvas.pushSprite(0, 0);
}

void loop() {
    unsigned long now = millis();
    static unsigned long lastFrame = 0;
    
    if (dead) {
        if (btnDown(BTN_CENTER)) {
            posX = 2.5; posY = 2.5;
            monsterX = 10.5; monsterY = 10.5;
            dead = false;
        }
        renderFrame();
        delay(100);
        return;
    }
    
    // Controls
    if (btnDown(BTN_UP)) {
        if(!isWall(int(posX + dirX * moveSpeed * 2.0), int(posY))) posX += dirX * moveSpeed;
        if(!isWall(int(posX), int(posY + dirY * moveSpeed * 2.0))) posY += dirY * moveSpeed;
        wobble += 0.3;
    }
    if (btnDown(BTN_DOWN)) {
        if(!isWall(int(posX - dirX * moveSpeed * 2.0), int(posY))) posX -= dirX * moveSpeed;
        if(!isWall(int(posX), int(posY - dirY * moveSpeed * 2.0))) posY -= dirY * moveSpeed;
        wobble -= 0.3;
    }
    if (btnDown(BTN_LEFT)) {
        // Rotate left
        float oldDirX = dirX;
        dirX = dirX * cos(rotSpeed) - dirY * sin(rotSpeed);
        dirY = oldDirX * sin(rotSpeed) + dirY * cos(rotSpeed);
        float oldPlaneX = planeX;
        planeX = planeX * cos(rotSpeed) - planeY * sin(rotSpeed);
        planeY = oldPlaneX * sin(rotSpeed) + planeY * cos(rotSpeed);
    }
    if (btnDown(BTN_RIGHT)) {
        // Rotate right
        float oldDirX = dirX;
        dirX = dirX * cos(-rotSpeed) - dirY * sin(-rotSpeed);
        dirY = oldDirX * sin(-rotSpeed) + dirY * cos(-rotSpeed);
        float oldPlaneX = planeX;
        planeX = planeX * cos(-rotSpeed) - planeY * sin(-rotSpeed);
        planeY = oldPlaneX * sin(-rotSpeed) + planeY * cos(-rotSpeed);
    }
    
    // Update Monster
    float mdx = posX - monsterX;
    float mdy = posY - monsterY;
    float dist = sqrt(mdx*mdx + mdy*mdy);
    
    if (dist > 16.0) {
        // Teleport near player if lost
        float ang = random(0, 360) * 3.1415 / 180.0;
        float nx = posX + cos(ang) * 12.0;
        float ny = posY + sin(ang) * 12.0;
        if (!isWall((int)nx, (int)ny)) {
            monsterX = nx; monsterY = ny;
        }
    } else if (dist > 0.6) {
        // Walk towards player slowly
        float nx = monsterX + (mdx/dist) * (moveSpeed * 0.4);
        float ny = monsterY + (mdy/dist) * (moveSpeed * 0.4);
        if (!isWall((int)nx, (int)monsterY)) monsterX = nx;
        if (!isWall((int)monsterX, (int)ny)) monsterY = ny;
    } else {
        dead = true;
    }
    
    if (now - lastFrame >= 33) { // 30 FPS cap
        renderFrame();
        lastFrame = now;
    }
}
