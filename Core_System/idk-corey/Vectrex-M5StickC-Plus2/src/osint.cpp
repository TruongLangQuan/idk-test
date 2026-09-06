
#include <Arduino.h>
#include <M5StickCPlus2.h>
#include <math.h>
#include "gbConfig.h"
#include "gbGlobals.h"

#include "osint.h"
#include "vecx.h"

#ifdef use_lib_cartdridge_flash_ram
#include "dataFlash/gbrom.h"
#endif

#include "esp32-hal-cpu.h"
#include "dataFlash/gbbios.h"

// Display for M5StickC Plus2
#include "display.h"

#define EMU_TIMER 20 // the emulators heart beats at 20 milliseconds

// 5-way tactile switch pins
#define PIN_UP 32
#define PIN_DOWN 33
#define PIN_LEFT 25
#define PIN_RIGHT 26
#define PIN_CENTER 0

// ROM selection menu
static int showROMMenu() {
    int selectedIndex = 0;
    bool menuActive = true;
    unsigned long lastInput = 0;
    const int inputDelay = 200;
    static int lastDrawnIndex = -1;
    
    // Force redraw on menu entry
    lastDrawnIndex = -1;
    
    M5.Lcd.fillScreen(BLACK);
    M5.Lcd.setTextColor(GREEN, BLACK);
    M5.Lcd.setTextSize(1);
    
    while (menuActive) {
        M5.update();
        
        // Read 5-way switch
        bool moveUp = (digitalRead(PIN_UP) == LOW);
        bool moveDown = (digitalRead(PIN_DOWN) == LOW);
        
        // Check for up/down input
        unsigned long now = millis();
        if (now - lastInput > inputDelay) {
            bool moved = false;
            
            // Up
            if (moveUp) {
                selectedIndex--;
                if (selectedIndex < 0) selectedIndex = max_list_rom - 1;
                moved = true;
            }
            // Down  
            else if (moveDown) {
                selectedIndex++;
                if (selectedIndex >= max_list_rom) selectedIndex = 0;
                moved = true;
            }
            
            if (moved) {
                lastInput = now;
            }
        }
        
        // Select with BtnA or Center
        if (M5.BtnA.wasPressed() || (digitalRead(PIN_CENTER) == LOW)) {
            menuActive = false;
        }
        
        // Only redraw if selection changed or first time
        if (selectedIndex != lastDrawnIndex) {
            lastDrawnIndex = selectedIndex;
            
            // Draw menu
            M5.Lcd.fillScreen(BLACK);
            M5.Lcd.setCursor(0, 0);
            M5.Lcd.setTextSize(1);
            M5.Lcd.setTextColor(GREEN, BLACK);
            M5.Lcd.println("=== SELECT ROM ===");
            M5.Lcd.println("");
        
            // Calculate scrolling window (show 10 items at a time)
            const int itemsPerScreen = 10;
            int startIdx = selectedIndex - (itemsPerScreen / 2);
            
            // Keep window in bounds
            if (startIdx < 0) startIdx = 0;
            if (startIdx > max_list_rom - itemsPerScreen) {
                startIdx = max_list_rom - itemsPerScreen;
            }
            if (startIdx < 0) startIdx = 0;  // Handle case where total items < itemsPerScreen
            
            // Show ROMs in scrolling window
            for (int i = 0; i < itemsPerScreen && (startIdx + i) < max_list_rom; i++) {
                int idx = startIdx + i;
                
                if (idx == selectedIndex) {
                    M5.Lcd.setTextColor(BLACK, GREEN);
                    M5.Lcd.print(">");
                } else {
                    M5.Lcd.setTextColor(GREEN, BLACK);
                    M5.Lcd.print(" ");
                }
                
                // Truncate name if too long
                String name = String(gb_list_rom_title[idx]);
                if (name.length() > 26) {
                    name = name.substring(0, 23) + "...";
                }
                M5.Lcd.println(name);
            }
        
            // Show position indicator at bottom
            M5.Lcd.println("");
            M5.Lcd.setTextColor(GREEN, BLACK);
            M5.Lcd.printf("%d/%d ", selectedIndex + 1, max_list_rom);
            M5.Lcd.println("A:Select");
        }
        
        delay(50);
    }
    
    return selectedIndex;
}

unsigned char gb_load_new_rom = 1;
unsigned char gb_id_cur_rom = 0; // rom actual; Default 0 = Minestorm = only available option 
unsigned char gb_salir = 0;

static int screenx;
static int screeny;
static int scl_factor;
static int offx;
static int offy;

void swap(int *i, int *j)
{
    int t = *i;
    *i = *j;
    *j = t;
}

void swap_short(short int *i, short int *j)
{
    short int t = *i;
    *i = *j;
    *j = t;
}

inline void drawdot(int x, int y) {
    // Convert Vectrex coordinates to LCD
    int lcdX = vectrexToLcdX(x);
    int lcdY = vectrexToLcdY(y);
    displayDrawPixel(lcdX, lcdY, VECTOR_COLOR);
}

#define drawskip 1
void drawline(int x1, int y1, int x2, int y2) {
    int x, y, xe, ye;
    int dx = x2 - x1;
    int dy = y2 - y1;
    int dx1 = labs(dx); // long abs
    int dy1 = labs(dy);
    int px = 2 * dy1 - dx1;
    int py = 2 * dx1 - dy1;
    if (dy1 <= dx1) {
        if (dx >= 0) {
            x = x1;
            y = y1;
            xe = x2;
        } else {
            x = x2;
            y = y2;
            xe = x1;
        }
        drawdot(x, y);
        // return;

        for (int i = 0; x < xe; i += drawskip) {
            x = x + 1;
            if (px < 0) {
                px = px + 2 * dy1;
            } else {
                if ((dx < 0 && dy < 0) || (dx > 0 && dy > 0)) {
                    y = y + 1;
                } else {
                    y = y - 1;
                }
                px = px + 2 * (dy1 - dx1);
            }
            drawdot(x, y);
        }
    } else {
        if (dy >= 0) {
            x = x1;
            y = y1;
            ye = y2;
        } else {
            x = x2;
            y = y2;
            ye = y1;
        }
        drawdot(x, y);

        for (int i = 0; y < ye; i += drawskip) {
            y = y + 1;
            if (py <= 0) {
                py = py + 2 * dx1;
            } else {
                if ((dx < 0 && dy < 0) || (dx > 0 && dy > 0)) {
                    x = x + 1;
                } else {
                    x = x - 1;
                }
                py = py + 2 * (dx1 - dy1);
            }
            drawdot(x, y);
        }
    }
}

void osint_render() {
    // Clear framebuffer
    displayClear();
    
    // Draw vectors with bounding box clipping
    int x0, x1, y0, y1;
    for (int v = 0; v < vector_draw_cnt; v++) {
        x0 = (int)(vectors_draw[v].x0);
        y0 = (int)(vectors_draw[v].y0);
        x1 = (int)(vectors_draw[v].x1);
        y1 = (int)(vectors_draw[v].y1);

        // Skip vectors completely off-screen (Vectrex intentionally draws many)
        if ((x0 < 0 && x1 < 0) || (x0 > 33000 && x1 > 33000)) continue;
        if ((y0 < 0 && y1 < 0) || (y0 > 41000 && y1 > 41000)) continue;

        drawline(x0, y0, x1, y1);
    }

    // Single SPI transfer per frame
    displayPresent();
}


static void initLoadROM() {
#ifdef use_lib_rom_no_use_ram
#ifdef use_lib_log_serial
    Serial.printf("Load rom FLASH\n");
#endif
    // fflush(stdout);
    rom = gb_rom_bios;
#else
    memcpy(rom, gb_rom_bios, 8192);
#endif

#ifdef use_lib_cartdridge_no_use_ram
#else
    memset(cart, 0, sizeof(cart));
#endif

#ifdef use_lib_log_serial
    Serial.printf("Load cartdridge FLASH id:%d size:%d\n", gb_id_cur_rom, gb_list_cart_size[gb_id_cur_rom]);
#endif

    int topeCartBytes;
    if (gb_list_cart_size[gb_id_cur_rom] < 32768) {
        topeCartBytes = gb_list_cart_size[gb_id_cur_rom];
    } else {
        topeCartBytes = 32768;
#ifdef use_lib_log_serial
        Serial.printf("Excede tamanio Cartucho 32768 bytes\n");
#endif
    }

#ifdef use_lib_cartdridge_no_use_ram
    cart = gb_list_rom_data[gb_id_cur_rom];
#else
    memcpy(cart, gb_list_rom_data[gb_id_cur_rom], topeCartBytes);
#endif
}

static void readevents() {
    // Update M5 button state
    M5.update();
    
    // Check for hold BtnB (1 second) to return to menu
    static unsigned long btnBHoldStart = 0;
    if (M5.BtnB.isPressed()) {
        if (btnBHoldStart == 0) {
            btnBHoldStart = millis();
        } else if (millis() - btnBHoldStart > 1000) {
            // Held for 1 second - return to menu
            gb_load_new_rom = 1;
            btnBHoldStart = 0;
            return;
        }
    } else {
        btnBHoldStart = 0;
    }
    
    // Check for BtnA hold (1 second) to toggle rotation
    static unsigned long btnAHoldStart = 0;
    static bool rotationToggled = false;
    if (M5.BtnA.isPressed()) {
        if (btnAHoldStart == 0) {
            btnAHoldStart = millis();
        } else if (millis() - btnAHoldStart > 1000 && !rotationToggled) {
            // Held for 1 second - toggle rotation
            displayToggleRotation();
            rotationToggled = true;
        }
    } else {
        btnAHoldStart = 0;
        rotationToggled = false;
    }
    
    // Read 5-way switch
    bool up = (digitalRead(PIN_UP) == LOW);
    bool down = (digitalRead(PIN_DOWN) == LOW);
    bool left = (digitalRead(PIN_LEFT) == LOW);
    bool right = (digitalRead(PIN_RIGHT) == LOW);
    bool center = (digitalRead(PIN_CENTER) == LOW);
    
    // NEW BUTTON MAPPING:
    // Center = Button 1 (Primary fire/action)
    // BtnA = Button 2 (Secondary action)
    // PWR = Button 3 (Special action)
    // BtnB short = Button 4 (Rare games)
    // BtnB hold = Return to menu
    
    // Button 1 = Center switch
    if (center){
        snd_regs[14] &= ~0x01;
    }
    else{
        snd_regs[14] |= 0x01;
    }

    // Button 2 = BtnA
    if (M5.BtnA.isPressed()){
        snd_regs[14] &= ~0x02;
    }
    else
        snd_regs[14] |= 0x02;

    // Button 3 = PWR button
    if (M5.BtnPWR.isPressed()){
        snd_regs[14] &= ~0x04;
    }
    else
        snd_regs[14] |= 0x04;

    // Button 4 = BtnB (short press only, long hold returns to menu)
    if (M5.BtnB.isPressed()){
        snd_regs[14] &= ~0x08;
    }
    else
        snd_regs[14] |= 0x08;

    // Joystick control simulation from 5-way switch
    uint8_t rawX = 127;
    uint8_t rawY = 127;
    
    if (left) rawX = 0;
    if (right) rawX = 255;
    if (up) rawY = 255;
    if (down) rawY = 0;
    
    // Rotate joystick based on screen orientation (landscape modes only)
    if (screenRotation == 1) {
        // Landscape (normal)
        alg_jch0 = rawX;
        alg_jch1 = rawY;
    } else {
        // Landscape 180 (upside down) - rotation 3
        alg_jch0 = 255 - rawX;
        alg_jch1 = 255 - rawY;
    }
}

void osint_emuloop() {
    unsigned long cpu_begin, cpu_end;
    // Portar 	Uint32 next_time = SDL_GetTicks() + EMU_TIMER;
    unsigned int next_time = millis() + EMU_TIMER;

    vecx_reset();

    for (;;) {
        if (gb_load_new_rom == 1) {
            break;
        }

        cpu_begin = micros();
        // Run emulation with current frame time
        vecx_emu((VECTREX_MHZ / 1000) * EMU_TIMER, 0);
        cpu_end = micros();
        gb_fps_unified++;
        gb_stats_time_cur_unified = (cpu_end - cpu_begin);
        if (gb_stats_time_cur_unified < gb_stats_time_min_unified)
            gb_stats_time_min_unified = gb_stats_time_cur_unified;
        if (gb_stats_time_cur_unified > gb_stats_time_max_unified)
            gb_stats_time_max_unified = gb_stats_time_cur_unified;

        readevents();

        gb_currentTime = millis();
        if ((gb_currentTime - gb_fps_time_ini_unified) > 1000) {
            gb_fps_time_ini_unified = gb_currentTime;
            unsigned int aux_fps = gb_fps_unified - gb_fps_ini_unified;
            gb_fps_ini_unified = gb_fps_unified;
#ifdef use_lib_log_serial
            Serial.printf("fps:%d\n", aux_fps);
#endif
            gb_stats_time_min_unified = 500000;
            gb_stats_time_max_unified = 0;
            gb_stats_video_min_unified = 500000;
            gb_stats_video_max_unified = 0;
        }

        // Frame timing - run as fast as possible, don't delay
        unsigned int now = millis();
        next_time = now + EMU_TIMER;
    }
}

//********************************
void ImprimeMemoria() {
#ifdef use_lib_log_serial
    Serial.printf("VECTREX_MHZ:%d\n", VECTREX_MHZ);
    Serial.printf("VECTREX_PDECAY:%d\n", GetVECTREX_PDECAY());
    Serial.printf("VECTOR_CNT:%d\n", GetVECTOR_CNT());
    Serial.printf("vectors_set:%d bytes\n", GetSizeBytes_vectors_set());
    Serial.printf("VECTOR_HASH:%d\n", GetVECTOR_HASH());
    Serial.printf("vector_hash:%d bytes\n", GetSizeBytes_vector_hash());
#endif
    // fflush(stdout);
}

//********************************
int mainEmulator() {
    ImprimeMemoria();

    // Initialize display system
    displayInit();

#ifdef use_lib_log_serial
    Serial.printf("Sizeof long:%d\n", sizeof(long));
    Serial.printf("screenx:%d screeny:%d\n", screenx, screeny);
#endif

    // Show ROM selection menu at startup
    int selectedROM = showROMMenu();
    if (selectedROM >= 0) {
        gb_id_cur_rom = selectedROM;
    }
    
    while (gb_salir == 0) {
        if (gb_load_new_rom == 1) {
            gb_load_new_rom = 0;
            
            // Show ROM menu again
            int selectedROM = showROMMenu();
            if (selectedROM >= 0) {
                gb_id_cur_rom = selectedROM;
            }
            
            initLoadROM(); // Cambio de init que lo usa fabgl
            osint_emuloop();
        }
    }

    return 0;
}
