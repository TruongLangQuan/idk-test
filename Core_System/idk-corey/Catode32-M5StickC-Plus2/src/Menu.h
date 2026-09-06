#pragma once
// Menu.h - Composable menu component adapted for 3-button navigation
//
// Navigation:
//   BtnA  short  = next item (down)
//   BtnPWR short = prev item (up)
//   BtnA  medium = select / enter submenu
//   BtnB  long   = back / close

#include "Renderer.h"
#include "Input.h"
#include "config.h"
#include <Arduino.h>

// ── MenuItem ──────────────────────────────────────────────────────────────

static const int MENU_MAX_ITEMS    = 16;
static const int MENU_MAX_DEPTH    = 4;
static const int MENU_VISIBLE_ROWS = 5;
static const int MENU_ROW_H        = 20;
static const int MENU_ICON_SIZE    = 13;

struct MenuItem {
    const char*  label       = nullptr;
    const Sprite* icon       = nullptr;
    int          actionType  = 0;   // 0=none, 1=scene, 2=settings, 3=custom
    const char*  actionParam = nullptr;
    // Submenu
    MenuItem*    submenu     = nullptr;
    int          submenuCount= 0;
    // Confirmation text
    const char*  confirm     = nullptr;

    // Custom action callback
    using ActionCb = void(*)(void* userData);
    ActionCb     cb          = nullptr;
    void*        cbData      = nullptr;
};

// Action type constants
static const int ACT_NONE     = 0;
static const int ACT_SCENE    = 1;
static const int ACT_SETTINGS = 2;
static const int ACT_CUSTOM   = 3;

// ── Menu stack frame ──────────────────────────────────────────────────────

struct MenuFrame {
    MenuItem* items;
    int       count;
    int       selected;
    int       scrollOffset;
};

// ── Menu result ───────────────────────────────────────────────────────────

struct MenuResult {
    bool        closed      = false;  // menu was dismissed
    bool        selected    = false;  // item was chosen
    const MenuItem* item    = nullptr;
};

// ── Menu ──────────────────────────────────────────────────────────────────

class Menu {
public:
    bool active = false;

    Menu(Renderer* r, InputHandler* inp) : _r(r), _inp(inp), _depth(0) {}

    void open(MenuItem* items, int count) {
        _stack[0] = { items, count, 0, 0 };
        _depth  = 1;
        active  = true;
        _pendingConfirm = nullptr;
    }

    void close() {
        active = false;
        _depth = 0;
        _pendingConfirm = nullptr;
    }

    MenuResult handleInput() {
        if (!active) return {};

        // Close on BtnB long
        if (_inp->back()) {
            if (_depth > 1) {
                // Go up one level
                _depth--;
                return {};
            }
            close();
            return { .closed=true };
        }

        // Confirmation pending
        if (_pendingConfirm) {
            if (_inp->next()) {
                _confirmYes = !_confirmYes;
            }
            if (_inp->select()) {
                MenuItem* item = _pendingConfirm;
                _pendingConfirm = nullptr;
                if (_confirmYes) {
                    close();
                    return { .selected=true, .item=item };
                }
                return {};
            }
            return {};
        }

        MenuFrame& frame = _stack[_depth - 1];

        if (_inp->next()) {
            frame.selected = (frame.selected + 1) % frame.count;
            _adjustScroll(frame);
        }
        if (_inp->prev()) {
            frame.selected = (frame.selected - 1 + frame.count) % frame.count;
            _adjustScroll(frame);
        }

        if (_inp->select()) {
            MenuItem* item = &frame.items[frame.selected];

            if (item->confirm) {
                _pendingConfirm = item;
                _confirmYes     = false;
                return {};
            }

            if (item->submenu && item->submenuCount > 0) {
                // Enter submenu
                if (_depth < MENU_MAX_DEPTH) {
                    _stack[_depth++] = { item->submenu, item->submenuCount, 0, 0 };
                }
                return {};
            }

            close();
            return { .selected=true, .item=item };
        }

        return {};
    }

    void draw() {
        if (!active) return;

        MenuFrame& frame = _stack[_depth - 1];

        // Background panel
        int panelX = 0, panelY = PLAY_Y;
        int panelW = DISPLAY_WIDTH, panelH = DISPLAY_HEIGHT - PLAY_Y;
        _r->drawRect(panelX, panelY, panelW, panelH, COLOR_UI_BG, true);
        _r->drawRect(panelX, panelY, panelW, panelH, COLOR_UI_BORDER, false);

        // Title (depth breadcrumb)
        _r->drawText(_depth > 1 ? "< Back" : "Menu",
                     6, panelY + 4, COLOR_UI_DIM, COLOR_UI_BG, 1);
        _r->drawLine(0, panelY + 16, DISPLAY_WIDTH, panelY + 16, COLOR_UI_BORDER);

        // Confirmation dialog overlay
        if (_pendingConfirm) {
            _drawConfirm();
            return;
        }

        // Items
        int startY = panelY + 20;
        for (int vi = 0; vi < MENU_VISIBLE_ROWS && vi < frame.count; vi++) {
            int idx = frame.scrollOffset + vi;
            if (idx >= frame.count) break;

            MenuItem& item = frame.items[idx];
            bool sel = (idx == frame.selected);
            int itemY = startY + vi * MENU_ROW_H;

            // Highlight
            if (sel)
                _r->drawRect(2, itemY, DISPLAY_WIDTH - 8, MENU_ROW_H - 2,
                             COLOR_UI_SELECT, true);

            // Icon
            if (item.icon) {
                _r->drawSpriteObj(item.icon, 4, itemY + 3, 0, false,
                                  sel ? COLOR_BLACK : COLOR_UI_TEXT,
                                  sel ? COLOR_BLACK : COLOR_CREAM, 1);
            }

            // Label
            int textX = item.icon ? 4 + MENU_ICON_SIZE * 1 + 4 : 8;
            _r->drawText(item.label, textX, itemY + 6,
                         sel ? COLOR_BLACK : COLOR_UI_TEXT, 0x0000, 1);

            // Submenu arrow
            if (item.submenu && item.submenuCount > 0) {
                _r->drawText(">", DISPLAY_WIDTH - 14, itemY + 6,
                             sel ? COLOR_BLACK : COLOR_UI_DIM, 0x0000, 1);
            }
        }

        // Scrollbar
        if (frame.count > MENU_VISIBLE_ROWS) {
            _r->drawScrollbar(DISPLAY_WIDTH - 5, startY,
                              MENU_VISIBLE_ROWS * MENU_ROW_H,
                              frame.count, MENU_VISIBLE_ROWS, frame.scrollOffset);
        }

    }

private:
    Renderer*     _r;
    InputHandler* _inp;
    MenuFrame     _stack[MENU_MAX_DEPTH];
    int           _depth;
    MenuItem*     _pendingConfirm = nullptr;
    bool          _confirmYes     = false;

    void _adjustScroll(MenuFrame& f) {
        if (f.selected < f.scrollOffset)
            f.scrollOffset = f.selected;
        else if (f.selected >= f.scrollOffset + MENU_VISIBLE_ROWS)
            f.scrollOffset = f.selected - MENU_VISIBLE_ROWS + 1;
    }

    void _drawConfirm() {
        int cx = 20, cy = PLAY_Y + 30;
        int cw = DISPLAY_WIDTH - 40, ch = 60;
        _r->drawRect(cx, cy, cw, ch, COLOR_UI_BG,     true);
        _r->drawRect(cx, cy, cw, ch, COLOR_UI_BORDER, false);
        _r->drawText(_pendingConfirm->confirm, cx+6, cy+8,
                     COLOR_UI_TEXT, COLOR_UI_BG, 1);
        _r->drawText(_confirmYes ? "> Yes    No" : "  Yes  > No",
                     cx+6, cy+30, COLOR_UI_TEXT, COLOR_UI_BG, 1);
        _r->drawText("A:select  PWR:toggle",
                     cx+4, cy+46, COLOR_UI_DIM, COLOR_UI_BG, 1);
    }
};
