#define LGFX_USE_V1
#include <LovyanGFX.hpp>
#include <Arduino.h>

// 1. Cấu hình Driver màn hình ngoài tùy chọn (LovyanGFX)
// Bạn có thể nối màn hình SPI ngoài vào ESP32-S3-N16R8 (ví dụ: ST7789 hoặc ILI9341)
class LGFX : public lgfx::LGFX_Device {
  lgfx::Panel_ST7789 _panel_instance; // Đổi sang Panel_ILI9341 nếu dùng màn hình ILI9341
  lgfx::Bus_SPI      _bus_instance;

public:
  LGFX(void) {
    {
      auto bus_cfg = _bus_instance.config();
      bus_cfg.spi_host = SPI2_HOST;
      bus_cfg.spi_mode = 0;
      bus_cfg.freq_write = 40000000;
      bus_cfg.freq_read  = 16000000;
      bus_cfg.pin_sclk = 36; // Cấu hình chân SPI CLK ngoài
      bus_cfg.pin_mosi = 35; // Cấu hình chân SPI MOSI ngoài
      bus_cfg.pin_miso = -1;
      bus_cfg.pin_dc   = 39; // Cấu hình chân DC ngoài
      _bus_instance.config(bus_cfg);
      _panel_instance.setBus(&_bus_instance);
    }
    {
      auto panel_cfg = _panel_instance.config();
      panel_cfg.pin_cs           = 7;  // Cấu hình chân CS ngoài
      panel_cfg.pin_rst          = 40; // Cấu hình chân RST ngoài
      panel_cfg.pin_busy         = -1;
      panel_cfg.panel_width      = 240;
      panel_cfg.panel_height     = 320;
      panel_cfg.offset_x         = 0;
      panel_cfg.offset_y         = 0;
      panel_cfg.invert           = true;
      panel_cfg.rgb_order        = false;
      _panel_instance.config(panel_cfg);
      setPanel(&_panel_instance);
    }
  }
};

namespace {

LGFX lcd;
LGFX_Sprite canvas(&lcd);
bool g_screen_enabled = false;

// Trạng thái nhận diện được từ M5Stick S3
bool g_sw_up = false;
bool g_sw_down = false;
bool g_sw_left = false;
bool g_sw_right = false;
bool g_sw_center = false;

bool g_m5_sd_mounted = false;
String g_m5_sd_type = "None";
String g_m5_sd_free = "0";
String g_m5_sd_test = "IDLE";

// Chân UART2 kết nối với M5Stick S3 (Grove)
// Nối chéo: RX2 (GPIO 18) vào TX (G10) của M5Stick S3
// Nối chéo: TX2 (GPIO 17) vào RX (G9) của M5Stick S3
constexpr int kPinRX2 = 18;
constexpr int kPinTX2 = 17;

// Hàm tính số Fibonacci lớn (Sử dụng CPU-heavy của Co-processor)
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

// Hàm test hiệu năng sắp xếp RAM lớn trên PSRAM (8MB PSRAM)
uint32_t benchmarkPSRAM(int size) {
  // Cấp phát mảng lớn trực tiếp trên PSRAM (SPIRAM)
  uint32_t* large_array = (uint32_t*)heap_caps_malloc(size * sizeof(uint32_t), MALLOC_CAP_SPIRAM);
  if (!large_array) {
    Serial.println("[Co-op] PSRAM allocation failed!");
    return 0;
  }

  // Khởi tạo dữ liệu ngẫu nhiên
  for (int i = 0; i < size; i++) {
    large_array[i] = random(0, 1000000);
  }

  // Chạy sắp xếp nổi bọt (Bubble Sort) hoặc tương đương để đẩy tải CPU lên 100%
  uint32_t start_ms = millis();
  for (int i = 0; i < size - 1; i++) {
    for (int j = 0; j < size - i - 1; j++) {
      if (large_array[j] > large_array[j+1]) {
        uint32_t temp = large_array[j];
        large_array[j] = large_array[j+1];
        large_array[j+1] = temp;
      }
    }
  }
  uint32_t elapsed = millis() - start_ms;

  heap_caps_free(large_array); // Giải phóng bộ nhớ PSRAM
  return elapsed;
}

void drawDashboard() {
  if (!g_screen_enabled) return;

  canvas.fillScreen(TFT_BLACK);
  canvas.fillRect(0, 0, 240, 20, 0x3186); // Header màu xanh navy
  canvas.setTextColor(TFT_WHITE);
  canvas.setTextDatum(top_left);
  canvas.drawString(" ESP32-S3-N16R8 CO-PROCESSOR", 4, 3);

  // Khối thông tin M5Stick S3 Status
  canvas.drawRect(5, 28, 230, 95, TFT_DARKGREY);
  canvas.setTextColor(TFT_CYAN);
  canvas.drawString("MASTER (M5STICKS3) STATUS", 10, 32);

  canvas.setTextColor(TFT_WHITE);
  canvas.drawString("SD Mounted:", 10, 48);
  if (g_m5_sd_mounted) {
    canvas.setTextColor(TFT_GREEN);
    canvas.drawString("YES", 100, 48);
  } else {
    canvas.setTextColor(TFT_RED);
    canvas.drawString("NO", 100, 48);
  }

  canvas.setTextColor(TFT_WHITE);
  canvas.drawString("SD Type:", 10, 64);
  canvas.setTextColor(TFT_YELLOW);
  canvas.drawString(g_m5_sd_type, 100, 64);

  canvas.setTextColor(TFT_WHITE);
  canvas.drawString("Free Space:", 10, 80);
  canvas.setTextColor(TFT_LIGHTGRAY);
  canvas.printf("%s MB", g_m5_sd_free.c_str());

  canvas.setTextColor(TFT_WHITE);
  canvas.drawString("SD Test:", 10, 96);
  canvas.setTextColor(TFT_ORANGE);
  canvas.drawString(g_m5_sd_test, 100, 96);

  // Khối hiển thị trực quan nút D-Pad 5 chiều
  canvas.drawRect(5, 130, 230, 105, TFT_DARKGREY);
  canvas.setTextColor(TFT_CYAN);
  canvas.drawString("REAL-TIME 5-WAY SWITCH MONITOR", 10, 134);

  // Vẽ các phím giả lập D-Pad
  auto drawKey = [](const char* label, bool pressed, int x, int y, int w, int h) {
    uint16_t boxColor = pressed ? TFT_GREEN : TFT_BLACK;
    uint16_t borderColor = pressed ? TFT_GREEN : TFT_LIGHTGRAY;
    uint16_t textColor = pressed ? TFT_BLACK : TFT_WHITE;
    canvas.fillRoundRect(x, y, w, h, 4, boxColor);
    canvas.drawRoundRect(x, y, w, h, 4, borderColor);
    canvas.setTextColor(textColor);
    canvas.setTextDatum(middle_center);
    canvas.drawString(label, x + w / 2, y + h / 2);
  };

  drawKey("UP",     g_sw_up,     100, 150, 40, 24);
  drawKey("DOWN",   g_sw_down,   100, 202, 40, 24);
  drawKey("LEFT",   g_sw_left,   55,  176, 40, 24);
  drawKey("RIGHT",  g_sw_right,  145, 176, 40, 24);
  drawKey("CTR",    g_sw_center, 100, 176, 40, 24);

  // Hiển thị trạng thái tải tính toán của Co-processor
  canvas.drawRect(5, 242, 230, 70, TFT_DARKGREY);
  canvas.setTextColor(TFT_CYAN);
  canvas.setTextDatum(top_left);
  canvas.drawString("CO-PROCESSING ENGAGED", 10, 246);
  canvas.setTextColor(TFT_WHITE);
  canvas.printf("PSRAM size: %d MB\n", ESP.getPsramSize() / (1024 * 1024));
  canvas.setCursor(10, 276);
  canvas.printf("Free PSRAM: %d KB\n", ESP.getFreePsram() / 1024);

  canvas.pushSprite(0, 0);
}

// Xử lý dữ liệu nhận được từ M5Stick S3
void handleM5Command(const String& cmd) {
  Serial.printf("[Co-op RX] Parsing: '%s'\n", cmd.c_str());

  if (cmd.startsWith("COOP:CONNECTED")) {
    Serial.println("[Co-op] Connection established with M5Stick S3!");
  }
  // 1. Phân tích phím nhấn
  else if (cmd.startsWith("SW:")) {
    int colon1 = cmd.indexOf(':');
    int colon2 = cmd.indexOf(':', colon1 + 1);
    if (colon1 != -1 && colon2 != -1) {
      String key = cmd.substring(colon1 + 1, colon2);
      bool pressed = cmd.substring(colon2 + 1).toInt() == 1;

      if (key == "UP") g_sw_up = pressed;
      else if (key == "DOWN") g_sw_down = pressed;
      else if (key == "LEFT") g_sw_left = pressed;
      else if (key == "RIGHT") g_sw_right = pressed;
      else if (key == "CENTER") g_sw_center = pressed;

      drawDashboard();
    }
  }
  // 2. Phân tích kết quả test SD Card
  else if (cmd.startsWith("SD_TEST:")) {
    g_m5_sd_test = cmd.substring(8);
    g_m5_sd_test.trim();
    drawDashboard();
  }
  // 3. Phân tích nhịp tim trạng thái định kỳ
  else if (cmd.startsWith("STATUS:")) {
    // Định dạng: STATUS:SD_MOUNTED=1:SD_TYPE=SDHC:SD_FREE=29800MB
    int mounted_idx = cmd.indexOf("SD_MOUNTED=");
    int type_idx = cmd.indexOf("SD_TYPE=");
    int free_idx = cmd.indexOf("SD_FREE=");

    if (mounted_idx != -1) {
      g_m5_sd_mounted = (cmd.substring(mounted_idx + 11, cmd.indexOf(':', mounted_idx)) == "1");
    }
    if (type_idx != -1) {
      g_m5_sd_type = cmd.substring(type_idx + 8, cmd.indexOf(':', type_idx));
    }
    if (free_idx != -1) {
      g_m5_sd_free = cmd.substring(free_idx + 8);
      g_m5_sd_free.replace("MB", "");
      g_m5_sd_free.trim();
    }
    drawDashboard();
  }
  // 4. Nhận lệnh tính toán tải nặng (Co-processing request)
  else if (cmd.startsWith("CALC:FIB:")) {
    int n = cmd.substring(9).toInt();
    Serial.printf("[Co-op CPU] Task received: Calculate Fibonacci for n=%d\n", n);
    
    uint32_t start = micros();
    uint64_t res = calculateFibonacci(n);
    uint32_t elapsed = micros() - start;

    Serial.printf("[Co-op CPU] Finished in %u us. Result: %llu\n", elapsed, res);
    // Gửi trả kết quả tính toán siêu tốc về cho M5Stick S3
    Serial2.printf("RESULT:FIB:%d:%llu:%uus\n", n, res, elapsed);
  }
  // 5. Nhận lệnh benchmark tốc độ ghi RAM PSRAM
  else if (cmd.startsWith("CALC:BENCH:")) {
    int array_size = cmd.substring(11).toInt();
    if (array_size <= 0) array_size = 500; // Mặc định 500 phần tử

    Serial.printf("[Co-op RAM] Task received: Sorting %d elements on external PSRAM...\n", array_size);
    uint32_t sort_time = benchmarkPSRAM(array_size);

    Serial.printf("[Co-op RAM] Sort completed in %u ms.\n", sort_time);
    Serial2.printf("RESULT:BENCH:%d:%ums\n", array_size, sort_time);
  }
}

} // namespace

void setup() {
  // Khởi động cổng giám sát USB
  Serial.begin(115200);
  delay(1000);

  Serial.println("\n=============================================");
  Serial.println("  ESP32-S3-N16R8 CO-PROCESSOR DISPATCHER      ");
  Serial.println("=============================================");

  // Khởi động cổng UART2 kết nối chéo sang M5Stick S3 (Baud: 115200)
  Serial2.begin(115200, SERIAL_8N1, kPinRX2, kPinTX2);
  Serial.printf("[UART] Initialized Serial2 Link (RX2=GPIO %d, TX2=GPIO %d)\n", kPinRX2, kPinTX2);

  // Kiểm tra bộ nhớ PSRAM trên ESP32-S3-N16R8
  if (psramInit()) {
    Serial.printf("[PSRAM] Found external OPI PSRAM! Total: %d MB\n", ESP.getPsramSize() / (1024 * 1024));
  } else {
    Serial.println("[PSRAM] External PSRAM failed to initialize or not present.");
  }

  // Khởi chạy LCD ngoài (tùy chọn)
  // Thử khởi tạo màn hình, nếu thành công cấu hình Sprite
  lcd.init();
  lcd.setRotation(1);
  canvas.createSprite(240, 320);
  g_screen_enabled = true;
  Serial.println("[Display] Optional external LCD initialized and double-buffered.");

  drawDashboard();
}

void loop() {
  // Lắng nghe dữ liệu dòng lệnh đổ về từ M5Stick S3
  if (Serial2.available()) {
    String command = Serial2.readStringUntil('\n');
    command.trim();
    if (command.length() > 0) {
      handleM5Command(command);
    }
  }

  // Nhận lệnh thủ công từ bàn phím cổng USB PC để test hiệu năng
  if (Serial.available()) {
    String cmd = Serial.readStringUntil('\n');
    cmd.trim();
    if (cmd.length() > 0) {
      handleM5Command(cmd);
    }
  }
}
