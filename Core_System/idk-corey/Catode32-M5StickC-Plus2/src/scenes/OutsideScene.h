#pragma once
// OutsideScene.h - Outdoor parallax scene with sky and butterflies

#include "Scene.h"
#include "Environment.h"
#include "Menu.h"
#include "entities/CharacterEntity.h"
#include "entities/ButterflyEntity.h"
#include "entities/behaviors/AffectionBehavior.h"
#include "entities/behaviors/AttentionBehavior.h"
#include "entities/behaviors/SnackingBehavior.h"
#include "entities/behaviors/PlayingBehavior.h"
#include "entities/behaviors/BeingGroomedBehavior.h"
#include "assets/nature_assets.h"
#include "assets/icons_assets.h"

static const int OUTSIDE_MENU_MAX = 12;
static const int OUTSIDE_TOY_MAX  = 6;

class OutsideScene : public Scene {
public:
    OutsideScene(GameContext* ctx, Renderer* r, InputHandler* inp)
        : Scene(ctx,r,inp), _menu(r,inp),
          _character(nullptr), _env(nullptr),
          _butterfly1(nullptr), _butterfly2(nullptr),
          _menuActive(false), _timeAnim(0.0f)
    {}

    ~OutsideScene() {
        delete _character;
        delete _env;
        delete _butterfly1;
        delete _butterfly2;
    }

    void load() override {
        float worldW = 256.0f * SPRITE_SCALE;
        _env = new Environment(worldW);

        float floorY = (PLAY_Y + PLAY_HEIGHT - 1);

        // Plants in foreground
        _env->addObject(LAYER_FOREGROUND, &PLANTER1,
            10*SPRITE_SCALE, floorY - PLANTER1.height * SPRITE_SCALE);
        _env->addObject(LAYER_FOREGROUND, &PLANT1,
            9*SPRITE_SCALE, floorY - (PLANTER1.height + PLANT1.height) * SPRITE_SCALE);

        _env->addObject(LAYER_FOREGROUND, &PLANTER1,
            94*SPRITE_SCALE, floorY - PLANTER1.height * SPRITE_SCALE);
        _env->addObject(LAYER_FOREGROUND, &PLANT2,
            90*SPRITE_SCALE, floorY - (PLANTER1.height + PLANT2.height) * SPRITE_SCALE);

        _env->addObject(LAYER_FOREGROUND, &PLANTER1,
            180*SPRITE_SCALE, floorY - PLANTER1.height * SPRITE_SCALE);
        _env->addObject(LAYER_FOREGROUND, &PLANT1,
            179*SPRITE_SCALE, floorY - (PLANTER1.height + PLANT1.height) * SPRITE_SCALE);

        // Ground + grass callback
        _env->addCustomDraw(LAYER_FOREGROUND, _drawGround, nullptr);

        // Sky gradient callback
        _env->addCustomDraw(LAYER_BACKGROUND, _drawSky, this);

        _character = new CharacterEntity(
            64.0f * SPRITE_SCALE,
            floorY,
            "sitting.forward.neutral",
            _context
        );
        _character->setCurrentBehavior(_character->makeIdleBehavior());
        _character->currentBehavior()->start();

        _butterfly1 = new ButterflyEntity(110*SPRITE_SCALE, 20*SPRITE_SCALE);
        _butterfly2 = new ButterflyEntity(50*SPRITE_SCALE, 30*SPRITE_SCALE);

        _buildMenuItems();
    }

    void unload() override {
        delete _character; _character = nullptr;
        delete _env;       _env       = nullptr;
        delete _butterfly1; _butterfly1 = nullptr;
        delete _butterfly2; _butterfly2 = nullptr;
    }

    void enter() override {}
    void exit() override {}

    SceneResult update(float dt) override {
        _timeAnim += dt;
        if (_character) _character->update(dt);
        if (_butterfly1) _butterfly1->update(dt);
        if (_butterfly2) _butterfly2->update(dt);
        return NO_CHANGE;
    }

    void draw() override {
        _renderer->clear(COLOR_SKY_DAY);

        if (_menuActive) {
            _menu.draw();
            return;
        }

        _env->draw(*_renderer);

        int camOff = (int)_env->cameraX;
        if (_character) _character->draw(*_renderer, true, camOff);
        if (_butterfly1) _butterfly1->draw(*_renderer, camOff);
        if (_butterfly2) _butterfly2->draw(*_renderer, camOff);

        _renderer->drawStatusBar(
            _context->fullness  / 100.0f,
            _context->energy    / 100.0f,
            _context->affection / 100.0f
        );
    }

    SceneResult handleInput() override {
        if (_menuActive) {
            MenuResult r = _menu.handleInput();
            if (r.closed) { _menuActive = false; }
            else if (r.selected && r.item) {
                _menuActive = false;
                _handleMenuAction(r.item);
            }
            return NO_CHANGE;
        }

        if (_input->quickAction()) {
            _menuActive = true;
            _menu.open(_menuItems, _menuItemCount);
            return NO_CHANGE;
        }

        if (_input->prev()) _env->pan(-PAN_SPEED * SPRITE_SCALE);
        if (_input->next()) _env->pan( PAN_SPEED * SPRITE_SCALE);

        return NO_CHANGE;
    }

private:
    Menu              _menu;
    CharacterEntity*  _character;
    Environment*      _env;
    ButterflyEntity*  _butterfly1;
    ButterflyEntity*  _butterfly2;
    bool              _menuActive;
    float             _timeAnim;

    MenuItem  _menuItems[OUTSIDE_MENU_MAX];
    MenuItem  _toysSubmenu[OUTSIDE_TOY_MAX];
    int       _menuItemCount = 0;

    void _buildMenuItems() {
        int n=0;
        _menuItems[n++] = { "Give pets",     nullptr, ACT_CUSTOM, "pets",       nullptr,0,nullptr, nullptr, nullptr };
        _menuItems[n++] = { "Groom",         nullptr, ACT_CUSTOM, "groom",      nullptr,0,nullptr, nullptr, nullptr };
        _menuItems[n++] = { "Point at bird", nullptr, ACT_CUSTOM, "point_bird", nullptr,0,nullptr, nullptr, nullptr };
        _menuItems[n++] = { "Throw stick",   nullptr, ACT_CUSTOM, "throw_stick",nullptr,0,nullptr, nullptr, nullptr };
        _menuItems[n++] = { "Give treat",    nullptr, ACT_CUSTOM, "treat",      nullptr,0,nullptr, nullptr, nullptr };

        int tn=0;
        for (int i=0; i<_context->inventory.toyCount && tn<OUTSIDE_TOY_MAX; i++) {
            _toysSubmenu[tn++] = { _context->inventory.toys[i], nullptr, ACT_CUSTOM,
                                    _context->inventory.toys[i], nullptr,0,nullptr, nullptr, nullptr };
        }
        if (tn>0) _menuItems[n++] = { "Play with toy", nullptr, ACT_NONE, nullptr, _toysSubmenu, tn };
        _menuItemCount = n;
    }

    void _handleMenuAction(const MenuItem* item) {
        if (!item || !item->actionParam) return;
        const char* p = item->actionParam;
        if (strcmp(p,"pets")==0)        { _character->trigger(new AffectionBehavior(_character)); }
        else if (strcmp(p,"groom")==0)  { _character->trigger(new BeingGroomedBehavior(_character)); }
        else if (strcmp(p,"point_bird")==0) { auto* b=new AttentionBehavior(_character); _character->trigger(b); b->start("point_bird"); }
        else if (strcmp(p,"throw_stick")==0){ auto* b=new PlayingBehavior(_character); _character->trigger(b); b->start("throw_stick"); }
        else if (strcmp(p,"treat")==0)  { auto* b=new SnackingBehavior(_character); _character->trigger(b); b->start("treat"); }
        else { auto* b=new PlayingBehavior(_character); _character->trigger(b); b->start("toy"); }
    }

    static void _drawGround(Renderer& r, float camX, float par, void* data) {
        int gy = PLAY_Y + PLAY_HEIGHT - 1;
        r.drawLine(0, gy, DISPLAY_WIDTH, gy, COLOR_GREEN);
        // Grass tufts
        float off = camX * par;
        int tufts[] = {10,35,80,110,150,190,230};
        for (int i=0; i<7; i++) {
            int sx = (int)(tufts[i]*SPRITE_SCALE - off);
            if (sx < -5 || sx > DISPLAY_WIDTH+5) continue;
            r.drawLine(sx, gy, sx-2, gy-4, COLOR_GREEN);
            r.drawLine(sx, gy, sx,   gy-5, COLOR_GREEN);
            r.drawLine(sx, gy, sx+2, gy-4, COLOR_GREEN);
        }
    }

    static void _drawSky(Renderer& r, float camX, float par, void* data) {
        OutsideScene* self = (OutsideScene*)data;
        int hour    = self ? self->_context->environment.time_of_day : 12;
        int weather = self ? self->_context->environment.weather     : 0;
        int moonPh  = self ? self->_context->environment.moon_phase  : 4;
        float anim  = self ? self->_timeAnim : 0.0f;

        // Classify time of day
        bool isNight = (hour < 5 || hour >= 21);
        bool isDawn  = (hour >= 5 && hour <= 7);
        bool isDusk  = (hour >= 18 && hour < 21);

        // Base sky color
        uint16_t skyColor;
        if (isNight)     skyColor = COLOR_SKY_NIGHT;        // deep navy
        else if (isDawn) skyColor = 0xFD60;                 // warm orange-pink
        else if (isDusk) skyColor = 0xCA46;                 // purple-orange
        else             skyColor = COLOR_SKY_DAY;          // bright blue day

        // Overcast tint for cloudy/rain/storm/snow
        if (weather >= 1) skyColor = 0x6B4D;                // flat gray

        int skyH = PLAY_HEIGHT * 2 / 3;
        r.drawRect(0, PLAY_Y, DISPLAY_WIDTH, skyH, skyColor, true);

        if (isNight && weather == 0) {
            // Stars (pseudo-random, twinkle based on anim)
            uint32_t seed = 0xDEADBEEF;
            for (int i = 0; i < 14; i++) {
                seed = seed * 1664525u + 1013904223u;
                int sx = (int)((seed >> 16) & 0xFFFF) % (DISPLAY_WIDTH - 4) + 2;
                seed = seed * 1664525u + 1013904223u;
                int sy = PLAY_Y + 2 + (int)((seed >> 16) & 0xFFFF) % (skyH - 4);
                if ((int)(anim * 3 + i) % 5 != 0)   // occasional twinkle off
                    r.drawPixel(sx, sy, COLOR_WHITE);
            }
            // Moon — shape based on moon_phase (0=new..4=full..7=waning)
            int moonX = DISPLAY_WIDTH - 22, moonY = PLAY_Y + 10;
            if (moonPh > 0) {
                r.drawCircle(moonX, moonY, 5, COLOR_WHITE, true);
                if (moonPh < 4) {
                    // Waxing — shadow on left
                    int off = (4 - moonPh) * 2;
                    r.drawCircle(moonX - off, moonY, 5, skyColor, true);
                } else if (moonPh > 4) {
                    // Waning — shadow on right
                    int off = (moonPh - 4) * 2;
                    r.drawCircle(moonX + off, moonY, 5, skyColor, true);
                }
            }
        } else if (!isNight && weather < 2) {
            // Sun arc: rises at left (6am), peaks center (12pm), sets right (18pm)
            float t = (float)(hour - 6) / 12.0f;
            if (t < 0) t = 0; if (t > 1) t = 1;
            int sunX = (int)(8 + t * (DISPLAY_WIDTH - 16));
            int sunY = PLAY_Y + skyH - 6 - (int)(sin(t * M_PI) * (skyH - 12));
            r.drawCircle(sunX, sunY, 6, COLOR_YELLOW, true);
            r.drawCircle(sunX, sunY, 8, COLOR_ORANGE, false);
        } else if (isDawn || isDusk) {
            // Glow on horizon
            int sunY = PLAY_Y + skyH - 6;
            int sunX = isDawn ? 20 : DISPLAY_WIDTH - 20;
            r.drawCircle(sunX, sunY, 9, COLOR_ORANGE, true);
            r.drawCircle(sunX, sunY, 6, COLOR_YELLOW, true);
        }

        // Weather particles
        if (weather == 2 || weather == 3) {
            // Rain — diagonal lines scrolling down
            for (int i = 0; i < 18; i++) {
                uint32_t s = (uint32_t)(i * 73 + 17);
                int rx = (int)((s * 1664525u + 1013904223u) >> 16) % DISPLAY_WIDTH;
                int ry = PLAY_Y + (int)(anim * 80.0f + i * (PLAY_HEIGHT / 18.0f)) % PLAY_HEIGHT;
                r.drawLine(rx, ry, rx - 1, ry + 5, 0x949F);
            }
        } else if (weather == 4) {
            // Snow — drifting flakes
            for (int i = 0; i < 10; i++) {
                uint32_t s = (uint32_t)(i * 97 + 31);
                int sx = (int)((s * 1664525u + 1013904223u) >> 16) % DISPLAY_WIDTH;
                int sy = PLAY_Y + (int)(anim * 25.0f + i * (PLAY_HEIGHT / 10.0f)) % PLAY_HEIGHT;
                r.drawPixel(sx, sy, COLOR_WHITE);
                r.drawPixel(sx + 1, sy, COLOR_WHITE);
            }
        }
    }
};
