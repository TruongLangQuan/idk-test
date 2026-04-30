#pragma once
// Renderer.h - Display rendering for M5StickC Plus2 via M5Unified

#include <M5Unified.h>
#include "config.h"

// ============================================================================
// Sprite data structures
// ============================================================================

// Max frames per sprite part
static const int MAX_FRAMES     = 8;
static const int MAX_FILL_FRAMES = 8;

// Animated integer value: if count==1 use values[0], else use values[frame % count]
struct AnimInt {
    int  values[MAX_FRAMES];
    int  count;

    int get(int frame = 0) const {
        if (count <= 1) return values[0];
        return values[frame % count];
    }
};

inline AnimInt makeStatic(int v)  { return {{v,0,0,0,0,0,0,0}, 1}; }
inline AnimInt makeAnim2(int a, int b) { return {{a,b,0,0,0,0,0,0}, 2}; }
inline AnimInt makeAnim4(int a,int b,int c,int d) { return {{a,b,c,d,0,0,0,0}, 4}; }

struct Sprite {
    int            width;
    int            height;
    int            anchor_x;
    int            anchor_y;
    float          speed;        // animation speed (frames per second)
    int            extra_frames; // extra hold frames at end of cycle

    // Optional attachment points (static or animated)
    AnimInt        head_x, head_y;   // where to attach head (body sprite)
    AnimInt        tail_x, tail_y;   // where to attach tail (body sprite)
    AnimInt        eye_x,  eye_y;    // where to attach eyes (head sprite)

    bool           head_first;  // draw head before body

    // Frame data (1-bit MONO_HLSB, row-major, MSB first)
    int                  frame_count;
    const uint8_t* const frames[MAX_FRAMES];

    // Optional fill frame data (white fill drawn beneath outline in fill color)
    int                  fill_frame_count;
    const uint8_t* const fill_frames[MAX_FILL_FRAMES];
};

// ============================================================================
// Renderer
// ============================================================================

class Renderer {
public:
    Renderer() : _canvas(nullptr) {}

    void begin() {
        M5.Display.setRotation(3);
        M5.Display.setBrightness(100);
        _canvas = new M5Canvas(&M5.Display);
        _canvas->createSprite(DISPLAY_WIDTH, DISPLAY_HEIGHT);
        clear();
        show();
    }

    void clear(uint16_t color = COLOR_BLACK) {
        if (_canvas) _canvas->fillScreen(color);
        else         M5.Display.fillScreen(color);
    }

    void show() {
        if (_canvas) _canvas->pushSprite(0, 0);
    }

    // ── Primitives ───────────────────────────────────────────────────────
    void drawText(const char* text, int x, int y,
                  uint16_t fg = COLOR_WHITE, uint16_t bg = COLOR_BLACK,
                  int textSize = 1) {
        if (_canvas) {
            _canvas->setTextColor(fg, bg); _canvas->setTextSize(textSize);
            _canvas->setCursor(x, y); _canvas->print(text);
        } else {
            M5.Display.setTextColor(fg, bg); M5.Display.setTextSize(textSize);
            M5.Display.setCursor(x, y); M5.Display.print(text);
        }
    }

    void drawTextf(int x, int y, uint16_t fg, uint16_t bg, int sz,
                   const char* fmt, ...) {
        char buf[128];
        va_list ap; va_start(ap, fmt);
        vsnprintf(buf, sizeof(buf), fmt, ap);
        va_end(ap);
        drawText(buf, x, y, fg, bg, sz);
    }

    void drawRect(int x, int y, int w, int h,
                  uint16_t color = COLOR_WHITE, bool filled = false) {
        if (_canvas) { if (filled) _canvas->fillRect(x,y,w,h,color); else _canvas->drawRect(x,y,w,h,color); }
        else         { if (filled) M5.Display.fillRect(x,y,w,h,color); else M5.Display.drawRect(x,y,w,h,color); }
    }

    void drawLine(int x1, int y1, int x2, int y2, uint16_t color = COLOR_WHITE) {
        if (_canvas) _canvas->drawLine(x1,y1,x2,y2,color);
        else         M5.Display.drawLine(x1,y1,x2,y2,color);
    }

    void drawPixel(int x, int y, uint16_t color = COLOR_WHITE) {
        if (_canvas) _canvas->drawPixel(x,y,color);
        else         M5.Display.drawPixel(x,y,color);
    }

    void drawCircle(int x, int y, int r, uint16_t color, bool filled = false) {
        if (_canvas) { if (filled) _canvas->fillCircle(x,y,r,color); else _canvas->drawCircle(x,y,r,color); }
        else         { if (filled) M5.Display.fillCircle(x,y,r,color); else M5.Display.drawCircle(x,y,r,color); }
    }

    void drawTriangle(int x0,int y0,int x1,int y1,int x2,int y2,
                      uint16_t color, bool filled = false) {
        if (_canvas) { if (filled) _canvas->fillTriangle(x0,y0,x1,y1,x2,y2,color); else _canvas->drawTriangle(x0,y0,x1,y1,x2,y2,color); }
        else         { if (filled) M5.Display.fillTriangle(x0,y0,x1,y1,x2,y2,color); else M5.Display.drawTriangle(x0,y0,x1,y1,x2,y2,color); }
    }

    // ── 1-bit sprite blit ────────────────────────────────────────────────
    void drawBitmap1bit(const uint8_t* data, int srcW, int srcH,
                        int x, int y,
                        uint16_t fgColor    = COLOR_WHITE,
                        uint16_t bgColor    = COLOR_BLACK,
                        bool     transparent = true,
                        int      scale       = 1,
                        bool     mirror_h    = false) {
        int stride = (srcW + 7) / 8;
        for (int row = 0; row < srcH; row++) {
            for (int col = 0; col < srcW; col++) {
                int srcCol = mirror_h ? (srcW - 1 - col) : col;
                int byteIdx = row * stride + srcCol / 8;
                int bitIdx  = 7 - (srcCol % 8);
                bool set    = (data[byteIdx] >> bitIdx) & 1;
                if (!set && transparent) continue;
                uint16_t color = set ? fgColor : bgColor;
                int dx = x + col * scale;
                int dy = y + row * scale;
                if (scale == 1) drawPixel(dx, dy, color);
                else            drawRect(dx, dy, scale, scale, color, true);
            }
        }
    }

    // ── Sprite object draw ───────────────────────────────────────────────
    // Draw a Sprite struct at (x,y) using fill_frames (if any) then outline frames.
    // fill_frames are drawn in fillColor (default cream) with white=transparent.
    // Outline frames are drawn in fgColor (default white) with black=transparent.
    void drawSpriteObj(const Sprite* s, int x, int y,
                       int      frame      = 0,
                       bool     mirror_h   = false,
                       uint16_t fgColor    = COLOR_WHITE,
                       uint16_t fillColor  = COLOR_CREAM,
                       int      scale      = SPRITE_SCALE) {
        if (!s) return;
        int f = (s->frame_count > 0) ? (frame % s->frame_count) : 0;

        // Draw fill first (white pixels = shape fill, black = transparent)
        if (s->fill_frame_count > 0 && s->fill_frames[0] != nullptr) {
            int ff = frame % s->fill_frame_count;
            drawBitmap1bit(s->fill_frames[ff],
                           s->width, s->height,
                           x, y,
                           fillColor, COLOR_BLACK,
                           true, // black is transparent in fill layer
                           scale, mirror_h);
        }

        // Draw outline (black = transparent)
        if (s->frame_count > 0 && s->frames[0] != nullptr) {
            drawBitmap1bit(s->frames[f],
                           s->width, s->height,
                           x, y,
                           fgColor, COLOR_BLACK,
                           true, scale, mirror_h);
        }
    }

    // ── Status bar ───────────────────────────────────────────────────────
    void drawStatusBar(float fullness, float energy, float mood) {
        // Black background strip
        drawRect(0, 0, DISPLAY_WIDTH, STATUS_BAR_HEIGHT, COLOR_UI_BG, true);
        drawLine(0, STATUS_BAR_HEIGHT - 1, DISPLAY_WIDTH, STATUS_BAR_HEIGHT - 1, COLOR_UI_BORDER);

        // Three mini stat bars
        _drawMiniBar(4,  3, 60, 9, fullness, "Food");
        _drawMiniBar(90, 3, 60, 9, energy,   "Nrg");
        _drawMiniBar(176,3, 60, 9, mood,     "Mood");
    }

    // ── Scrollbar ────────────────────────────────────────────────────────
    void drawScrollbar(int x, int y, int height,
                       int totalItems, int visibleItems, int scrollOffset) {
        if (totalItems <= visibleItems) return;
        drawRect(x, y, 3, height, COLOR_UI_BORDER, false);
        float ratio   = (float)visibleItems / totalItems;
        float startR  = (float)scrollOffset / totalItems;
        int   barH    = max(4, (int)(height * ratio));
        int   barY    = y + (int)(height * startR);
        drawRect(x, barY, 3, barH, COLOR_UI_SELECT, true);
    }

    // ── FPS counter ──────────────────────────────────────────────────────
    void drawFps(float fps) {
        char buf[16];
        snprintf(buf, sizeof(buf), "%.0f", fps);
        drawText(buf, DISPLAY_WIDTH - 16, 0, COLOR_MED_GRAY, COLOR_BLACK, 1);
    }

private:
    M5Canvas* _canvas;

    void _drawMiniBar(int x, int y, int w, int h, float pct, const char* label) {
        uint16_t color = (pct > 0.6f) ? COLOR_BAR_HIGH
                       : (pct > 0.3f) ? COLOR_BAR_MED
                                       : COLOR_BAR_LOW;
        drawRect(x, y, w, h, COLOR_UI_BORDER, false);
        int fillW = (int)(w * max(0.0f, min(1.0f, pct)));
        if (fillW > 0) drawRect(x+1, y+1, fillW-2, h-2, color, true);
    }
};
