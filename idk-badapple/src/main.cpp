#include <M5Unified.h>
#include <SPIFFS.h>
#include <SD.h>
#include <SPI.h>

// Hardware specific: M5StickC Plus 2 has shared Pin 14 for LCD_DC and SD_CS.
// High-speed toggling can corrupt SD cards if not handled.
static constexpr int kSD_CS = 14; 

static constexpr int kCols = 48;
static constexpr int kRows = 22;
static constexpr int kCellW = 5;
static constexpr int kCellH = 6;
static constexpr int kFrameSize = 1056; // 48 * 22

M5Canvas canvas(&M5.Display);
uint32_t last_time = 0;
const uint32_t frame_delay = 1000 / 8; // 8 FPS
File videoFile;
uint8_t frame_buffer[kFrameSize];

void setup() {
    pinMode(14, OUTPUT);
    digitalWrite(14, HIGH);
    delay(100);

    // Safety: ensure Pin 14 is HIGH (SD deselected) as early as possible
    pinMode(kSD_CS, OUTPUT);
    digitalWrite(kSD_CS, HIGH);
    delay(100);

    auto cfg = M5.config();
    cfg.internal_imu = true;
    cfg.internal_rtc = true;
    cfg.internal_mic = false; // Disable MIC (G0 conflicts with SD SCK)
    cfg.internal_spk = false; // Disable SPK
    M5.begin(cfg);

    M5.Display.setBaseColor(TFT_BLACK);
    M5.Display.setRotation(3);
    M5.Display.setBrightness(128);

    canvas.setPsram(true);
    canvas.setColorDepth(8);
    canvas.createSprite(240, 135);
    canvas.setFont(&fonts::Font0);
    canvas.setTextSize(1);
    
    // Initialize the global SPI bus exactly like idk-video
    SPI.begin(0, 36, 26, 14);

    bool videoLoaded = false;

    // Try SD Card First (CS is Pin 14)
    if (SD.begin(14, SPI, 15000000)) {
        videoFile = SD.open("/badapple.bin", FILE_READ);
        if (videoFile) {
            videoLoaded = true;
            Serial.println("Loaded from SD Card");
        }
    }

    // Fallback to SPIFFS
    if (!videoLoaded) {
        if (SPIFFS.begin(true)) {
            videoFile = SPIFFS.open("/badapple.bin", FILE_READ);
            if (videoFile) {
                videoLoaded = true;
                Serial.println("Loaded from SPIFFS");
            }
        }
    }

    if (!videoLoaded) {
        canvas.fillSprite(TFT_BLACK);
        canvas.setTextColor(TFT_RED);
        canvas.drawString("badapple.bin not found!", 10, 60);
        canvas.drawString("Checked SD & SPIFFS", 10, 75);
        canvas.pushSprite(0, 0);
        while(1) delay(100);
    }

    last_time = millis();
}

void loop() {
    M5.update();
    
    uint32_t now = millis();
    if (now - last_time >= frame_delay) {
        last_time = now;
        
        // Read next frame
        if (videoFile.available() < kFrameSize) {
            videoFile.seek(0); // Loop back to start
        }
        
        size_t bytesRead = 0;
        while (bytesRead < kFrameSize) {
            int r = videoFile.read(frame_buffer + bytesRead, kFrameSize - bytesRead);
            if (r <= 0) break;
            bytesRead += r;
        }
        
        canvas.startWrite();
        canvas.fillSprite(TFT_BLACK);
        
        for (int r = 0; r < kRows; ++r) {
            for (int c = 0; c < kCols; ++c) {
                uint8_t ch = frame_buffer[r * kCols + c];
                uint16_t color = TFT_WHITE;
                
                if (ch == ' ' || ch == '.') color = TFT_DARKGREY;
                else if (ch == ':' || ch == '-') color = TFT_LIGHTGREY;
                
                canvas.setTextColor(color, TFT_BLACK);
                canvas.drawChar(ch, c * kCellW, r * kCellH);
            }
        }
        
        canvas.endWrite();
        canvas.pushSprite(0, 0);
    }
    
    if (M5.BtnA.wasPressed() || M5.BtnB.wasPressed() || M5.BtnPWR.wasPressed()) {
        if (videoFile) videoFile.seek(0); // Reset
    }
    
    delay(1);
}
