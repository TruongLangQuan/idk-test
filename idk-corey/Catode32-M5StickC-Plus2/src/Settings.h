#pragma once
// Settings.h - Environment settings overlay
// Lets the player adjust time-of-day, season, weather, moon phase.

#include "Renderer.h"
#include "Input.h"
#include "GameContext.h"

struct SettingsEntry {
    const char* label;
    const char** options;
    int          optionCount;
    int*         value;   // index into options[]
};

class Settings {
public:
    Settings(Renderer* r, InputHandler* inp, GameContext* ctx)
        : _renderer(r), _input(inp), _ctx(ctx),
          _open(false), _cursor(0), _editMode(false)
    {
        _buildEntries();
    }

    bool isOpen() const { return _open; }

    void open() {
        _open     = true;
        _cursor   = 0;
        _editMode = false;
    }

    void close() { _open = false; }

    void draw() {
        if (!_open) return;

        // Semi-transparent panel overlay (filled rect with border)
        int px=10, py=PLAY_Y+5, pw=DISPLAY_WIDTH-20, ph=PLAY_HEIGHT-10;
        _renderer->drawRect(px, py, pw, ph, COLOR_UI_BG, true);
        _renderer->drawRect(px, py, pw, ph, COLOR_UI_BORDER, false);

        _renderer->drawText("Environment", px+4, py+3, COLOR_UI_SELECT, COLOR_UI_BG, 1);
        _renderer->drawLine(px+2, py+13, px+pw-2, py+13, COLOR_UI_BORDER);

        int rowH = 14, startY = py + 16;
        for (int i=0; i<_entryCount; i++) {
            SettingsEntry& e = _entries[i];
            int rowY = startY + i*rowH;
            bool sel = (i == _cursor);
            bool editing = sel && _editMode;

            uint16_t labelColor = sel ? COLOR_UI_SELECT : COLOR_UI_TEXT;
            uint16_t valColor   = editing ? COLOR_YELLOW : COLOR_WHITE;
            uint16_t bgColor    = sel ? 0x1082 : COLOR_UI_BG; // dark highlight

            _renderer->drawRect(px+2, rowY, pw-4, rowH-1, bgColor, true);
            _renderer->drawText(e.label, px+4, rowY+2, labelColor, bgColor, 1);

            if (e.value && *e.value >= 0 && *e.value < e.optionCount) {
                const char* val = e.options[*e.value];
                int vx = px + pw - (int)strlen(val)*6 - 6;
                _renderer->drawText(val, vx, rowY+2, valColor, bgColor, 1);
                if (editing) {
                    _renderer->drawText("<", vx-7, rowY+2, COLOR_UI_SELECT, bgColor, 1);
                    int rw = (int)strlen(val)*6 + 6;
                    _renderer->drawText(">", vx+rw, rowY+2, COLOR_UI_SELECT, bgColor, 1);
                }
            }
        }

        // Instructions
        _renderer->drawText("A:dn PWR:up A-med:edit B:close",
                            px+4, py+ph-10, COLOR_UI_DIM, COLOR_UI_BG, 1);
    }

    // Returns true while settings is still open, false when closed
    bool handleInput() {
        if (!_open) return false;

        if (_input->back()) { close(); return false; }

        if (!_editMode) {
            if (_input->next()) _cursor = (_cursor+1) % _entryCount;
            if (_input->prev()) _cursor = (_cursor-1+_entryCount) % _entryCount;
            if (_input->select()) { _editMode = true; }
        } else {
            SettingsEntry& e = _entries[_cursor];
            if (_input->next()) {
                if (e.value) *e.value = (*e.value + 1) % e.optionCount;
                _applyChanges();
            }
            if (_input->prev()) {
                if (e.value) *e.value = (*e.value - 1 + e.optionCount) % e.optionCount;
                _applyChanges();
            }
            if (_input->select() || _input->back()) { _editMode = false; }
        }
        return true;
    }

private:
    Renderer*     _renderer;
    InputHandler* _input;
    GameContext*  _ctx;
    bool          _open;
    int           _cursor;
    bool          _editMode;

    static const int MAX_ENTRIES = 8;
    SettingsEntry _entries[MAX_ENTRIES];
    int           _entryCount = 0;

    static const char* HOUR_OPTS[];
    static const char* SEASON_OPTS[];
    static const char* WEATHER_OPTS[];
    static const char* MOON_OPTS[];

    void _buildEntries() {
        _entryCount = 0;
        _entries[_entryCount++] = { "Time of day", HOUR_OPTS,    24, &_ctx->environment.time_of_day };
        _entries[_entryCount++] = { "Season",      SEASON_OPTS,   4, &_ctx->environment.season      };
        _entries[_entryCount++] = { "Weather",     WEATHER_OPTS,  5, &_ctx->environment.weather     };
        _entries[_entryCount++] = { "Moon phase",  MOON_OPTS,     8, &_ctx->environment.moon_phase  };
    }

    void _applyChanges() {
        _ctx->saveEnv();
    }
};

// Option string tables
inline const char* Settings::HOUR_OPTS[] = {
    "12am","1am","2am","3am","4am","5am",
    "6am","7am","8am","9am","10am","11am",
    "12pm","1pm","2pm","3pm","4pm","5pm",
    "6pm","7pm","8pm","9pm","10pm","11pm"
};
inline const char* Settings::SEASON_OPTS[]  = { "Spring","Summer","Autumn","Winter" };
inline const char* Settings::WEATHER_OPTS[] = { "Clear","Cloudy","Rain","Thunderstorm","Snow" };
inline const char* Settings::MOON_OPTS[]    = { "New","WaxCres","FirstQ","WaxGib",
                                                 "Full","WanGib","LastQ","WanCres" };
