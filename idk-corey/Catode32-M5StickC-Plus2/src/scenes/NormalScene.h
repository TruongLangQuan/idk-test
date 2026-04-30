#pragma once
// NormalScene.h - Main indoor gameplay scene

#include "Scene.h"
#include "Environment.h"
#include "Menu.h"
#include "entities/CharacterEntity.h"
#include "entities/behaviors/AffectionBehavior.h"
#include "entities/behaviors/AttentionBehavior.h"
#include "entities/behaviors/SnackingBehavior.h"
#include "entities/behaviors/PlayingBehavior.h"
#include "entities/behaviors/BeingGroomedBehavior.h"
#include "entities/behaviors/EatingBehavior.h"
#include "assets/furniture_assets.h"
#include "assets/nature_assets.h"
#include "assets/items_assets.h"
#include "assets/icons_assets.h"

static const int NORMAL_MENU_MAX  = 20;
static const int NORMAL_SUBMENU_MAX = 8;

class NormalScene : public Scene {
public:
    NormalScene(GameContext* ctx, Renderer* r, InputHandler* inp)
        : Scene(ctx,r,inp), _menu(r,inp),
          _character(nullptr), _env(nullptr),
          _menuActive(false), _fishAngle(0.0f),
          _fishObj(nullptr), _foodBowlObj(nullptr), _eatingBehavior(nullptr)
    {}

    ~NormalScene() {
        delete _character;
        delete _env;
    }

    void load() override {
        _env = new Environment(192.0f * SPRITE_SCALE);

        // ── Furniture ────────────────────────────────────────────
        float floorY = (PLAY_Y + PLAY_HEIGHT - 1);

        _env->addObject(LAYER_FOREGROUND, &BOOKSHELF,
            0, floorY - BOOKSHELF.height * SPRITE_SCALE);
        _env->addObject(LAYER_FOREGROUND, &BOX_SMALL_1,
            2*SPRITE_SCALE, floorY - (BOOKSHELF.height + BOX_SMALL_1.height) * SPRITE_SCALE);
        _env->addObject(LAYER_FOREGROUND, &PLANTER_SMALL_1,
            14*SPRITE_SCALE, floorY - (BOOKSHELF.height + PLANTER_SMALL_1.height)*SPRITE_SCALE);

        _env->addObject(LAYER_FOREGROUND, &PLANTER1,
            42*SPRITE_SCALE, floorY - PLANTER1.height * SPRITE_SCALE);
        _env->addObject(LAYER_FOREGROUND, &PLANT3,
            43*SPRITE_SCALE, floorY - (PLANTER1.height + PLANT3.height) * SPRITE_SCALE);

        // Fish (animated — we update its frame externally)
        _fishObj = _env->addObject(LAYER_FOREGROUND, &FISH1,
            160*SPRITE_SCALE, 20*SPRITE_SCALE);

        // Right-side plants (visible when panned)
        _env->addObject(LAYER_FOREGROUND, &PLANTER1,
            140*SPRITE_SCALE, floorY - PLANTER1.height * SPRITE_SCALE);
        _env->addObject(LAYER_FOREGROUND, &PLANT3,
            141*SPRITE_SCALE, floorY - (PLANTER1.height + PLANT3.height) * SPRITE_SCALE);

        // ── Ground line ──────────────────────────────────────────
        _env->addCustomDraw(LAYER_FOREGROUND, _drawGround, this);

        // ── Character ────────────────────────────────────────────
        _character = new CharacterEntity(
            100.0f * SPRITE_SCALE,
            (float)(PLAY_Y + PLAY_HEIGHT - 1),
            "sitting.forward.neutral",
            _context
        );

        // Start idle behavior
        _character->setCurrentBehavior(_character->makeIdleBehavior());
        _character->currentBehavior()->start();

        _buildMenuItems();
    }

    void unload() override {
        delete _character; _character = nullptr;
        delete _env;       _env       = nullptr;
    }

    void enter() override {}
    void exit() override {}

    SceneResult update(float dt) override {
        if (!_character || !_env) return NO_CHANGE;

        _character->update(dt);

        // Animate fish rotation (frame cycling)
        _fishAngle += dt * 25.0f;
        if (_fishObj) _fishObj->frame = ((int)(_fishAngle / 15.0f)) % max(1, FISH1.frame_count);

        // Sync food bowl with eating behavior
        if (_foodBowlObj && _eatingBehavior) {
            if (_eatingBehavior->active()) {
                int bx, by;
                _eatingBehavior->getBowlPosition(_character->x, _character->y, false, bx, by);
                _foodBowlObj->x     = bx;
                _foodBowlObj->y     = by;
                _foodBowlObj->frame = _eatingBehavior->getBowlFrame();
            } else {
                // Eating ended — remove bowl
                _env->removeObject(_foodBowlObj);
                _foodBowlObj    = nullptr;
                _eatingBehavior = nullptr;
            }
        }

        return NO_CHANGE;
    }

    void draw() override {
        // Room lighting based on time of day
        int hour = _context->environment.time_of_day;
        bool isNight = (hour < 6 || hour >= 20);
        uint16_t bgColor = isNight ? COLOR_BLACK : 0x1082; // black at night, very dark gray in day
        _renderer->clear(bgColor);

        if (_menuActive) {
            _menu.draw();
            return;
        }

        // Draw environment
        _env->draw(*_renderer);

        // Draw character
        int camOff = (int)_env->cameraX;
        _character->draw(*_renderer, false, camOff);

        // Status bar
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

        // Open care menu on quick action (BtnA long)
        if (_input->quickAction()) {
            _menuActive = true;
            _menu.open(_menuItems, _menuItemCount);
            return NO_CHANGE;
        }

        // Camera pan: PWR=left, A short=right
        if (_input->prev()) _env->pan(-PAN_SPEED * SPRITE_SCALE);
        if (_input->next()) _env->pan( PAN_SPEED * SPRITE_SCALE);

        return NO_CHANGE;
    }

private:
    Menu             _menu;
    CharacterEntity* _character;
    Environment*     _env;
    bool             _menuActive;
    float            _fishAngle;
    EnvObject*       _fishObj;
    EnvObject*       _foodBowlObj;
    EatingBehavior*  _eatingBehavior;

    // Pre-built menu items
    MenuItem  _menuItems[NORMAL_MENU_MAX];
    MenuItem  _mealsSubmenu[4];
    MenuItem  _snacksSubmenu[NORMAL_SUBMENU_MAX];
    MenuItem  _toysSubmenu[NORMAL_SUBMENU_MAX];
    int       _menuItemCount = 0;

    void _buildMenuItems() {
        int n = 0;
        _menuItems[n++] = { "Give pets",  nullptr, ACT_CUSTOM, "pets",  nullptr,0,nullptr, _onMenuAct, this };
        _menuItems[n++] = { "Psst psst",  nullptr, ACT_CUSTOM, "psst",  nullptr,0,nullptr, _onMenuAct, this };
        _menuItems[n++] = { "Give kiss",  nullptr, ACT_CUSTOM, "kiss",  nullptr,0,nullptr, _onMenuAct, this };
        _menuItems[n++] = { "Groom",      nullptr, ACT_CUSTOM, "groom", nullptr,0,nullptr, _onMenuAct, this };

        // Meals submenu
        int m=0;
        _mealsSubmenu[m++] = { "Chicken", nullptr, ACT_CUSTOM, "meal:chicken", nullptr,0,nullptr, _onMenuAct, this };
        _mealsSubmenu[m++] = { "Fish",    nullptr, ACT_CUSTOM, "meal:fish",    nullptr,0,nullptr, _onMenuAct, this };
        _menuItems[n++] = { "Meals", nullptr, ACT_NONE, nullptr, _mealsSubmenu, m };

        // Snacks from inventory
        int sn=0;
        for (int i=0; i<_context->inventory.snackCount && sn<NORMAL_SUBMENU_MAX; i++) {
            _snacksSubmenu[sn] = { _context->inventory.snacks[i], nullptr, ACT_CUSTOM,
                                    _context->inventory.snacks[i], nullptr,0,nullptr, _onMenuAct, this };
            sn++;
        }
        if (sn>0) _menuItems[n++] = { "Give snacks", nullptr, ACT_NONE, nullptr, _snacksSubmenu, sn };

        // Toys from inventory
        int tn=0;
        for (int i=0; i<_context->inventory.toyCount && tn<NORMAL_SUBMENU_MAX; i++) {
            _toysSubmenu[tn] = { _context->inventory.toys[i], nullptr, ACT_CUSTOM,
                                  _context->inventory.toys[i], nullptr,0,nullptr, _onMenuAct, this };
            tn++;
        }
        if (tn>0) _menuItems[n++] = { "Use toys", nullptr, ACT_NONE, nullptr, _toysSubmenu, tn };

        _menuItemCount = n;
    }

    void _handleMenuAction(const MenuItem* item) {
        if (!item || !item->actionParam) return;
        const char* p = item->actionParam;

        if (strcmp(p,"pets")==0)  { _character->trigger(new AffectionBehavior(_character)); }
        else if (strcmp(p,"kiss")==0)  { auto* b=new AffectionBehavior(_character); _character->trigger(b); b->start("kiss"); }
        else if (strcmp(p,"psst")==0)  { auto* b=new AttentionBehavior(_character); _character->trigger(b); b->start("psst"); }
        else if (strcmp(p,"groom")==0) { _character->trigger(new BeingGroomedBehavior(_character)); }
        else if (strncmp(p,"meal:",5)==0) { _startEating(p+5); }
        else if (strcmp(p,"Treat")==0) { auto* b=new SnackingBehavior(_character); _character->trigger(b); b->start("treat"); }
        else { // toy or snack
            auto* b=new PlayingBehavior(_character); _character->trigger(b); b->start("toy");
        }
    }

    void _startEating(const char* mealType) {
        auto* b = new EatingBehavior(_character);
        b->start(&FOOD_BOWL, mealType, [](bool, float, void* ud){
            NormalScene* self = (NormalScene*)ud;
            if (self->_foodBowlObj) {
                self->_env->removeObject(self->_foodBowlObj);
                self->_foodBowlObj = nullptr;
            }
            self->_eatingBehavior = nullptr;
        }, this);

        _eatingBehavior = b;
        _character->setCurrentBehavior(b);

        // Add food bowl to environment
        int bx, by;
        b->getBowlPosition(_character->x, _character->y, false, bx, by);
        _foodBowlObj = _env->addObject(LAYER_FOREGROUND, &FOOD_BOWL, bx, by);
    }

    static void _onMenuAct(void* ud) {
        // Used by callback-based menu items; handled via handleMenuAction instead
    }

    static void _drawGround(Renderer& r, float camX, float par, void* data) {
        float off = camX * par;
        int groundY = PLAY_Y + PLAY_HEIGHT - 1;
        r.drawLine(0, groundY, DISPLAY_WIDTH, groundY, COLOR_WHITE);
    }
};
