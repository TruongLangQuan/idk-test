#pragma once
// SceneManager.h - Scene lifecycle, overlay routing, main menu

#include "Scene.h"
#include "Menu.h"
#include "Settings.h"
#include "Transitions.h"
#include "GameContext.h"
#include "Renderer.h"
#include "Input.h"
#include "scenes/NormalScene.h"
#include "scenes/OutsideScene.h"
#include "scenes/StatsScene.h"
#include "scenes/ZoomiesScene.h"
#include "scenes/MazeScene.h"
#include "scenes/BreakoutScene.h"
#include "scenes/TicTacToeScene.h"
#include "scenes/SnakeScene.h"

// Scene IDs
enum class SceneID {
    NONE, NORMAL, OUTSIDE, STATS, ZOOMIES, MAZE, BREAKOUT, TICTACTOE, SNAKE
};

// ── Minigames submenu entries (built once) ────────────────────────────────────
static const char* MINIGAME_NAMES[] = { "Zoomies", "Breakout", "Tic-Tac-Toe", "Snake" };
static SceneID     MINIGAME_IDS[]   = {
    SceneID::ZOOMIES, SceneID::BREAKOUT, SceneID::TICTACTOE, SceneID::SNAKE
};
static const int MINIGAME_COUNT = 4;

class SceneManager {
public:
    SceneManager(GameContext* ctx, Renderer* r, InputHandler* inp)
        : _ctx(ctx), _renderer(r), _inp(inp),
          _current(nullptr), _currentID(SceneID::NONE),
          _mainMenu(r, inp), _settings(r, inp, ctx),
          _trans(r),
          _mainMenuOpen(false), _settingsOpen(false),
          _pendingID(SceneID::NONE),
          _cache{nullptr, SceneID::NONE, nullptr, SceneID::NONE}
    {}

    ~SceneManager() {
        _destroyCache(0);
        _destroyCache(1);
    }

    void begin() {
        _buildMainMenu();
        _switchTo(SceneID::NORMAL, false);
    }

    void update(float dt) {
        // Pending scene transition from last frame
        if (_pendingID != SceneID::NONE) {
            _switchTo(_pendingID, true);
            _pendingID = SceneID::NONE;
        }

        // Settings overlay
        if (_settingsOpen) {
            _settings.handleInput();
            if (!_settings.isOpen()) _settingsOpen = false;
            return;
        }

        // Main menu overlay
        if (_mainMenuOpen) {
            MenuResult r = _mainMenu.handleInput();
            if (r.closed) { _mainMenuOpen = false; }
            else if (r.selected && r.item) {
                _mainMenuOpen = false;
                _handleMainMenuAction(r.item);
            }
            return;
        }

        // Scene update + input
        if (_current) {
            SceneResult result = _current->handleInput();
            if (result.changeScene) {
                _pendingID = _sceneIDFromName(result.sceneName);
            } else {
                result = _current->update(dt);
                if (result.changeScene)
                    _pendingID = _sceneIDFromName(result.sceneName);
            }
        }

        // Double-press PWR = open main menu
        if (_inp->mainMenu()) {
            _openMainMenu();
        }
    }

    void draw() {
        if (_current) _current->draw();
        if (_settingsOpen) _settings.draw();
        if (_mainMenuOpen) _mainMenu.draw();
    }

private:
    GameContext*  _ctx;
    Renderer*     _renderer;
    InputHandler* _inp;

    Scene*   _current;
    SceneID  _currentID;

    Menu      _mainMenu;
    Settings  _settings;
    TransitionManager _trans;

    bool    _mainMenuOpen, _settingsOpen;
    SceneID _pendingID;

    // LRU scene cache (max 2 entries)
    struct CacheEntry { Scene* scene; SceneID id; };
    CacheEntry _cache[2];

    // Main menu items + submenus
    static const int MAIN_MENU_MAX = 10;
    static const int MINI_MENU_MAX = 6;
    MenuItem _mainMenuItems[MAIN_MENU_MAX];
    MenuItem _miniSubItems[MINI_MENU_MAX];
    int      _mainMenuCount = 0;

    void _buildMainMenu() {
        _mainMenuCount = 0;

        _mainMenuItems[_mainMenuCount++] = { "Go inside",     nullptr, ACT_CUSTOM, "goto:normal",   nullptr,0,nullptr, nullptr, nullptr };
        _mainMenuItems[_mainMenuCount++] = { "Go outside",    nullptr, ACT_CUSTOM, "goto:outside",  nullptr,0,nullptr, nullptr, nullptr };
        _mainMenuItems[_mainMenuCount++] = { "Pet stats",     nullptr, ACT_CUSTOM, "goto:stats",    nullptr,0,nullptr, nullptr, nullptr };

        // Minigames submenu
        int mn=0;
        for (int i=0; i<MINIGAME_COUNT; i++) {
            _miniSubItems[mn++] = { MINIGAME_NAMES[i], nullptr, ACT_CUSTOM,
                                     MINIGAME_NAMES[i], nullptr,0,nullptr, nullptr, nullptr };
        }
        _mainMenuItems[_mainMenuCount++] = { "Minigames", nullptr, ACT_NONE, nullptr, _miniSubItems, mn };

        _mainMenuItems[_mainMenuCount++] = { "Environment",   nullptr, ACT_CUSTOM, "settings",       nullptr,0,nullptr, nullptr, nullptr };
    }

    void _openMainMenu() {
        _mainMenuOpen = true;
        _mainMenu.open(_mainMenuItems, _mainMenuCount);
    }

    void _handleMainMenuAction(const MenuItem* item) {
        if (!item || !item->actionParam) return;
        const char* p = item->actionParam;
        if (strncmp(p, "goto:", 5)==0) {
            _pendingID = _sceneIDFromName(p+5);
        } else if (strcmp(p,"settings")==0) {
            _settingsOpen = true;
            _settings.open();
        } else {
            // Minigame name
            for (int i=0; i<MINIGAME_COUNT; i++) {
                if (strcmp(p, MINIGAME_NAMES[i])==0) {
                    _pendingID = MINIGAME_IDS[i];
                    break;
                }
            }
        }
    }

    Scene* _makeScene(SceneID id) {
        switch(id) {
        case SceneID::NORMAL:   return new NormalScene(_ctx, _renderer, _inp);
        case SceneID::OUTSIDE:  return new OutsideScene(_ctx, _renderer, _inp);
        case SceneID::STATS:    return new StatsScene(_ctx, _renderer, _inp);
        case SceneID::ZOOMIES:  return new ZoomiesScene(_ctx, _renderer, _inp);
        case SceneID::MAZE:     return new MazeScene(_ctx, _renderer, _inp);
        case SceneID::BREAKOUT: return new BreakoutScene(_ctx, _renderer, _inp);
        case SceneID::TICTACTOE:return new TicTacToeScene(_ctx, _renderer, _inp);
        case SceneID::SNAKE:    return new SnakeScene(_ctx, _renderer, _inp);
        default: return nullptr;
        }
    }

    SceneID _sceneIDFromName(const char* name) {
        if (!name) return SceneID::NONE;
        if (strcmp(name,"normal")==0)   return SceneID::NORMAL;
        if (strcmp(name,"outside")==0)  return SceneID::OUTSIDE;
        if (strcmp(name,"stats")==0)    return SceneID::STATS;
        if (strcmp(name,"zoomies")==0)  return SceneID::ZOOMIES;
        if (strcmp(name,"maze")==0)     return SceneID::MAZE;
        if (strcmp(name,"breakout")==0) return SceneID::BREAKOUT;
        if (strcmp(name,"tictactoe")==0)return SceneID::TICTACTOE;
        if (strcmp(name,"snake")==0)    return SceneID::SNAKE;
        return SceneID::NONE;
    }

    void _switchTo(SceneID id, bool doTransition) {
        if (id == SceneID::NONE || id == _currentID) return;

        // Exit current scene
        if (_current) {
            _current->exit();
        }

        // Find in cache
        Scene* next = nullptr;
        int cacheSlot = -1;
        for (int i=0; i<2; i++) {
            if (_cache[i].id == id && _cache[i].scene) {
                next      = _cache[i].scene;
                cacheSlot = i;
                break;
            }
        }

        if (!next) {
            // Create new scene and load
            next = _makeScene(id);
            if (!next) return;
            next->load();

            // Store in cache (evict LRU = slot 1)
            // Slide cache: slot1 ← slot0, slot0 ← new
            _destroyCache(1);
            _cache[1] = _cache[0];
            _cache[0] = { next, id };
        }

        // Transition (simple direct switch for now)
        if (doTransition && _current) {
            _current   = next;
            _currentID = id;
            _current->enter();
        } else {
            _current   = next;
            _currentID = id;
            _current->enter();
        }
    }

    void _destroyCache(int slot) {
        if (_cache[slot].scene) {
            if (_cache[slot].scene != _current) {
                _cache[slot].scene->unload();
                delete _cache[slot].scene;
            }
            _cache[slot] = { nullptr, SceneID::NONE };
        }
    }
};
