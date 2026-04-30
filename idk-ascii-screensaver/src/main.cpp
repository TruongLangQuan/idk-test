#include <M5Unified.h>
#include <math.h>

namespace {

// ─── ASCII Screensaver Modes ────────────────────────────────────
enum Saver {
  AsciiMatrix,
  AsciiFire,
  AsciiMandelbrot,
  AsciiPlasma,
  AsciiStarfield,
  AsciiConway,
  AsciiSpinner,
  AsciiPulse,
  AsciiRain,
  AsciiCode,
  AsciiScroll,
  AsciiWave,
  AsciiBlocks,
  AsciiDNA,
  AsciiAquarium,
  Count,
};

static const char* kNames[] = {
    "matrix",   "fire",      "mandelbrot", "plasma",   "starfield",
    "conway",   "spinner",   "pulse",      "rain",     "code",
    "scroll",   "wave",      "blocks",     "dna",      "aquarium",
};

static Saver g_saver = AsciiMatrix;
static uint32_t g_last_tick = 0;
static int g_frame = 0;
static uint32_t g_last_fire_update = 0;
static constexpr uint32_t kFireInterval = 50;  // ~20 FPS for fire effect

// ─── State arrays ──────────────────────────────────────────────
static uint8_t g_fire[30][16];
static uint8_t g_fire_next[30][16];  // Double buffer for smooth transitions
static uint8_t g_life[30][16];
static uint8_t g_life_next[30][16];
static char g_fire_char[30][16];      // Cached character display
static uint16_t g_fire_color[30][16];  // Cached color display
static int g_rain_y[30];
static int g_rain_speed[30];
static char g_rain_char[30];

const char* saverName() { return kNames[g_saver]; }

void clearTopBar() {
  M5.Display.fillRect(0, 0, 240, 14, TFT_BLACK);
  M5.Display.setCursor(4, 4);
  M5.Display.setTextColor(TFT_YELLOW, TFT_BLACK);
  M5.Display.printf("ascii-%s [%d/%d]", saverName(), g_saver + 1, Count);
}

// ─── Init functions ─────────────────────────────────────────────
void initRain() {
  for (int i = 0; i < 30; ++i) {
    g_rain_y[i] = -(esp_random() % 180);
    g_rain_speed[i] = 2 + (esp_random() % 5);
    g_rain_char[i] = '!' + (esp_random() % 90);
  }
}

void initFire() {
  memset(g_fire, 0, sizeof(g_fire));
  memset(g_fire_next, 0, sizeof(g_fire_next));
  memset(g_fire_char, 0, sizeof(g_fire_char));
  memset(g_fire_color, 0, sizeof(g_fire_color));
}

void initLife() {
  for (int x = 0; x < 30; ++x)
    for (int y = 0; y < 16; ++y)
      g_life[x][y] = esp_random() % 2;
}

void initSaver() {
  if (g_saver == AsciiRain || g_saver == AsciiMatrix) initRain();
  if (g_saver == AsciiFire) initFire();
  if (g_saver == AsciiConway) initLife();
  g_frame = 0;
}

// ─── Matrix (vertical code rain) ────────────────────────────────
void drawMatrix() {
  M5.Display.fillScreen(TFT_BLACK);
  M5.Display.setTextSize(1);
  for (int col = 0; col < 30; ++col) {
    for (int trail = 0; trail < 8; ++trail) {
      int y = g_rain_y[col] - trail * 10;
      if (y < 14 || y > 135) continue;
      int brightness = 255 - trail * 30;
      if (brightness < 40) brightness = 40;
      uint16_t c = trail == 0 ? TFT_WHITE : M5.Display.color565(0, brightness, brightness / 4);
      M5.Display.setTextColor(c, TFT_BLACK);
      M5.Display.setCursor(col * 8, y);
      char ch = 33 + ((g_frame + col * 7 + trail * 3) % 93);
      M5.Display.print(ch);
    }
    g_rain_y[col] += g_rain_speed[col];
    if (g_rain_y[col] > 200) {
      g_rain_y[col] = -(esp_random() % 120);
      g_rain_speed[col] = 2 + (esp_random() % 5);
    }
  }
}

// ─── Fire effect ────────────────────────────────────────────────
void drawFire() {
  const uint32_t now = millis();
  if (now - g_last_fire_update < kFireInterval) return;
  g_last_fire_update = now;

  M5.Display.setTextSize(1);
  const char* heat = " .:-=+*#%@";
  int hlen = strlen(heat);

  // Clear top bar (only on mode switch, not every frame)
  M5.Display.fillRect(0, 14, 240, 121, TFT_BLACK);

  // Propagate upward with cooling - update model
  for (int x = 0; x < 30; ++x) {
    g_fire[x][15] = 60 + (esp_random() % 40);
  }

  for (int y = 0; y < 15; ++y) {
    for (int x = 0; x < 30; ++x) {
      int sum = 0;
      int cnt = 0;
      for (int dx = -1; dx <= 1; ++dx) {
        int nx = x + dx;
        if (nx >= 0 && nx < 30) {
          sum += g_fire[nx][y + 1];
          cnt++;
        }
      }
      int avg = sum / cnt;
      int cool = (esp_random() % 6);
      g_fire[x][y] = avg > cool ? avg - cool : 0;
    }
  }

  // Render with double buffering - only update changed cells
  for (int y = 0; y < 16; ++y) {
    for (int x = 0; x < 30; ++x) {
      int v = g_fire[x][y];
      int ci = (v * (hlen - 1)) / 100;
      if (ci >= hlen) ci = hlen - 1;
      if (ci < 0) ci = 0;

      uint16_t color;
      if (v > 70) color = TFT_YELLOW;
      else if (v > 50) color = TFT_ORANGE;
      else if (v > 30) color = TFT_RED;
      else if (v > 10) color = M5.Display.color565(100, 20, 0);
      else color = M5.Display.color565(30, 5, 0);

      char new_char = heat[ci];

      // Only redraw if character or color changed
      if (g_fire_char[x][y] != new_char || g_fire_color[x][y] != color) {
        g_fire_char[x][y] = new_char;
        g_fire_color[x][y] = color;
        M5.Display.setTextColor(color, TFT_BLACK);
        M5.Display.setCursor(x * 8, 14 + y * 7);
        M5.Display.print(new_char);
      }
    }
  }
}

// ─── Mandelbrot fractal ─────────────────────────────────────────
void drawMandelbrot() {
  M5.Display.fillScreen(TFT_BLACK);
  M5.Display.setTextSize(1);
  const char* shade = " .:-=+*#%@";
  int slen = strlen(shade);
  float zoom = 2.5f + sinf(g_frame * 0.02f) * 0.8f;
  float cx = -0.5f + sinf(g_frame * 0.01f) * 0.2f;
  float cy = sinf(g_frame * 0.015f) * 0.2f;

  for (int row = 0; row < 10; ++row) {
    M5.Display.setCursor(0, 16 + row * 11);
    for (int col = 0; col < 30; ++col) {
      float x0 = (col - 15.0f) / (15.0f / zoom) + cx;
      float y0 = (row - 5.0f) / (5.0f / zoom) + cy;
      float x = 0, y = 0;
      int iter = 0;
      while (x*x + y*y <= 4.0f && iter < 20) {
        float xt = x*x - y*y + x0;
        y = 2*x*y + y0;
        x = xt;
        iter++;
      }
      int ci = (iter * (slen - 1)) / 20;
      uint16_t color = iter < 20 ? M5.Display.color565(iter * 12, iter * 8, 255 - iter * 12) : TFT_BLACK;
      M5.Display.setTextColor(color, TFT_BLACK);
      M5.Display.print(shade[ci]);
    }
  }
}

// ─── Plasma ─────────────────────────────────────────────────────
void drawPlasma() {
  M5.Display.fillScreen(TFT_BLACK);
  M5.Display.setTextSize(1);
  const char* chars = ".:;+=xX#@";
  int clen = strlen(chars);
  float t = g_frame * 0.1f;

  for (int y = 0; y < 10; ++y) {
    M5.Display.setCursor(0, 16 + y * 11);
    for (int x = 0; x < 30; ++x) {
      float v = sinf(x * 0.3f + t) + sinf(y * 0.4f + t * 0.7f);
      v += sinf((x + y) * 0.2f + t * 0.5f) + sinf(sqrtf(x*x + y*y) * 0.3f);
      v = (v + 4.0f) / 8.0f;  // normalize to 0-1
      int ci = (int)(v * (clen - 1));
      if (ci >= clen) ci = clen - 1;
      if (ci < 0) ci = 0;

      int r = (int)((sinf(v * 3.14f) + 1) * 127);
      int g = (int)((cosf(v * 3.14f + 1) + 1) * 127);
      int b = (int)((sinf(v * 3.14f + 2) + 1) * 127);
      M5.Display.setTextColor(M5.Display.color565(r, g, b), TFT_BLACK);
      M5.Display.print(chars[ci]);
    }
  }
}

// ─── Starfield ──────────────────────────────────────────────────
void drawStarfield() {
  M5.Display.fillScreen(TFT_BLACK);
  M5.Display.setTextSize(1);
  for (int i = 0; i < 40; ++i) {
    int x = (int)((i * 37 + g_frame * (1 + i % 4)) % 240);
    int y = 14 + (int)((i * 73 + g_frame * (1 + i % 3)) % 121);
    char c = (i % 4 == 0) ? '*' : ((i % 3 == 0) ? '+' : '.');
    int speed = 1 + (i % 4);
    int bright = 100 + speed * 35;
    M5.Display.setTextColor(M5.Display.color565(bright, bright, bright), TFT_BLACK);
    M5.Display.setCursor(x, y);
    M5.Display.print(c);
  }
}

// ─── Conway's Game of Life ──────────────────────────────────────
void drawConway() {
  M5.Display.fillScreen(TFT_BLACK);
  M5.Display.setTextSize(1);

  for (int x = 0; x < 30; ++x) {
    for (int y = 0; y < 12; ++y) {
      if (g_life[x][y]) {
        M5.Display.setTextColor(TFT_GREEN, TFT_BLACK);
        M5.Display.setCursor(x * 8, 16 + y * 10);
        M5.Display.print('#');
      }
    }
  }

  // Step life
  for (int x = 0; x < 30; ++x) {
    for (int y = 0; y < 12; ++y) {
      int n = 0;
      for (int dx = -1; dx <= 1; ++dx)
        for (int dy = -1; dy <= 1; ++dy) {
          if (!dx && !dy) continue;
          int nx = (x + dx + 30) % 30;
          int ny = (y + dy + 12) % 12;
          n += g_life[nx][ny] ? 1 : 0;
        }
      g_life_next[x][y] = (n == 3 || (g_life[x][y] && n == 2)) ? 1 : 0;
    }
  }
  memcpy(g_life, g_life_next, sizeof(g_life));

  // Re-seed if dead
  int alive = 0;
  for (int x = 0; x < 30; ++x)
    for (int y = 0; y < 12; ++y) alive += g_life[x][y];
  if (alive < 3) initLife();
}

// ─── Spinner ────────────────────────────────────────────────────
void drawSpinner() {
  M5.Display.fillScreen(TFT_BLACK);
  M5.Display.setTextSize(2);

  const char* spinners[] = {"|", "/", "-", "\\"};
  const char* braille[] = {"⠋", "⠙", "⠹", "⠸", "⠼", "⠴", "⠦", "⠧", "⠇", "⠏"};
  const char* dots = ".oOo";

  int phase = g_frame % 4;
  M5.Display.setTextColor(TFT_CYAN, TFT_BLACK);
  M5.Display.setCursor(100, 40);
  M5.Display.print(spinners[phase]);

  M5.Display.setTextColor(TFT_GREEN, TFT_BLACK);
  M5.Display.setCursor(40, 70);
  M5.Display.print(dots[phase]);
  M5.Display.print(dots[(phase + 1) % 4]);
  M5.Display.print(dots[(phase + 2) % 4]);

  M5.Display.setTextSize(1);
  M5.Display.setTextColor(TFT_YELLOW, TFT_BLACK);
  M5.Display.setCursor(80, 100);
  M5.Display.printf("Loading %c", spinners[phase][0]);
}

// ─── Pulse (heartbeat) ─────────────────────────────────────────
void drawPulse() {
  M5.Display.fillScreen(TFT_BLACK);
  M5.Display.setTextSize(1);
  M5.Display.setTextColor(TFT_GREEN, TFT_BLACK);

  // ECG-like waveform
  for (int x = 0; x < 30; ++x) {
    int phase = (x + g_frame) % 30;
    int y;
    if (phase == 8) y = -4;
    else if (phase == 9) y = 6;
    else if (phase == 10) y = -8;
    else if (phase == 11) y = 3;
    else if (phase == 12) y = -1;
    else y = 0;

    M5.Display.setCursor(x * 8, 65 + y * 3);
    M5.Display.print(y != 0 ? '^' : '-');
  }

  M5.Display.setTextColor(TFT_RED, TFT_BLACK);
  M5.Display.setCursor(80, 30);
  int bpm = 60 + (int)(sinf(g_frame * 0.05f) * 15);
  M5.Display.printf("BPM: %d", bpm);

  // Heart ASCII
  M5.Display.setTextColor(((g_frame / 4) % 2) ? TFT_RED : TFT_MAROON, TFT_BLACK);
  M5.Display.setCursor(90, 95);
  M5.Display.print("<3");
}

// ─── Rain ───────────────────────────────────────────────────────
void drawRain() {
  M5.Display.fillScreen(TFT_BLACK);
  M5.Display.setTextSize(1);
  const char* chars = "01#@*~+=-./:;";
  for (int i = 0; i < 30; ++i) {
    int x = (i * 8) % 240;
    int y = (g_frame * 3 + i * 4) % 121 + 14;
    char c = chars[(g_frame + i) % strlen(chars)];
    uint16_t color = M5.Display.color565(0, 200 + (i % 55), (i * 12) % 256);
    M5.Display.setTextColor(color, TFT_BLACK);
    M5.Display.setCursor(x, y);
    M5.Display.print(c);
  }
}

// ─── Code ───────────────────────────────────────────────────────
void drawCode() {
  M5.Display.fillScreen(TFT_BLACK);
  M5.Display.setTextSize(1);
  M5.Display.setTextColor(0x07E0, TFT_BLACK);
  const char* lines[] = {
      "void loop() {",
      "  M5.update();",
      "  if(btn) {",
      "    draw();",
      "  }",
      "  delay(10);",
      "}",
      "#include <M5>",
  };
  int offset = (g_frame / 6) % 8;
  for (int i = 0; i < 8; ++i) {
    M5.Display.setCursor(4, 16 + i * 14);
    M5.Display.print(lines[(i + offset) % 8]);
  }
}

// ─── Scroll ─────────────────────────────────────────────────────
void drawScroll() {
  M5.Display.fillScreen(TFT_BLACK);
  M5.Display.setTextSize(2);
  const char* text = "  ASCII SCREENSAVER  ";
  int len = strlen(text);
  int pixLen = len * 12;
  int pos = g_frame * 2 % (pixLen + 240);

  for (int i = 0; i < len; ++i) {
    int x = 240 - pos + i * 12;
    if (x < -12 || x > 240) continue;
    int r = (i * 37 + g_frame * 3) % 256;
    int g = (i * 73 + g_frame * 5) % 256;
    int b = (i * 101 + g_frame * 7) % 256;
    M5.Display.setTextColor(M5.Display.color565(r, g, b), TFT_BLACK);
    M5.Display.setCursor(x, 55);
    M5.Display.print(text[i]);
  }
}

// ─── Wave ───────────────────────────────────────────────────────
void drawWave() {
  M5.Display.fillScreen(TFT_BLACK);
  M5.Display.setTextSize(1);
  const char wave[] = "~-~-~-~-~-~-~-~-~-~-~-~-~-~-";
  for (int row = 0; row < 8; ++row) {
    int phase = (g_frame + row * 4) % 240;
    int offset = (int)(sinf(phase * 3.14159f / 120.0f) * 40.0f);
    M5.Display.setTextColor(
      M5.Display.color565(200 - row * 20, 100 + row * 10, 150),
      TFT_BLACK
    );
    M5.Display.setCursor(20 + offset, 20 + row * 12);
    M5.Display.print(wave);
  }
}

// ─── Blocks ─────────────────────────────────────────────────────
void drawBlocks() {
  M5.Display.fillScreen(TFT_BLACK);
  M5.Display.setTextSize(1);
  for (int y = 0; y < 8; ++y) {
    for (int x = 0; x < 30; ++x) {
      int idx = ((g_frame + x + y) % 4);
      int py = 20 + y * 12;
      int px = x * 8;
      uint16_t color = M5.Display.color565(
        ((x + g_frame) * 8) % 256,
        ((y + g_frame) * 8) % 256,
        ((x ^ y ^ g_frame) * 12) % 256
      );
      const char blockChars[] = "#*+.";
      M5.Display.setTextColor(color, TFT_BLACK);
      M5.Display.setCursor(px, py);
      M5.Display.print(blockChars[idx]);
    }
  }
}

// ─── DNA Helix ──────────────────────────────────────────────────
void drawDNA() {
  M5.Display.fillScreen(TFT_BLACK);
  M5.Display.setTextSize(1);
  const char bases[] = "ATCG";
  float t = g_frame * 0.15f;

  for (int y = 0; y < 10; ++y) {
    float phase = t + y * 0.6f;
    int x1 = 15 + (int)(sinf(phase) * 10.0f);
    int x2 = 15 + (int)(sinf(phase + 3.14159f) * 10.0f);

    // Left strand
    M5.Display.setTextColor(TFT_CYAN, TFT_BLACK);
    M5.Display.setCursor(x1 * 8, 16 + y * 11);
    M5.Display.print(bases[(y + g_frame) % 4]);

    // Right strand
    M5.Display.setTextColor(TFT_MAGENTA, TFT_BLACK);
    M5.Display.setCursor(x2 * 8, 16 + y * 11);
    M5.Display.print(bases[(y + g_frame + 2) % 4]);

    // Connection
    if (abs(x1 - x2) < 4) {
      int mx = ((x1 + x2) / 2) * 8;
      M5.Display.setTextColor(TFT_DARKGREY, TFT_BLACK);
      M5.Display.setCursor(mx, 16 + y * 11);
      M5.Display.print('-');
    }
  }
}

// ─── Aquarium ───────────────────────────────────────────────────
void drawAquarium() {
  M5.Display.fillScreen(M5.Display.color565(0, 0, 30));
  M5.Display.setTextSize(1);

  // Fish swimming
  for (int i = 0; i < 6; ++i) {
    int x = (g_frame * (2 + i % 3) + i * 40) % 260 - 20;
    int y = 20 + i * 18 + (int)(sinf(g_frame * 0.1f + i) * 5);
    M5.Display.setTextColor(
      M5.Display.color565(200 + (i * 30) % 55, 150 + (i * 50) % 105, 50),
      M5.Display.color565(0, 0, 30)
    );
    M5.Display.setCursor(x, y);
    M5.Display.print((i % 2) ? "><>" : "<><");
  }

  // Bubbles
  for (int i = 0; i < 5; ++i) {
    int x = 30 + i * 45;
    int y = 135 - (g_frame * 2 + i * 20) % 130;
    M5.Display.setTextColor(TFT_CYAN, M5.Display.color565(0, 0, 30));
    M5.Display.setCursor(x, y);
    M5.Display.print('o');
  }

  // Seaweed
  for (int i = 0; i < 4; ++i) {
    int x = 20 + i * 60;
    for (int j = 0; j < 3; ++j) {
      int sway = (int)(sinf(g_frame * 0.08f + i + j * 0.5f) * 2);
      M5.Display.setTextColor(TFT_GREEN, M5.Display.color565(0, 0, 30));
      M5.Display.setCursor(x + sway, 105 + j * 10);
      M5.Display.print((j + g_frame / 3) % 2 ? '(' : ')');
    }
  }
}

// ─── Draw dispatcher ────────────────────────────────────────────
void drawCurrent() {
  switch (g_saver) {
    case AsciiMatrix:      drawMatrix(); break;
    case AsciiFire:        drawFire(); break;
    case AsciiMandelbrot:  drawMandelbrot(); break;
    case AsciiPlasma:      drawPlasma(); break;
    case AsciiStarfield:   drawStarfield(); break;
    case AsciiConway:      drawConway(); break;
    case AsciiSpinner:     drawSpinner(); break;
    case AsciiPulse:       drawPulse(); break;
    case AsciiRain:        drawRain(); break;
    case AsciiCode:        drawCode(); break;
    case AsciiScroll:      drawScroll(); break;
    case AsciiWave:        drawWave(); break;
    case AsciiBlocks:      drawBlocks(); break;
    case AsciiDNA:         drawDNA(); break;
    case AsciiAquarium:    drawAquarium(); break;
    default: break;
  }
  clearTopBar();
}

}  // namespace

void setup() {
  auto cfg = M5.config();
  M5.begin(cfg);
  M5.Display.setRotation(3);
  M5.Display.setBrightness(180);
  initSaver();
  drawCurrent();
}

void loop() {
  M5.update();
  if (M5.BtnA.wasPressed()) {
    g_saver = static_cast<Saver>((g_saver + 1) % Count);
    initSaver();
    M5.Display.fillScreen(TFT_BLACK);
  }
  if (M5.BtnB.wasPressed()) {
    g_saver = static_cast<Saver>((g_saver + Count - 1) % Count);
    initSaver();
    M5.Display.fillScreen(TFT_BLACK);
  }
  if (millis() - g_last_tick > 70) {
    g_last_tick = millis();
    g_frame++;
    drawCurrent();
  }
  delay(10);
}
