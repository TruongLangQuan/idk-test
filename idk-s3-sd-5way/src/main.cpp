#include <M5Unified.h>
#include <SPI.h>
#include <SD.h>
#include <FS.h>

namespace {

// SD Card Module SPI Pinout (G5, G4, G6, G7)
constexpr int kPinSD_CLK  = 5;
constexpr int kPinSD_MISO = 4;
constexpr int kPinSD_MOSI = 6;
constexpr int kPinSD_CS   = 7;

// 5-Way Tactile Switch Pins (GPIO G1, G2, G3, G8, G43)
constexpr int kPinSW_UP     = 1;
constexpr int kPinSW_DOWN   = 2;
constexpr int kPinSW_LEFT   = 3;
constexpr int kPinSW_RIGHT  = 8;
constexpr int kPinSW_CENTER = 43;

struct Button {
  const char* name;
  int pin;
  bool pressed;
  int grid_x;
  int grid_y;
};

// 5-Way Switch Button Layout for Visual D-Pad
Button g_buttons[] = {
  {"UP",     kPinSW_UP,     false, 165, 30},
  {"DOWN",   kPinSW_DOWN,   false, 165, 80},
  {"LEFT",   kPinSW_LEFT,   false, 125, 55},
  {"RIGHT",  kPinSW_RIGHT,  false, 205, 55},
  {"CENTER", kPinSW_CENTER, false, 165, 55}
};

// SD Card State Variables
bool g_sd_mounted = false;
String g_sd_type = "None";
uint64_t g_sd_total_bytes = 0;
uint64_t g_sd_free_bytes = 0;
String g_sd_test_result = "IDLE";

// Co-op Benchmark State Variables
bool g_show_benchmark = false;
bool g_coop_connected = false;
uint32_t g_bench_local_fib_us = 0;
uint32_t g_bench_coop_fib_us = 0;
uint32_t g_bench_coop_sort_ms = 0;

uint64_t calculateFibonacci(int n) {
  if (n <= 0) return 0;
  if (n == 1) return 1;
  uint64_t a = 0, b = 1, c;
  for (int i = 2; i <= n; i++) {
    c = a + b;
    a = b;
    b = c;
  }
  return b;
}

// Debounce helper variables
uint32_t g_last_poll_ms = 0;
constexpr uint32_t kPollIntervalMs = 20;

// Initialize SD Card
bool initSDCard() {
  Serial.println("[SD] Initializing SPI bus...");
  SPI.begin(kPinSD_CLK, kPinSD_MISO, kPinSD_MOSI, kPinSD_CS);

  Serial.println("[SD] Mounting SD Card...");
  if (!SD.begin(kPinSD_CS, SPI, 4000000)) { // 4MHz frequency for safety
    Serial.println("[SD] Mount failed.");
    g_sd_mounted = false;
    g_sd_type = "Fail";
    return false;
  }

  g_sd_mounted = true;
  uint8_t cardType = SD.cardType();
  if (cardType == CARD_MMC) {
    g_sd_type = "MMC";
  } else if (cardType == CARD_SD) {
    g_sd_type = "SDSC";
  } else if (cardType == CARD_SDHC) {
    g_sd_type = "SDHC";
  } else {
    g_sd_type = "Unknown";
  }

  g_sd_total_bytes = SD.totalBytes();
  g_sd_free_bytes = SD.totalBytes() - SD.usedBytes();

  Serial.printf("[SD] Card Type: %s\n", g_sd_type.c_str());
  Serial.printf("[SD] Card Size: %llu MB\n", g_sd_total_bytes / (1024 * 1024));
  Serial.printf("[SD] Free Space: %llu MB\n", g_sd_free_bytes / (1024 * 1024));

  return true;
}

// Perform active Read/Write check on SD Card
void verifySDCard() {
  if (!g_sd_mounted) {
    g_sd_test_result = "NO SD";
    Serial.println("[SD Test] Cannot run test, SD not mounted.");
    Serial2.printf("SD_TEST:%s\n", g_sd_test_result.c_str());
    return;
  }

  g_sd_test_result = "RUNNING";
  Serial.println("[SD Test] Starting write test...");
  Serial2.printf("SD_TEST:%s\n", g_sd_test_result.c_str());
  
  File testFile = SD.open("/sd_test.txt", FILE_WRITE);
  if (!testFile) {
    g_sd_test_result = "WR_FAIL";
    Serial.println("[SD Test] Failed to open file for writing.");
    Serial2.printf("SD_TEST:%s\n", g_sd_test_result.c_str());
    return;
  }

  String testMessage = "M5StickS3 Co-op Test successful! Millis: " + String(millis());
  if (testFile.println(testMessage)) {
    Serial.println("[SD Test] Write success.");
  } else {
    g_sd_test_result = "WR_FAIL";
    Serial.println("[SD Test] Write operation failed.");
    Serial2.printf("SD_TEST:%s\n", g_sd_test_result.c_str());
    testFile.close();
    return;
  }
  testFile.close();

  Serial.println("[SD Test] Starting read validation...");
  testFile = SD.open("/sd_test.txt", FILE_READ);
  if (!testFile) {
    g_sd_test_result = "RD_FAIL";
    Serial.println("[SD Test] Failed to open file for reading.");
    Serial2.printf("SD_TEST:%s\n", g_sd_test_result.c_str());
    return;
  }

  String readLine = testFile.readStringUntil('\n');
  readLine.trim();
  testFile.close();

  Serial.printf("[SD Test] Read string: '%s'\n", readLine.c_str());
  if (readLine.startsWith("M5StickS3 Co-op Test successful!")) {
    g_sd_test_result = "PASS";
    Serial.println("[SD Test] Validation Passed!");
    Serial2.printf("SD_TEST:%s\n", g_sd_test_result.c_str());
  } else {
    g_sd_test_result = "VERIFY_ERR";
    Serial.println("[SD Test] Validation Failed - content mismatch.");
    Serial2.printf("SD_TEST:%s\n", g_sd_test_result.c_str());
  }
}

// Draw the UI Screen
void drawUI() {
  M5.Display.startWrite();
  M5.Display.fillScreen(TFT_BLACK);

  // Top Title Bar
  M5.Display.fillRect(0, 0, 240, 18, 0x1A6D); // Dark slate header
  M5.Display.setTextColor(TFT_WHITE);
  M5.Display.setTextDatum(top_left);
  M5.Display.drawString(" STICKS3 CO-OP INTERACTIVE TEST", 4, 3);

  // Left Column - SD Card Status OR Co-op Benchmark Status
  M5.Display.drawRect(5, 24, 110, 105, TFT_DARKGREY);
  
  if (!g_show_benchmark) {
    M5.Display.setTextColor(TFT_CYAN);
    M5.Display.drawString("SD CARD INFO", 10, 28);
    
    M5.Display.setTextColor(TFT_WHITE);
    M5.Display.drawString("Mount:", 10, 44);
    if (g_sd_mounted) {
      M5.Display.setTextColor(TFT_GREEN);
      M5.Display.drawString("OK", 55, 44);
    } else {
      M5.Display.setTextColor(TFT_RED);
      M5.Display.drawString("FAIL", 55, 44);
    }

    M5.Display.setTextColor(TFT_WHITE);
    M5.Display.drawString("Type:", 10, 60);
    M5.Display.setTextColor(TFT_YELLOW);
    M5.Display.drawString(g_sd_type, 55, 60);

    M5.Display.setTextColor(TFT_WHITE);
    M5.Display.drawString("Size:", 10, 76);
    if (g_sd_mounted) {
      M5.Display.setTextColor(TFT_LIGHTGRAY);
      M5.Display.printf("%lluMB", g_sd_total_bytes / (1024 * 1024));
    } else {
      M5.Display.drawString("-", 55, 76);
    }

    M5.Display.setTextColor(TFT_WHITE);
    M5.Display.drawString("Test:", 10, 92);
    if (g_sd_test_result == "PASS") {
      M5.Display.setTextColor(TFT_GREEN);
    } else if (g_sd_test_result == "IDLE") {
      M5.Display.setTextColor(TFT_LIGHTGRAY);
    } else {
      M5.Display.setTextColor(TFT_RED);
    }
    M5.Display.drawString(g_sd_test_result, 55, 92);

    M5.Display.setTextColor(TFT_DARKGREY);
    M5.Display.drawString("[CTR] key to test SD", 8, 112);
  } else {
    M5.Display.setTextColor(TFT_CYAN);
    M5.Display.drawString("CO-OP BENCH", 10, 28);

    M5.Display.setTextColor(TFT_WHITE);
    M5.Display.drawString("Link:", 10, 44);
    if (g_coop_connected) {
      M5.Display.setTextColor(TFT_GREEN);
      M5.Display.drawString("OK", 55, 44);
    } else {
      M5.Display.setTextColor(TFT_RED);
      M5.Display.drawString("FAIL", 55, 44);
    }

    M5.Display.setTextColor(TFT_WHITE);
    M5.Display.drawString("L_Fib:", 10, 60);
    M5.Display.setTextColor(TFT_YELLOW);
    if (g_bench_local_fib_us > 0) {
      M5.Display.printf("%uus", g_bench_local_fib_us);
    } else {
      M5.Display.drawString("-", 55, 60);
    }

    M5.Display.setTextColor(TFT_WHITE);
    M5.Display.drawString("C_Fib:", 10, 76);
    M5.Display.setTextColor(TFT_GREEN);
    if (g_bench_coop_fib_us > 0) {
      M5.Display.printf("%uus", g_bench_coop_fib_us);
    } else {
      M5.Display.drawString("-", 55, 76);
    }

    M5.Display.setTextColor(TFT_WHITE);
    M5.Display.drawString("Sort:", 10, 92);
    M5.Display.setTextColor(TFT_ORANGE);
    if (g_bench_coop_sort_ms > 0) {
      M5.Display.printf("%ums", g_bench_coop_sort_ms);
    } else {
      M5.Display.drawString("-", 55, 92);
    }

    M5.Display.setTextColor(TFT_DARKGREY);
    M5.Display.drawString("[UP]Fib  [DN]Sort  [L/R]Pg", 6, 112);
  }

  // Right Column - 5-Way switch D-pad visualizer
  M5.Display.drawRect(120, 24, 115, 105, TFT_DARKGREY);
  M5.Display.setTextColor(TFT_CYAN);
  M5.Display.drawString("5-WAY SWITCH", 125, 28);

  // Draw D-pad buttons
  for (const auto& btn : g_buttons) {
    uint16_t boxColor = btn.pressed ? TFT_GREEN : TFT_BLACK;
    uint16_t borderColor = btn.pressed ? TFT_GREEN : TFT_LIGHTGRAY;
    uint16_t textColor = btn.pressed ? TFT_BLACK : TFT_WHITE;

    M5.Display.fillRoundRect(btn.grid_x, btn.grid_y, 32, 20, 4, boxColor);
    M5.Display.drawRoundRect(btn.grid_x, btn.grid_y, 32, 20, 4, borderColor);
    
    // Draw character initial centered
    M5.Display.setTextColor(textColor);
    String letter = String(btn.name[0]);
    if (strcmp(btn.name, "CENTER") == 0) {
      letter = "CTR";
    }
    M5.Display.setTextDatum(middle_center);
    M5.Display.drawString(letter, btn.grid_x + 16, btn.grid_y + 10);
  }

  M5.Display.endWrite();
}

} // namespace

void setup() {
  auto cfg = M5.config();
  M5.begin(cfg);
  
  M5.Display.setRotation(1); // Set to landscape mode
  M5.Display.setBrightness(180);
  
  Serial.begin(115200);
  delay(500);
  
  Serial.println("\n==================================");
  Serial.println("   M5STICK S3 CO-OP TEST RUNNER   ");
  Serial.println("==================================");

  // Initialize Serial2 for ESP32-S3 co-processor communication
  // Grove port pins on M5StickS3: G9 (RX), G10 (TX)
  Serial2.begin(115200, SERIAL_8N1, 9, 10);
  Serial.println("[Co-op] Initialized Serial2 (G9=RX, G10=TX) at 115200 baud.");
  Serial2.println("COOP:CONNECTED");

  // Configure switch pins as INPUT_PULLUP
  for (const auto& btn : g_buttons) {
    pinMode(btn.pin, INPUT_PULLUP);
    Serial.printf("[Switch] Registered pin %d for %s\n", btn.pin, btn.name);
  }

  // Attempt initial SD card mount
  initSDCard();

  // Render initial GUI
  drawUI();
}

void loop() {
  M5.update();
  uint32_t now = millis();
  bool state_changed = false;

  // Lắng nghe dữ liệu phản hồi từ mạch phụ Co-processor (ESP32-S3-N16R8)
  if (Serial2.available()) {
    String resp = Serial2.readStringUntil('\n');
    resp.trim();
    if (resp.length() > 0) {
      Serial.printf("[Co-op RX] Received: '%s'\n", resp.c_str());
      
      if (resp.startsWith("COOP:CONNECTED")) {
        g_coop_connected = true;
        state_changed = true;
      }
      else if (resp.startsWith("RESULT:FIB:")) {
        // Định dạng: RESULT:FIB:<N>:<VAL>:<TIMING>us
        int colon1 = resp.indexOf(':', 7);
        int colon2 = resp.indexOf(':', colon1 + 1);
        int colon3 = resp.indexOf(':', colon2 + 1);
        String timingStr = (colon3 != -1) ? resp.substring(colon2 + 1, colon3) : resp.substring(colon2 + 1);
        timingStr.replace("us", "");
        g_bench_coop_fib_us = timingStr.toInt();
        g_coop_connected = true;
        state_changed = true;
      }
      else if (resp.startsWith("RESULT:BENCH:")) {
        // Định dạng: RESULT:BENCH:<SIZE>:<TIMING>ms
        int colon1 = resp.indexOf(':', 10);
        int colon2 = resp.indexOf(':', colon1 + 1);
        String timingStr = (colon2 != -1) ? resp.substring(colon1 + 1, colon2) : resp.substring(colon1 + 1);
        timingStr.replace("ms", "");
        g_bench_coop_sort_ms = timingStr.toInt();
        g_coop_connected = true;
        state_changed = true;
      }
    }
  }

  if (now - g_last_poll_ms >= kPollIntervalMs) {
    g_last_poll_ms = now;
    
    // Quét trạng thái nút bấm 5 hướng
    for (auto& btn : g_buttons) {
      bool current_pressed = (digitalRead(btn.pin) == LOW);
      if (current_pressed != btn.pressed) {
        btn.pressed = current_pressed;
        state_changed = true;
        
        Serial.printf("[Switch] Event: %s was %s\n", btn.name, current_pressed ? "PRESSED" : "RELEASED");
        
        // Gửi sự kiện nút bấm sang Co-processor
        Serial2.printf("SW:%s:%d\n", btn.name, current_pressed ? 1 : 0);
        
        if (current_pressed) {
          // 1. Phím UP: Kích hoạt đo hiệu năng so sánh Fibonacci (Local vs Co-op)
          if (strcmp(btn.name, "UP") == 0) {
            g_show_benchmark = true; // Chuyển sang trang hiển thị hiệu năng
            
            // Tính cục bộ (Local) trên M5Stick S3
            uint32_t start = micros();
            uint64_t val = calculateFibonacci(35); // Tính số Fib 35
            g_bench_local_fib_us = micros() - start;
            
            // Gửi yêu cầu tính toán sang ESP32-S3-N16R8 Co-processor
            g_bench_coop_fib_us = 0; // Đặt về 0 (đang chờ kết quả)
            Serial2.println("CALC:FIB:35");
            Serial.printf("[Bench] Fib35 Local: %u us. Sending request to ESP32-S3-N16R8...\n", g_bench_local_fib_us);
          }
          
          // 2. Phím DOWN: Kích hoạt benchmark sắp xếp 1000 phần tử trên PSRAM của Co-processor
          else if (strcmp(btn.name, "DOWN") == 0) {
            g_show_benchmark = true; // Chuyển sang trang hiển thị hiệu năng
            
            g_bench_coop_sort_ms = 0; // Đặt về 0 (đang chờ kết quả)
            Serial2.println("CALC:BENCH:1000"); // Yêu cầu sắp xếp 1000 phần tử trên PSRAM ngoài
            Serial.println("[Bench] Sent request to sort 1000 elements on external PSRAM to ESP32-S3-N16R8.");
          }
          
          // 3. Phím CENTER: Chạy test thẻ nhớ SD cục bộ
          else if (strcmp(btn.name, "CENTER") == 0) {
            verifySDCard();
          }
          
          // 4. Phím LEFT/RIGHT: Chuyển đổi qua lại giữa trang SD Card và trang Benchmark
          else if (strcmp(btn.name, "LEFT") == 0 || strcmp(btn.name, "RIGHT") == 0) {
            g_show_benchmark = !g_show_benchmark;
          }
        }
      }
    }
    
    // Gửi tín hiệu nhịp tim định kỳ (Heartbeat) sang Co-processor mỗi 2 giây
    static uint32_t last_coop_hb = 0;
    if (now - last_coop_hb >= 2000) {
      last_coop_hb = now;
      Serial2.printf("STATUS:SD_MOUNTED=%d:SD_TYPE=%s:SD_FREE=%lluMB\n", 
                     g_sd_mounted ? 1 : 0, g_sd_type.c_str(), g_sd_free_bytes / (1024 * 1024));
    }
    
    // Vẽ lại giao diện nếu có thay đổi trạng thái
    if (state_changed) {
      drawUI();
    }
  }
}
