#pragma once
// StatsScene.h - Scrollable pet stats display

#include "Scene.h"
#include <string.h>

struct StatEntry {
    bool        isHeader;
    const char* name;
    const char* key;
    const char* desc;
};

static const StatEntry STATS_TABLE[] = {
    {true,  "Vitals",      nullptr, nullptr},
    {false, "Health",      "health",      "Overall physical wellbeing."},
    {false, "Fullness",    "fullness",    "How full your pet's belly is. Feed meals to maintain."},
    {false, "Energy",      "energy",      "Available energy. Rest and sleep to restore."},
    {true,  "",            nullptr, nullptr},
    {true,  "Physical",    nullptr, nullptr},
    {false, "Comfort",     "comfort",     "Physical ease with environment. Keep it clean and calm."},
    {false, "Cleanliness", "cleanliness", "How clean your pet is. Groom regularly."},
    {false, "Fitness",     "fitness",     "Athletic conditioning. Maintained through exercise and play."},
    {false, "Grace",       "grace",       "Physical elegance. Develops through active movement."},
    {false, "Appetite",    "appetite",    "Interest in food variety. A varied diet broadens palate."},
    {true,  "",            nullptr, nullptr},
    {true,  "Mental",      nullptr, nullptr},
    {false, "Focus",       "focus",       "Concentration ability. Good rest and calm helps."},
    {false, "Intelligence","intelligence","Problem-solving. Puzzles and training develop this."},
    {false, "Curiosity",   "curiosity",   "Drive to explore. Expose your pet to new things."},
    {true,  "",            nullptr, nullptr},
    {true,  "Emotional",   nullptr, nullptr},
    {false, "Playfulness", "playfulness", "Desire to play. Use toys and games to satisfy."},
    {false, "Affection",   "affection",   "How loved your pet feels. Pet and spend time together."},
    {false, "Fulfillment", "fulfillment", "Life satisfaction. A happy varied routine helps."},
    {false, "Resilience",  "resilience",  "Ability to recover from stress. Stability builds this."},
    {false, "Serenity",    "serenity",    "Inner peace. A low-stress routine maintains this."},
    {false, "Patience",    "patience",    "Tolerance for waiting. Builds through consistent care."},
    {true,  "",            nullptr, nullptr},
    {true,  "Social",      nullptr, nullptr},
    {false, "Sociability", "sociability", "Eagerness to interact. Social play keeps this high."},
    {false, "Independence","independence","Comfort being alone. Confident pets are happily solo."},
    {false, "Charisma",    "charisma",    "Appeal to others. Health and confidence influence this."},
    {true,  "",            nullptr, nullptr},
    {true,  "Character",   nullptr, nullptr},
    {false, "Courage",     "courage",     "Boldness in new situations. Positive experiences build this."},
    {false, "Loyalty",     "loyalty",     "Strength of attachment. Grows through consistent care."},
    {false, "Mischief",    "mischievousness","Tendency toward trouble. High energy and boredom raise it."},
    {false, "Dignity",     "dignity",     "How they carry themselves. Bounces back from embarrassment."},
    {false, "Maturity",    "maturity",    "Behavioral sophistication. Grows naturally with experience."},
    {false, "Craftiness",  "craftiness",  "Cleverness in getting what they want."},
    {false, "Routine",     "routine",     "Comfort with familiar patterns. Consistency helps."},
};
static const int STATS_TABLE_SIZE = sizeof(STATS_TABLE) / sizeof(STATS_TABLE[0]);

class StatsScene : public Scene {
public:
    static const int ROW_H         = 18;
    static const int BAR_W         = 80;
    static const int BAR_H         = 6;
    static const int VISIBLE_ROWS  = 6;

    StatsScene(GameContext* ctx, Renderer* r, InputHandler* inp)
        : Scene(ctx,r,inp), _selected(0), _scrollOff(0),
          _showDetail(false), _detailTimer(0.0f)
    {}

    void enter() override { _selected=0; _scrollOff=0; _showDetail=false; }

    SceneResult update(float dt) override {
        if (_showDetail) {
            _detailTimer += dt;
            if (_detailTimer > 3.0f) _showDetail = false;
        }
        return NO_CHANGE;
    }

    void draw() override {
        _renderer->clear();
        _renderer->drawText("Pet Stats", 4, PLAY_Y + 2, COLOR_UI_TEXT, COLOR_BLACK, 1);
        _renderer->drawLine(0, PLAY_Y+12, DISPLAY_WIDTH, PLAY_Y+12, COLOR_UI_BORDER);

        int startY = PLAY_Y + 14;
        int visStat = 0; // visible stat entries counter

        for (int i = _scrollOff; i < STATS_TABLE_SIZE && visStat < VISIBLE_ROWS; i++) {
            const StatEntry& e = STATS_TABLE[i];
            int rowY = startY + visStat * ROW_H;

            if (e.isHeader) {
                if (e.name[0] != '\0') {
                    _renderer->drawText(e.name, 4, rowY + 4,
                                        COLOR_UI_DIM, COLOR_BLACK, 1);
                    _renderer->drawLine(4 + strlen(e.name)*6+2, rowY+8,
                                        DISPLAY_WIDTH - 8, rowY+8, COLOR_UI_BORDER);
                }
            } else {
                bool sel = (i == _selected);

                if (sel)
                    _renderer->drawRect(0, rowY, DISPLAY_WIDTH-5, ROW_H-2,
                                        COLOR_UI_SELECT, true);

                // Name
                _renderer->drawText(e.name, 4, rowY+2,
                                    sel ? COLOR_BLACK : COLOR_UI_TEXT,
                                    sel ? COLOR_UI_SELECT : COLOR_BLACK, 1);

                // Bar
                float val = _context->getStat(e.key) / 100.0f;
                uint16_t barCol = (val > 0.6f) ? COLOR_BAR_HIGH
                                : (val > 0.3f) ? COLOR_BAR_MED
                                               : COLOR_BAR_LOW;
                int barX = DISPLAY_WIDTH - BAR_W - 10;
                _renderer->drawRect(barX, rowY+5, BAR_W, BAR_H,
                                    sel ? COLOR_BLACK : COLOR_UI_BORDER, false);
                int filled = (int)(val * (BAR_W - 2));
                if (filled > 0)
                    _renderer->drawRect(barX+1, rowY+6, filled, BAR_H-2, barCol, true);

                // Value text
                char vbuf[8];
                snprintf(vbuf, sizeof(vbuf), "%d", (int)_context->getStat(e.key));
                _renderer->drawText(vbuf, barX + BAR_W + 2, rowY+4,
                                    sel ? COLOR_BLACK : COLOR_UI_DIM,
                                    sel ? COLOR_UI_SELECT : COLOR_BLACK, 1);
            }
            visStat++;
        }

        // Scrollbar
        _renderer->drawScrollbar(DISPLAY_WIDTH-4, PLAY_Y+14,
            VISIBLE_ROWS*ROW_H, STATS_TABLE_SIZE, VISIBLE_ROWS, _scrollOff);

        // Detail popup for selected stat
        if (_showDetail && _selected < STATS_TABLE_SIZE && !STATS_TABLE[_selected].isHeader) {
            _drawDetail(STATS_TABLE[_selected]);
        }

        _renderer->drawText("A:dn PWR:up sel:desc B:back",
                            4, DISPLAY_HEIGHT-9, COLOR_UI_DIM, COLOR_BLACK, 1);
    }

    SceneResult handleInput() override {
        if (_showDetail) {
            if (_input->back() || _input->select()) { _showDetail=false; return NO_CHANGE; }
            return NO_CHANGE;
        }

        if (_input->back()) return changeTo("normal");

        if (_input->next()) {
            _selected = (_selected + 1) % STATS_TABLE_SIZE;
            _adjustScroll();
        }
        if (_input->prev()) {
            _selected = (_selected - 1 + STATS_TABLE_SIZE) % STATS_TABLE_SIZE;
            _adjustScroll();
        }
        if (_input->select()) {
            if (!STATS_TABLE[_selected].isHeader) {
                _showDetail  = true;
                _detailTimer = 0.0f;
            }
        }
        return NO_CHANGE;
    }

private:
    int   _selected, _scrollOff;
    bool  _showDetail;
    float _detailTimer;

    void _adjustScroll() {
        if (_selected < _scrollOff) _scrollOff = _selected;
        else if (_selected >= _scrollOff + VISIBLE_ROWS)
            _scrollOff = _selected - VISIBLE_ROWS + 1;
    }

    void _drawDetail(const StatEntry& e) {
        int px=10, py=PLAY_Y+20, pw=DISPLAY_WIDTH-20, ph=55;
        _renderer->drawRect(px, py, pw, ph, COLOR_UI_BG, true);
        _renderer->drawRect(px, py, pw, ph, COLOR_UI_BORDER, false);
        _renderer->drawText(e.name, px+4, py+4, COLOR_UI_SELECT, COLOR_UI_BG, 1);
        // Word-wrap description
        const char* desc = e.desc;
        int lineY = py + 16;
        char lineBuf[28]; int lb=0;
        for (int ci=0; desc[ci] && lineY < py+ph-10; ci++) {
            lineBuf[lb++] = desc[ci];
            if (lb >= 26 || desc[ci+1]=='\0') {
                lineBuf[lb]='\0';
                _renderer->drawText(lineBuf, px+4, lineY, COLOR_UI_TEXT, COLOR_UI_BG, 1);
                lineY += 10; lb = 0;
            }
        }
    }
};
