#include <M5Unified.h>
#include <math.h>

namespace {

// ─── ASCII Modes ───────────────────────────────────────────────
enum AsciiMode {
  DETAILED,
  STANDARD,
  MINIMAL,
  BLOCK,
  DOTS,
  MODE_COUNT,
};

// ─── Animation Types ───────────────────────────────────────────
enum AnimType {
  ANIM_CUBE,
  ANIM_TORUS,
  ANIM_HELIX,
  ANIM_SPHERE,
  ANIM_WAVE3D,
  ANIM_TUNNEL,
  ANIM_FRACTAL_TREE,
  ANIM_LISSAJOUS,
  ANIM_COUNT,
};

const char* kModeNames[] = {"detailed", "standard", "minimal", "block", "dots"};
const char* kAnimNames[] = {"cube", "torus", "helix", "sphere", "wave3d", "tunnel", "tree", "lissa"};

const char* kStandardChars = "@#%*+-=~`!.|/\\O";
const char* kMinimalChars = "#.-|+/\\O";
const char* kBlockChars = "#*+.";
const char* kDotsChars = ".:;'^v<>";
const char* kDetailedChars = "@#%&*Oo:. ";

// ─── 5-way tactile switch GPIO mapping ──────────────────────────
#if defined(STICKS3)
static constexpr int kPinUp     = 1;
static constexpr int kPinDown   = 2;
static constexpr int kPinLeft   = 3;
static constexpr int kPinRight  = 8;
static constexpr int kPinCenter = 43;
#elif defined(PCBFUN)
static constexpr int kPinUp     = 1;
static constexpr int kPinDown   = 2;
static constexpr int kPinLeft   = 3;
static constexpr int kPinRight  = 4;
static constexpr int kPinCenter = 5;
#else
static constexpr int kPinUp     = 32;
static constexpr int kPinDown   = 33;
static constexpr int kPinLeft   = 25;
static constexpr int kPinRight  = 26;
static constexpr int kPinCenter = 0;
#endif

struct Button {
  int pin;
  bool pressed;
  uint32_t lastRepeatMs;
};

static Button g_buttons[] = {
    {kPinUp, false, 0},
    {kPinDown, false, 0},
    {kPinLeft, false, 0},
    {kPinRight, false, 0},
    {kPinCenter, false, 0},
};

static constexpr uint32_t kRepeatMs = 150;

// ─── State ─────────────────────────────────────────────────────
static AsciiMode g_mode = STANDARD;
static AnimType g_anim = ANIM_CUBE;
static uint32_t g_frame = 0;
static int g_speed = 2;
static bool g_paused = false;
static uint32_t g_last_update = 0;

// ─── Depth buffer for 3D projection ────────────────────────────
static constexpr int kW = 30;
static constexpr int kH = 10;
static char g_screen[kH][kW + 1];
static float g_zbuf[kH][kW];

// ─── Helpers ───────────────────────────────────────────────────

bool readPressed(int pin) {
  return digitalRead(pin) == LOW;
}

char getCharForDepth(float depth, AsciiMode mode) {
  const char* charset;
  int charCount;
  switch (mode) {
    case DETAILED: charset = kDetailedChars; break;
    case STANDARD: charset = kStandardChars; break;
    case MINIMAL: charset = kMinimalChars; break;
    case BLOCK: charset = kBlockChars; break;
    case DOTS: charset = kDotsChars; break;
    default: charset = kStandardChars;
  }
  charCount = strlen(charset);
  float v = std::max(0.0f, std::min(1.0f, depth));
  int idx = (int)(v * (charCount - 1));
  return charset[idx];
}

void clearScreen() {
  for (int y = 0; y < kH; ++y) {
    for (int x = 0; x < kW; ++x) {
      g_screen[y][x] = ' ';
      g_zbuf[y][x] = -1000.0f;
    }
    g_screen[y][kW] = '\0';
  }
}

void plotPoint(float sx, float sy, float z, float brightness) {
  int ix = (int)((sx + 1.5f) / 3.0f * kW);
  int iy = (int)((sy + 1.0f) / 2.0f * kH);
  if (ix < 0 || ix >= kW || iy < 0 || iy >= kH) return;
  if (z > g_zbuf[iy][ix]) {
    g_zbuf[iy][ix] = z;
    g_screen[iy][ix] = getCharForDepth(brightness, g_mode);
  }
}

void renderToDisplay() {
  M5.Display.setTextSize(1);
  for (int y = 0; y < kH; ++y) {
    M5.Display.setCursor(0, 16 + y * 11);
    for (int x = 0; x < kW; ++x) {
      char c = g_screen[y][x];
      if (c == ' ') {
        M5.Display.setTextColor(TFT_BLACK, TFT_BLACK);
      } else {
        float depth = g_zbuf[y][x];
        int r = std::min(255, (int)(depth * 80 + 100));
        int g = std::min(255, (int)(depth * 100 + 60));
        int b = std::min(255, (int)(depth * 40 + 80));
        M5.Display.setTextColor(M5.Display.color565(r, g, b), TFT_BLACK);
      }
      M5.Display.print(c);
    }
  }
}

// ─── 3D Rotation helpers ───────────────────────────────────────
struct Vec3 { float x, y, z; };

Vec3 rotateX(Vec3 p, float a) {
  return {p.x, p.y * cosf(a) - p.z * sinf(a), p.y * sinf(a) + p.z * cosf(a)};
}
Vec3 rotateY(Vec3 p, float a) {
  return {p.x * cosf(a) + p.z * sinf(a), p.y, -p.x * sinf(a) + p.z * cosf(a)};
}
Vec3 rotateZ(Vec3 p, float a) {
  return {p.x * cosf(a) - p.y * sinf(a), p.x * sinf(a) + p.y * cosf(a), p.z};
}

Vec3 project(Vec3 p) {
  float fov = 3.0f;
  float z = p.z + fov;
  if (z < 0.1f) z = 0.1f;
  return {p.x / z * fov, p.y / z * fov, p.z};
}

// ─── Animations ────────────────────────────────────────────────

void drawCube3D() {
  clearScreen();
  float t = g_frame * 0.06f;

  // 12 edges of a cube
  float verts[8][3] = {
      {-1,-1,-1},{1,-1,-1},{1,1,-1},{-1,1,-1},
      {-1,-1,1},{1,-1,1},{1,1,1},{-1,1,1}
  };
  int edges[12][2] = {
      {0,1},{1,2},{2,3},{3,0},{4,5},{5,6},{6,7},{7,4},{0,4},{1,5},{2,6},{3,7}
  };

  for (auto& e : edges) {
    Vec3 a = {verts[e[0]][0], verts[e[0]][1], verts[e[0]][2]};
    Vec3 b = {verts[e[1]][0], verts[e[1]][1], verts[e[1]][2]};
    a = rotateX(rotateY(a, t), t * 0.7f);
    b = rotateX(rotateY(b, t), t * 0.7f);
    Vec3 pa = project(a);
    Vec3 pb = project(b);

    // Draw line with interpolation
    int steps = 20;
    for (int s = 0; s <= steps; ++s) {
      float frac = s / (float)steps;
      float sx = pa.x + (pb.x - pa.x) * frac;
      float sy = pa.y + (pb.y - pa.y) * frac;
      float sz = pa.z + (pb.z - pa.z) * frac;
      float bright = (sz + 2.0f) / 4.0f;
      plotPoint(sx, sy, sz, bright);
    }
  }
  renderToDisplay();
}

void drawTorus3D() {
  clearScreen();
  float t = g_frame * 0.04f;

  for (float theta = 0; theta < 6.28f; theta += 0.2f) {
    for (float phi = 0; phi < 6.28f; phi += 0.3f) {
      float R = 1.0f, r = 0.4f;
      Vec3 p = {
          (R + r * cosf(theta)) * cosf(phi),
          (R + r * cosf(theta)) * sinf(phi),
          r * sinf(theta)
      };
      p = rotateX(rotateY(p, t), t * 0.5f);
      Vec3 pp = project(p);
      float bright = (p.z + 2.0f) / 4.0f;
      plotPoint(pp.x, pp.y, p.z, bright);
    }
  }
  renderToDisplay();
}

void drawHelix3D() {
  clearScreen();
  float t = g_frame * 0.05f;

  for (int strand = 0; strand < 2; ++strand) {
    float offset = strand * 3.14159f;
    for (float i = -3; i < 3; i += 0.1f) {
      Vec3 p = {cosf(i * 2 + offset + t) * 0.5f, i * 0.3f, sinf(i * 2 + offset + t) * 0.5f};
      p = rotateY(p, t * 0.3f);
      Vec3 pp = project(p);
      float bright = (p.z + 2.0f) / 4.0f;
      plotPoint(pp.x, pp.y, p.z, bright);
    }
  }

  // Connections
  for (float i = -3; i < 3; i += 0.6f) {
    Vec3 a = {cosf(i * 2 + t) * 0.5f, i * 0.3f, sinf(i * 2 + t) * 0.5f};
    Vec3 b = {cosf(i * 2 + 3.14159f + t) * 0.5f, i * 0.3f, sinf(i * 2 + 3.14159f + t) * 0.5f};
    a = rotateY(a, t * 0.3f);
    b = rotateY(b, t * 0.3f);
    for (int s = 0; s <= 8; ++s) {
      float f = s / 8.0f;
      Vec3 m = {a.x + (b.x - a.x) * f, a.y + (b.y - a.y) * f, a.z + (b.z - a.z) * f};
      Vec3 pm = project(m);
      plotPoint(pm.x, pm.y, m.z, 0.3f);
    }
  }
  renderToDisplay();
}

void drawSphere3D() {
  clearScreen();
  float t = g_frame * 0.03f;

  for (float lat = -1.5f; lat < 1.5f; lat += 0.15f) {
    for (float lon = 0; lon < 6.28f; lon += 0.2f) {
      float r = cosf(lat);
      Vec3 p = {r * cosf(lon), sinf(lat), r * sinf(lon)};
      p = rotateX(rotateY(p, t), t * 0.6f);
      Vec3 pp = project(p);
      float bright = (p.z + 1.5f) / 3.0f;
      plotPoint(pp.x, pp.y, p.z, bright);
    }
  }
  renderToDisplay();
}

void drawWave3D() {
  clearScreen();
  float t = g_frame * 0.08f;

  for (float x = -1.5f; x < 1.5f; x += 0.12f) {
    for (float z = -1.5f; z < 1.5f; z += 0.12f) {
      float d = sqrtf(x * x + z * z);
      float y = sinf(d * 3 - t) * 0.3f * (1.0f / (d + 1));
      Vec3 p = {x, y, z};
      p = rotateX(rotateY(p, t * 0.2f), 0.5f);
      Vec3 pp = project(p);
      float bright = (y + 0.5f);
      plotPoint(pp.x, pp.y, p.z, bright);
    }
  }
  renderToDisplay();
}

void drawTunnel3D() {
  clearScreen();
  float t = g_frame * 0.1f;

  for (float depth = 0.5f; depth < 5.0f; depth += 0.3f) {
    for (float angle = 0; angle < 6.28f; angle += 0.3f) {
      float r = 0.8f;
      float wobble = sinf(depth * 2 + t) * 0.1f;
      Vec3 p = {
          (r + wobble) * cosf(angle + depth * 0.3f),
          (r + wobble) * sinf(angle + depth * 0.3f),
          depth - 2.5f
      };
      Vec3 pp = project(p);
      float bright = 1.0f - depth / 5.0f;
      plotPoint(pp.x, pp.y, p.z, bright);
    }
  }
  renderToDisplay();
}

void drawFractalTree3D() {
  clearScreen();
  float t = g_frame * 0.03f;

  // Recursive tree approximation with iteration
  struct Branch { Vec3 start; Vec3 dir; float len; int depth; };
  Branch stack[64];
  int stackSize = 0;
  stack[stackSize++] = {{0, -1, 0}, {0, 1, 0}, 0.5f, 0};

  while (stackSize > 0 && stackSize < 60) {
    Branch b = stack[--stackSize];
    if (b.depth > 4 || b.len < 0.05f) continue;

    Vec3 end = {b.start.x + b.dir.x * b.len, b.start.y + b.dir.y * b.len, b.start.z + b.dir.z * b.len};

    // Draw branch
    for (int s = 0; s <= 6; ++s) {
      float f = s / 6.0f;
      Vec3 p = {b.start.x + (end.x - b.start.x) * f,
                b.start.y + (end.y - b.start.y) * f,
                b.start.z + (end.z - b.start.z) * f};
      p = rotateY(p, t);
      Vec3 pp = project(p);
      float bright = 0.3f + (1.0f - b.depth / 5.0f) * 0.7f;
      plotPoint(pp.x, pp.y, p.z, bright);
    }

    float angle1 = 0.5f + sinf(t + b.depth) * 0.2f;
    float angle2 = -0.5f + cosf(t + b.depth) * 0.2f;
    Vec3 d1 = rotateZ(b.dir, angle1);
    Vec3 d2 = rotateZ(b.dir, angle2);
    if (stackSize < 62) {
      stack[stackSize++] = {end, d1, b.len * 0.7f, b.depth + 1};
      stack[stackSize++] = {end, d2, b.len * 0.7f, b.depth + 1};
    }
  }
  renderToDisplay();
}

void drawLissajous3D() {
  clearScreen();
  float t = g_frame * 0.04f;

  for (float i = 0; i < 6.28f; i += 0.03f) {
    Vec3 p = {
        sinf(3 * i + t),
        sinf(4 * i + t * 0.7f),
        cosf(5 * i + t * 0.5f)
    };
    p = rotateY(p, t * 0.2f);
    Vec3 pp = project(p);
    float bright = (p.z + 1.5f) / 3.0f;
    plotPoint(pp.x, pp.y, p.z, bright);
  }
  renderToDisplay();
}

// ─── Main draw ─────────────────────────────────────────────────
void drawScene() {
  M5.Display.fillScreen(TFT_BLACK);

  switch (g_anim) {
    case ANIM_CUBE:    drawCube3D(); break;
    case ANIM_TORUS:   drawTorus3D(); break;
    case ANIM_HELIX:   drawHelix3D(); break;
    case ANIM_SPHERE:  drawSphere3D(); break;
    case ANIM_WAVE3D:  drawWave3D(); break;
    case ANIM_TUNNEL:  drawTunnel3D(); break;
    case ANIM_FRACTAL_TREE: drawFractalTree3D(); break;
    case ANIM_LISSAJOUS: drawLissajous3D(); break;
    default: break;
  }

  M5.Display.setTextColor(TFT_YELLOW, TFT_BLACK);
  M5.Display.setCursor(4, 4);
  M5.Display.printf("%s|%s [%d/5] %s", kAnimNames[g_anim], kModeNames[g_mode],
                    g_speed, g_paused ? "PAUSED" : "");
}

void handleInput() {
  const uint32_t now = millis();

  auto handleDir = [&](Button& btn, int action) {
    const bool down = readPressed(btn.pin);
    if (!down) { btn.pressed = false; return; }

    bool trigger = false;
    if (!btn.pressed) {
      btn.pressed = true;
      btn.lastRepeatMs = now;
      trigger = true;
    } else if (now - btn.lastRepeatMs > kRepeatMs) {
      btn.lastRepeatMs = now;
      trigger = true;
    }

    if (trigger) {
      if (action == 0) { if (g_speed < 5) g_speed++; }       // UP - speed up
      else if (action == 1) { if (g_speed > 1) g_speed--; }  // DOWN - speed down
      else if (action == 2) {                                  // LEFT - prev anim
        g_anim = (AnimType)((g_anim - 1 + ANIM_COUNT) % ANIM_COUNT);
      }
      else if (action == 3) {                                  // RIGHT - next anim
        g_anim = (AnimType)((g_anim + 1) % ANIM_COUNT);
      }
    }
  };

  handleDir(g_buttons[0], 0);
  handleDir(g_buttons[1], 1);
  handleDir(g_buttons[2], 2);
  handleDir(g_buttons[3], 3);

  // CENTER - toggle mode
  if (readPressed(kPinCenter)) {
    static uint32_t lastCenter = 0;
    if (now - lastCenter > 300) {
      g_mode = (AsciiMode)((g_mode + 1) % MODE_COUNT);
      lastCenter = now;
    }
  }

  // A - pause
  if (M5.BtnA.wasPressed()) g_paused = !g_paused;

  // PWR - reset
  if (M5.BtnPWR.wasPressed()) {
    g_frame = 0;
    g_speed = 2;
    g_mode = STANDARD;
    g_anim = ANIM_CUBE;
    g_paused = false;
  }
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

  for (auto& btn : g_buttons) {
    pinMode(btn.pin, INPUT_PULLUP);
  }

  g_last_update = millis();
}

void loop() {
  M5.update();
  handleInput();

  uint32_t now = millis();
  if (!g_paused && (now - g_last_update) > (uint32_t)(100 / g_speed)) {
    g_frame++;
    g_last_update = now;
  }

  drawScene();
  delay(5);
}
