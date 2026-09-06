#pragma once
// ZoomiesScene.h - Endless runner minigame
// BtnA short = jump / start.  BtnB long = back / forfeit.

#include "Scene.h"
#include "assets/minigame_char_assets.h"

class ZoomiesScene : public Scene {
public:
    static const int GRAVITY    = 800;   // px/s²
    static const int JUMP_VEL   = -420;  // px/s  upward
    static const int FLOOR_Y    = PLAY_Y + PLAY_HEIGHT - 12;
    static const int BASE_SPEED = 110;   // px/s world scroll
    static const int SPEED_INC  = 8;     // added per obstacle cleared

    ZoomiesScene(GameContext* ctx, Renderer* r, InputHandler* inp)
        : Scene(ctx,r,inp), _state(ST_IDLE) {}

    void enter() override { _reset(); }

    SceneResult update(float dt) override {
        switch(_state) {
        case ST_IDLE:
            break;
        case ST_PLAYING:
            _update(dt);
            break;
        case ST_DEAD:
            _deadTimer -= dt;
            if (_deadTimer <= 0) _state = ST_IDLE;
            break;
        }
        return NO_CHANGE;
    }

    void draw() override {
        _renderer->clear();

        // Ground
        _renderer->drawLine(0, FLOOR_Y+1, DISPLAY_WIDTH, FLOOR_Y+1, COLOR_WHITE);

        // Player — y position follows physics (_py = bottom of character)
        {
            int ph = RUNCAT1.height * SPRITE_SCALE;
            int frame = _state==ST_DEAD ? 0 : (int)(_animTime * 8) % max(1, RUNCAT1.frame_count);
            _renderer->drawSpriteObj(&RUNCAT1, (int)_px, (int)_py - ph, false, frame);
        }

        // Obstacles (drawn as simple filled rects)
        for (int i=0; i<MAX_OBS; i++) {
            if (!_obs[i].active) continue;
            int oh = _obs[i].h;
            _renderer->drawRect(_obs[i].x, FLOOR_Y - oh, _obs[i].w, oh, COLOR_RED, true);
        }

        // HUD
        char buf[24];
        snprintf(buf, sizeof(buf), "Score:%d", _score);
        _renderer->drawText(buf, 4, PLAY_Y+2, COLOR_UI_TEXT, COLOR_BLACK, 1);

        if (_state == ST_IDLE) {
            _renderer->drawText("A: Start / Jump", 30, PLAY_Y+PLAY_HEIGHT/2-5,                                COLOR_UI_SELECT, COLOR_BLACK, 1);
        }
        if (_state == ST_DEAD) {
            _renderer->drawText("Ouch!", 90, PLAY_Y+PLAY_HEIGHT/2-5,
                                COLOR_RED, COLOR_BLACK, 1);
        }
    }

    SceneResult handleInput() override {
        if (_input->btnPWR_shortPress()) {
            if (_score > _context->highScoreZoomies)
                _context->highScoreZoomies = _score;
            return changeTo("normal");
        }
        // BtnA short = jump / start
        if (_input->btnA_shortPress()) {
            if (_state == ST_IDLE) { _reset(); _state = ST_PLAYING; }
            else if (_state == ST_PLAYING && _py >= FLOOR_Y - 2) { _vy = JUMP_VEL; }
        }
        return NO_CHANGE;
    }

private:
    enum State { ST_IDLE, ST_PLAYING, ST_DEAD };

    static const int MAX_OBS  = 4;
    static const int PLAYER_W = 16;
    static const int PLAYER_H = 20;

    struct Obs { bool active; int x, w, h, type; };

    State _state;
    float _px, _py, _vy;
    float _speed, _animTime;
    float _spawnTimer, _spawnInterval;
    float _deadTimer;
    int   _score;
    Obs   _obs[MAX_OBS];

    void _reset() {
        _px = 30;
        _py = FLOOR_Y;
        _vy = 0;
        _speed = BASE_SPEED;
        _animTime = 0;
        _spawnTimer = 0;
        _spawnInterval = 2.2f;
        _deadTimer = 1.5f;
        _score = 0;
        for (int i=0; i<MAX_OBS; i++) _obs[i].active=false;
    }

    void _update(float dt) {
        _animTime += dt;

        // Player physics
        _vy += GRAVITY * dt;
        _py += _vy * dt;
        if (_py >= FLOOR_Y) { _py = FLOOR_Y; _vy = 0; }

        // Spawn obstacle
        _spawnTimer += dt;
        if (_spawnTimer >= _spawnInterval) {
            _spawnTimer = 0;
            _spawnInterval = 1.5f + (float)(random(0, 100)) / 100.0f * 1.2f;
            _spawnObs();
        }

        // Move obstacles
        for (int i=0; i<MAX_OBS; i++) {
            if (!_obs[i].active) continue;
            _obs[i].x -= (int)(_speed * dt);
            if (_obs[i].x + _obs[i].w < 0) {
                _obs[i].active = false;
                _score++;
                _speed += SPEED_INC;
            }
        }

        // Collision — use actual sprite dimensions with a small forgiveness margin
        int spriteW    = RUNCAT1.width  * SPRITE_SCALE;
        int spriteH    = RUNCAT1.height * SPRITE_SCALE;
        int charBottom = (int)_py       - 3;
        int charTop    = (int)_py       - spriteH + 3;
        int charLeft   = (int)_px       + 3;
        int charRight  = (int)_px       + spriteW - 3;
        for (int i=0; i<MAX_OBS; i++) {
            if (!_obs[i].active) continue;
            int obsBottom = FLOOR_Y;
            int obsTop    = obsBottom - _obs[i].h;
            if (charRight >= _obs[i].x && charLeft <= _obs[i].x + _obs[i].w
                && charBottom >= obsTop && charTop <= obsBottom) {
                _state = ST_DEAD;
                _deadTimer = 2.0f;
                if (_score > _context->highScoreZoomies)
                    _context->highScoreZoomies = _score;
                return;
            }
        }
    }

    void _spawnObs() {
        for (int i=0; i<MAX_OBS; i++) {
            if (!_obs[i].active) {
                _obs[i].active = true;
                _obs[i].x      = DISPLAY_WIDTH + 4;
                _obs[i].type   = random(0,2);
                _obs[i].w      = 10 * SPRITE_SCALE;
                _obs[i].h      = (_obs[i].type==0) ? 16 : 24;
                return;
            }
        }
    }
};
