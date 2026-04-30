
#include <math.h>
#include "gbConfig.h"
#include "gbGlobals.h"

#include "osint.h"
#include "vecx.h"

#ifdef use_lib_cartdridge_flash_ram
#include "dataFlash/gbrom.h"
#endif

#include "esp32-hal-cpu.h"
#include <Arduino.h>
#include <M5StickCPlus2.h>
#include "dataFlash/gbbios.h"

// Display for M5StickC Plus2
#include "display.h"

#define EMU_TIMER 20 // the emulators heart beats at 20 milliseconds

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
        
        // Read joystick
        uint16_t xValue = joyc.getADCValue(0);
        
        // Check for up/down input
        unsigned long now = millis();
        if (now - lastInput > inputDelay) {
            bool moved = false;
            
            // Up
            if (xValue > 2800) {
                selectedIndex--;
                if (selectedIndex < 0) selectedIndex = max_list_rom - 1;
                moved = true;
            }
            // Down  
            else if (xValue < 1300) {
                selectedIndex++;
                if (selectedIndex >= max_list_rom) selectedIndex = 0;
                moved = true;
            }
            
            if (moved) {
                lastInput = now;
            }
        }
        
        // Select with BtnA
        if (M5.BtnA.wasPressed()) {
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
    
    // Read JoyC values
    uint16_t xValue = joyc.getADCValue(0);  // Vertical axis
    uint16_t yValue = joyc.getADCValue(1);  // Horizontal axis
    bool joyBtn = (joyc.getButtonStatus() == 0);  // JoyC button pressed (active low)
    
    // NEW BUTTON MAPPING:
    // JoyC Button = Button 1 (Primary fire/action)
    // BtnA = Button 2 (Secondary action)
    // PWR = Button 3 (Special action)
    // BtnB short = Button 4 (Rare games)
    // BtnB hold = Return to menu
    
    // Button 1 = JoyC button
    if (joyBtn){
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

    // Joystick control from JoyC with rotation support
    // JoyC ADC values: ~2048 centered, 0-4095 range
    // Vectrex VIA expects: nibbles 0-15 (center = 8)
    // Note: We're using 8-bit (0-255) but scaled to match nibble behavior
    
    // First, convert raw ADC to directional values
    uint8_t rawX, rawY;
    
    // Y-axis (horizontal on joystick): yValue controls this
    // Left = high values (>2800), Right = low values (<1300), Center = ~2048
    if (yValue > 2800) {
        rawX = 0;  // Full left
    } else if (yValue < 1300) {
        rawX = 255;  // Full right
    } else {
        rawX = map(yValue, 2800, 1300, 0, 255);
    }
    
    // X-axis (vertical on joystick): xValue controls this  
    // Up = high values (>2800), Down = low values (<1300), Center = ~2048
    if (xValue > 2800) {
        rawY = 255;  // Full up
    } else if (xValue < 1300) {
        rawY = 0;  // Full down
    } else {
        rawY = map(xValue, 1300, 2800, 0, 255);
    }
    
    // Rotate joystick based on screen orientation (landscape modes only)
    if (screenRotation == 3) {
        // Landscape 180 (upside down)
        alg_jch0 = 255 - rawX;
        alg_jch1 = 255 - rawY;
    } else {
        // Landscape (normal) - rotation 1
        alg_jch0 = rawX;
        alg_jch1 = rawY;
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
