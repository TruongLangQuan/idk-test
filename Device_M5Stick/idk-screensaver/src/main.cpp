#include <M5Unified.h>
#include <math.h>

namespace {

enum Saver {
  BashAlpha,
  BashBouncing,
  BashCuteSaver,
  BashFireworks,
  BashLife,
  BashMatrix,
  BashPipes,
  BashRain,
  BashSpeaky,
  BashStars,
  BashTunnel,
  BashVibe,
  TTAnt,
  TTBalls,
  TTBubble,
  TTCube,
  TTLife,
  TTPipes3D,
  TTRings,
  TTSand,
  TTSplits,
  TTTunnel,
  // ─── ASCII-based screensavers ──────────────────────
  AsciiRain,
  AsciiCode,
  AsciiScroll,
  AsciiWave,
  AsciiBlocks,
  Count,
};

static const char* kNames[] = {
    "bash-alpha",   "bash-bouncing", "bash-cutesaver", "bash-fireworks", "bash-life",   "bash-matrix",
    "bash-pipes",   "bash-rain",     "bash-speaky",    "bash-stars",     "bash-tunnel", "bash-vibe",
    "tt-ant",       "tt-balls",      "tt-bubble",      "tt-cube",        "tt-life",     "tt-pipes3d",
    "tt-rings",     "tt-sand",       "tt-splits",      "tt-tunnel",
    "ascii-rain",   "ascii-code",    "ascii-scroll",   "ascii-wave",     "ascii-blocks",
};

static Saver g_saver = BashAlpha;
static uint32_t g_last_tick = 0;
static int g_frame = 0;
static int g_rain_x[20];
static int g_rain_y[20];
static int g_star_x[32];
static int g_star_y[32];
static int g_star_speed[32];
static int g_ball_x[8];
static int g_ball_y[8];
static int g_ball_vx[8];
static int g_ball_vy[8];
static uint8_t g_life[20][14];
static uint8_t g_life_next[20][14];
static uint8_t g_sand[30][18];
static int g_ant_x = 15;
static int g_ant_y = 9;
static int g_ant_dir = 0;
static int g_speaky_phase = 0;

const char* saverName() { return kNames[g_saver]; }

void clearTopBar() {
  M5.Display.fillRect(0, 0, 240, 14, TFT_BLACK);
  M5.Display.setCursor(4, 4);
  M5.Display.setTextColor(TFT_YELLOW, TFT_BLACK);
  M5.Display.printf("%s", saverName());
}

void initRain() {
  for (int i = 0; i < 20; ++i) {
    g_rain_x[i] = i * 12;
    g_rain_y[i] = -(esp_random() % 180);
  }
}

void initStars() {
  for (int i = 0; i < 32; ++i) {
    g_star_x[i] = esp_random() % 240;
    g_star_y[i] = 14 + (esp_random() % 121);
    g_star_speed[i] = 1 + (esp_random() % 4);
  }
}

void initBalls() {
  for (int i = 0; i < 8; ++i) {
    g_ball_x[i] = 18 + i * 24;
    g_ball_y[i] = 18 + i * 10;
    g_ball_vx[i] = (i % 2) ? 2 : -2;
    g_ball_vy[i] = (i % 3) ? 1 : -1;
  }
}

void initLife() {
  for (int x = 0; x < 20; ++x) {
    for (int y = 0; y < 14; ++y) g_life[x][y] = esp_random() % 2;
  }
}

void initSand() {
  memset(g_sand, 0, sizeof(g_sand));
  for (int i = 0; i < 180; ++i) g_sand[esp_random() % 30][esp_random() % 8] = 1;
}

void initAnt() {
  memset(g_life, 0, sizeof(g_life));
  g_ant_x = 15;
  g_ant_y = 9;
  g_ant_dir = 0;
}

void initSaver() {
  if (g_saver == BashMatrix || g_saver == BashRain) initRain();
  if (g_saver == BashStars || g_saver == BashFireworks || g_saver == BashAlpha) initStars();
  if (g_saver == BashBouncing || g_saver == TTBalls || g_saver == TTBubble || g_saver == BashCuteSaver) initBalls();
  if (g_saver == BashLife || g_saver == TTLife) initLife();
  if (g_saver == TTSand) initSand();
  if (g_saver == TTAnt) initAnt();
  // ─── ASCII screensavers need no special init ──────────
  // Just reset frame counter
  g_speaky_phase = 0;
}

void drawMatrixLike() {
  M5.Display.fillScreen(TFT_BLACK);
  for (int i = 0; i < 20; ++i) {
    for (int j = 0; j < 7; ++j) {
      int y = g_rain_y[i] - j * 9;
      if (y < 12 || y > 135) continue;
      uint16_t c = j == 0 ? TFT_WHITE : M5.Display.color565(24, 200 - j * 24, 50);
      M5.Display.setTextColor(c, TFT_BLACK);
      M5.Display.setCursor(g_rain_x[i], y);
      M5.Display.print(char('A' + ((g_frame + i + j) % 26)));
    }
    g_rain_y[i] += 7;
    if (g_rain_y[i] > 190) g_rain_y[i] = -(esp_random() % 90);
  }
}

void drawAlpha() {
  M5.Display.fillScreen(TFT_BLACK);
  for (int i = 0; i < 180; ++i) {
    int x = esp_random() % 240;
    int y = 14 + (esp_random() % 121);
    uint16_t c = M5.Display.color565(esp_random() % 255, esp_random() % 255, esp_random() % 255);
    M5.Display.drawPixel(x, y, c);
  }
}

void drawRain() {
  M5.Display.fillScreen(TFT_BLACK);
  for (int i = 0; i < 20; ++i) {
    M5.Display.drawLine(g_rain_x[i], g_rain_y[i], g_rain_x[i] - 2, g_rain_y[i] + 8, TFT_CYAN);
    g_rain_y[i] += 10;
    if (g_rain_y[i] > 160) g_rain_y[i] = -(esp_random() % 90);
  }
}

void drawStars() {
  M5.Display.fillScreen(TFT_BLACK);
  for (int i = 0; i < 32; ++i) {
    M5.Display.drawPixel(g_star_x[i], g_star_y[i], (i % 3) ? TFT_WHITE : TFT_CYAN);
    g_star_x[i] -= g_star_speed[i];
    if (g_star_x[i] < 0) {
      g_star_x[i] = 239;
      g_star_y[i] = 14 + (esp_random() % 121);
    }
  }
}

void drawFireworks() {
  M5.Display.fillScreen(TFT_BLACK);
  int cx = 120 + static_cast<int>(sinf(g_frame * 0.07f) * 40.0f);
  int cy = 68 + static_cast<int>(cosf(g_frame * 0.05f) * 18.0f);
  for (int i = 0; i < 24; ++i) {
    float a = (2.0f * PI * i) / 24.0f;
    int r = 8 + ((g_frame * 2 + i) % 38);
    int x = cx + cosf(a) * r;
    int y = cy + sinf(a) * r;
    M5.Display.drawLine(cx, cy, x, y, (i % 2) ? TFT_MAGENTA : TFT_YELLOW);
  }
}

void drawBouncing(bool filled) {
  M5.Display.fillScreen(TFT_BLACK);
  for (int i = 0; i < 8; ++i) {
    int r = 4 + i % 3;
    if (filled) M5.Display.fillCircle(g_ball_x[i], g_ball_y[i], r, (i % 2) ? TFT_GREEN : TFT_CYAN);
    else M5.Display.drawCircle(g_ball_x[i], g_ball_y[i], r + 2, TFT_CYAN);
    g_ball_x[i] += g_ball_vx[i];
    g_ball_y[i] += g_ball_vy[i];
    if (g_ball_x[i] < 8 || g_ball_x[i] > 232) g_ball_vx[i] = -g_ball_vx[i];
    if (g_ball_y[i] < 18 || g_ball_y[i] > 126) g_ball_vy[i] = -g_ball_vy[i];
  }
}

void drawCuteSaver() {
  M5.Display.fillScreen(TFT_BLACK);
  for (int i = 0; i < 5; ++i) {
    int x = 30 + i * 38 + static_cast<int>(sinf((g_frame + i * 5) * 0.12f) * 6.0f);
    int y = 60 + static_cast<int>(cosf((g_frame + i * 9) * 0.11f) * 12.0f);
    M5.Display.fillCircle(x, y, 9, TFT_PINK);
    M5.Display.fillCircle(x - 8, y - 8, 4, TFT_PINK);
    M5.Display.fillCircle(x + 8, y - 8, 4, TFT_PINK);
    M5.Display.fillCircle(x - 3, y - 2, 1, TFT_BLACK);
    M5.Display.fillCircle(x + 3, y - 2, 1, TFT_BLACK);
    M5.Display.drawLine(x - 3, y + 4, x + 3, y + 4, TFT_BLACK);
  }
}

void drawLife(uint16_t color) {
  M5.Display.fillScreen(TFT_BLACK);
  const int cell = 8;
  const int ox = 40;
  const int oy = 16;
  for (int x = 0; x < 20; ++x) {
    for (int y = 0; y < 14; ++y) {
      if (g_life[x][y]) M5.Display.fillRect(ox + x * cell, oy + y * cell, cell - 1, cell - 1, color);
    }
  }
  for (int x = 0; x < 20; ++x) {
    for (int y = 0; y < 14; ++y) {
      int n = 0;
      for (int oxn = -1; oxn <= 1; ++oxn) {
        for (int oyn = -1; oyn <= 1; ++oyn) {
          if (!oxn && !oyn) continue;
          int nx = (x + oxn + 20) % 20;
          int ny = (y + oyn + 14) % 14;
          n += g_life[nx][ny] ? 1 : 0;
        }
      }
      g_life_next[x][y] = (n == 3 || (g_life[x][y] && n == 2)) ? 1 : 0;
    }
  }
  memcpy(g_life, g_life_next, sizeof(g_life));
}

void drawPipes2D() {
  M5.Display.fillScreen(TFT_BLACK);
  int cx = 120;
  int cy = 68;
  for (int i = 0; i < 7; ++i) {
    float a = (g_frame * 0.08f) + i * 0.8f;
    int x = cx + cosf(a) * (20 + i * 12);
    int y = cy + sinf(a * 1.3f) * (10 + i * 7);
    M5.Display.drawCircle(x, y, 5 + (i % 3), TFT_CYAN);
    if (i > 0) {
      float b = (g_frame * 0.08f) + (i - 1) * 0.8f;
      int px = cx + cosf(b) * (20 + (i - 1) * 12);
      int py = cy + sinf(b * 1.3f) * (10 + (i - 1) * 7);
      M5.Display.drawLine(px, py, x, y, TFT_GREEN);
    }
  }
}

void drawPipes3D() {
  M5.Display.fillScreen(TFT_BLACK);
  int cx = 120;
  int cy = 68;
  for (int i = 0; i < 8; ++i) {
    float a = g_frame * 0.06f + i * 0.7f;
    int x = cx + cosf(a) * (15 + i * 10);
    int y = cy + sinf(a * 1.2f) * (8 + i * 6);
    M5.Display.drawRoundRect(x - 4, y - 4, 8, 8, 2, TFT_CYAN);
    if (i > 0) {
      float b = g_frame * 0.06f + (i - 1) * 0.7f;
      int px = cx + cosf(b) * (15 + (i - 1) * 10);
      int py = cy + sinf(b * 1.2f) * (8 + (i - 1) * 6);
      M5.Display.drawLine(px, py, x, y, TFT_MAGENTA);
      M5.Display.drawLine(px + 3, py - 3, x + 3, y - 3, TFT_GREEN);
    }
  }
}

void drawTunnel(bool colorful) {
  M5.Display.fillScreen(TFT_BLACK);
  for (int i = 0; i < 12; ++i) {
    int r = 8 + i * 9 + (g_frame % 9);
    uint16_t c = colorful ? ((i % 2) ? TFT_MAGENTA : TFT_BLUE) : TFT_CYAN;
    M5.Display.drawRoundRect(120 - r, 68 - r / 2, r * 2, r, 4, c);
  }
}

void drawSpeaky() {
  static const char* kWords[] = {"HELLO", "WOW", "IDK", "M5", "NICE", "GO"};
  M5.Display.fillScreen(TFT_BLACK);
  M5.Display.fillRoundRect(50, 40, 140, 54, 8, TFT_WHITE);
  M5.Display.fillTriangle(100, 94, 118, 94, 108, 108, TFT_WHITE);
  M5.Display.setTextColor(TFT_BLACK, TFT_WHITE);
  M5.Display.setCursor(78, 60);
  M5.Display.print(kWords[g_speaky_phase % 6]);
  g_speaky_phase++;
}

void drawVibe() {
  M5.Display.fillScreen(TFT_BLACK);
  for (int x = 0; x < 240; x += 8) {
    int h = 20 + static_cast<int>((sinf((g_frame + x) * 0.08f) + 1.0f) * 30.0f);
    uint16_t c = M5.Display.color565(100 + (x % 120), 40 + (x % 80), 120 + (x % 100));
    M5.Display.fillRect(x, 120 - h, 6, h, c);
  }
}

void drawSand() {
  M5.Display.fillScreen(TFT_BLACK);
  const int cell = 8;
  const int oy = 8;
  for (int y = 16; y >= 0; --y) {
    for (int x = 0; x < 30; ++x) {
      if (!g_sand[x][y]) continue;
      if (y + 1 < 18 && !g_sand[x][y + 1]) {
        g_sand[x][y] = 0;
        g_sand[x][y + 1] = 1;
      } else if (y + 1 < 18 && x > 0 && !g_sand[x - 1][y + 1]) {
        g_sand[x][y] = 0;
        g_sand[x - 1][y + 1] = 1;
      } else if (y + 1 < 18 && x + 1 < 30 && !g_sand[x + 1][y + 1]) {
        g_sand[x][y] = 0;
        g_sand[x + 1][y + 1] = 1;
      }
    }
  }
  if (g_frame % 2 == 0) g_sand[esp_random() % 30][0] = 1;
  for (int x = 0; x < 30; ++x) {
    for (int y = 0; y < 18; ++y) {
      if (g_sand[x][y]) M5.Display.fillRect(x * cell, oy + y * cell, cell - 1, cell - 1, TFT_YELLOW);
    }
  }
}

void drawAnt() {
  M5.Display.fillScreen(TFT_BLACK);
  const int cell = 8;
  const int ox = 40;
  const int oy = 16;
  uint8_t& tile = g_life[g_ant_x % 20][g_ant_y % 14];
  if (tile) g_ant_dir = (g_ant_dir + 3) % 4;
  else g_ant_dir = (g_ant_dir + 1) % 4;
  tile = !tile;
  if (g_ant_dir == 0) g_ant_y = (g_ant_y + 13) % 14;
  if (g_ant_dir == 1) g_ant_x = (g_ant_x + 1) % 20;
  if (g_ant_dir == 2) g_ant_y = (g_ant_y + 1) % 14;
  if (g_ant_dir == 3) g_ant_x = (g_ant_x + 19) % 20;
  for (int x = 0; x < 20; ++x) {
    for (int y = 0; y < 14; ++y) {
      if (g_life[x][y]) M5.Display.fillRect(ox + x * cell, oy + y * cell, cell - 1, cell - 1, TFT_WHITE);
    }
  }
  M5.Display.fillCircle(ox + g_ant_x * cell + 3, oy + g_ant_y * cell + 3, 3, TFT_RED);
}

void drawRings() {
  M5.Display.fillScreen(TFT_BLACK);
  for (int i = 0; i < 10; ++i) {
    int rx = 20 + i * 10 + (g_frame % 12);
    int ry = 8 + i * 5;
    M5.Display.drawEllipse(120, 68, rx, ry, (i % 2) ? TFT_CYAN : TFT_MAGENTA);
  }
}

void drawBubble() {
  M5.Display.fillScreen(TFT_BLACK);
  for (int i = 0; i < 8; ++i) {
    int r = 5 + (i % 3) * 2;
    M5.Display.drawCircle(g_ball_x[i], g_ball_y[i], r, TFT_CYAN);
    M5.Display.drawPixel(g_ball_x[i] - 1, g_ball_y[i] - 1, TFT_WHITE);
    g_ball_y[i] -= 1 + (i % 2);
    if (g_ball_y[i] < 10) {
      g_ball_y[i] = 130;
      g_ball_x[i] = 20 + (esp_random() % 200);
    }
  }
}

void drawCube() {
  M5.Display.fillScreen(TFT_BLACK);
  int cx = 120;
  int cy = 68;
  int r1 = 22;
  int r2 = 38;
  for (int i = 0; i < 4; ++i) {
    float a = g_frame * 0.05f + i * PI / 2.0f;
    float b = g_frame * 0.05f + (i + 1) * PI / 2.0f;
    int x1 = cx + cosf(a) * r1;
    int y1 = cy + sinf(a) * r1;
    int x2 = cx + cosf(b) * r1;
    int y2 = cy + sinf(b) * r1;
    int x3 = cx + cosf(a + 0.5f) * r2;
    int y3 = cy + sinf(a + 0.5f) * r2;
    int x4 = cx + cosf(b + 0.5f) * r2;
    int y4 = cy + sinf(b + 0.5f) * r2;
    M5.Display.drawLine(x1, y1, x2, y2, TFT_GREEN);
    M5.Display.drawLine(x3, y3, x4, y4, TFT_CYAN);
    M5.Display.drawLine(x1, y1, x3, y3, TFT_MAGENTA);
  }
}

void drawSplits() {
  M5.Display.fillScreen(TFT_BLACK);
  for (int i = 0; i < 18; ++i) {
    int x = (g_frame * 7 + i * 13) % 240;
    int y = 18 + (i * 6) % 108;
    M5.Display.drawLine(120, 68, x, y, (i % 2) ? TFT_YELLOW : TFT_MAGENTA);
    M5.Display.drawLine(x, y, x + ((i % 3) - 1) * 10, y + 12, TFT_CYAN);
  }
}

// ─── ASCII-based screensavers ──────────────────────────────────

void drawAsciiRain() {
  M5.Display.fillScreen(TFT_BLACK);
  M5.Display.setTextSize(1);
  const char* chars = "01#@*~+=-./:;,!?\\|/";
  for (int i = 0; i < 30; ++i) {
    int x = (i * 8) % 240;
    int y = (g_frame * 3 + i * 4) % 135 + 14;
    char c = chars[(g_frame + i) % strlen(chars)];
    uint16_t color = M5.Display.color565(0, 200 + (i % 55), (i * 12) % 256);
    M5.Display.setTextColor(color, TFT_BLACK);
    M5.Display.setCursor(x, y);
    M5.Display.print(c);
  }
}

void drawAsciiCode() {
  M5.Display.fillScreen(TFT_BLACK);
  M5.Display.setTextSize(1);
  M5.Display.setTextColor(0x07E0, TFT_BLACK);  // green text
  const char* lines[] = {
      "int x = 42;",
      "if(x > 0) {",
      "  loop();",
      "}",
      "for(;;) {}"
  };
  int offset = (g_frame / 4) % 5;
  for (int i = 0; i < 5; ++i) {
    int lineX = ((i - offset + 5) % 5) * 4 - 4;
    if (lineX >= 0 && lineX < 240) {
      M5.Display.setCursor(4 + lineX, 20 + i * 16);
      M5.Display.print(lines[(i + offset) % 5]);
    }
  }
}

void drawAsciiScroll() {
  M5.Display.fillScreen(TFT_BLACK);
  M5.Display.setTextSize(1);
  const char* text = "   ASCII SCREENSAVER   ";
  int len = strlen(text);
  int pos = (g_frame / 2) % (240 / 6 + len);
  
  M5.Display.setTextColor(TFT_YELLOW, TFT_BLACK);
  M5.Display.setCursor(pos * 6 - len * 6, 60);
  for (int i = 0; i < len; ++i) {
    char c = text[i];
    uint16_t fade = (i * 255 / len) * pos / (240 / 6);
    M5.Display.setTextColor(
      M5.Display.color565(
        ((int)c % 3) * 80 + fade / 4,
        ((int)c % 5) * 50,
        ((int)c % 7) * 30 + fade / 4
      ),
      TFT_BLACK
    );
    M5.Display.print(c);
  }
}

void drawAsciiWave() {
  M5.Display.fillScreen(TFT_BLACK);
  M5.Display.setTextSize(1);
  const char wave[] = "~-~-~-~-~-~-~-~-~-~-";
  for (int row = 0; row < 8; ++row) {
    int phase = (g_frame + row * 4) % 240;
    int offset = sin(phase * 3.14159 / 120.0) * 40;
    M5.Display.setTextColor(
      M5.Display.color565(
        200 - row * 20,
        100 + row * 10,
        150
      ),
      TFT_BLACK
    );
    M5.Display.setCursor(20 + offset, 20 + row * 12);
    M5.Display.print(wave);
  }
}

void drawAsciiBlocks() {
  M5.Display.fillScreen(TFT_BLACK);
  M5.Display.setTextSize(1);
  // Block characters: ▓ ▒ ░ █ ■
  char blocks[] = {0xDB, 0xB0, 0xB1, 0xB2};  // Block density chars
  for (int y = 0; y < 8; ++y) {
    for (int x = 0; x < 30; ++x) {
      int idx = ((g_frame + x + y) % 4);
      int py = 20 + y * 12;
      int px = 8 + x * 8;
      uint16_t color = M5.Display.color565(
        ((x + g_frame) * 8) % 256,
        ((y + g_frame) * 8) % 256,
        ((x ^ y ^ g_frame) * 12) % 256
      );
      M5.Display.setTextColor(color, TFT_BLACK);
      M5.Display.setCursor(px, py);
      M5.Display.print(char('*' + (idx % 4)));
    }
  }
}

void drawCurrent() {
  switch (g_saver) {
    case BashAlpha: drawAlpha(); break;
    case BashBouncing: drawBouncing(true); break;
    case BashCuteSaver: drawCuteSaver(); break;
    case BashFireworks: drawFireworks(); break;
    case BashLife: drawLife(TFT_GREEN); break;
    case BashMatrix: drawMatrixLike(); break;
    case BashPipes: drawPipes2D(); break;
    case BashRain: drawRain(); break;
    case BashSpeaky: drawSpeaky(); break;
    case BashStars: drawStars(); break;
    case BashTunnel: drawTunnel(true); break;
    case BashVibe: drawVibe(); break;
    case TTAnt: drawAnt(); break;
    case TTBalls: drawBouncing(true); break;
    case TTBubble: drawBubble(); break;
    case TTCube: drawCube(); break;
    case TTLife: drawLife(TFT_CYAN); break;
    case TTPipes3D: drawPipes3D(); break;
    case TTRings: drawRings(); break;
    case TTSand: drawSand(); break;
    case TTSplits: drawSplits(); break;
    case TTTunnel: drawTunnel(false); break;
    case AsciiRain: drawAsciiRain(); break;
    case AsciiCode: drawAsciiCode(); break;
    case AsciiScroll: drawAsciiScroll(); break;
    case AsciiWave: drawAsciiWave(); break;
    case AsciiBlocks: drawAsciiBlocks(); break;
    default: break;
  }
  clearTopBar();
}

}  // namespace

void setup() {
  auto cfg = M5.config();
  M5.begin(cfg);
#if defined(STICKS3)
  M5.Display.setRotation(1);
#else
  M5.Display.setRotation(3);
#endif
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
