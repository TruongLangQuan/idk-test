#pragma once
// SnakeScene.h - Classic snake minigame
// BtnB short = turn right (clockwise).  BtnPWR short = turn left (counter-clockwise).
// BtnA short = start (from idle screen).  BtnB long = exit.

#include "Scene.h"

class SnakeScene : public Scene {
public:
    static const int COLS      = 20;
    static const int ROWS      = 14;
    static const int CEL       = 9;   // cell pixel size
    static const int GRID_X    = (DISPLAY_WIDTH  - COLS * CEL) / 2;
    static const int GRID_Y    = PLAY_Y + (PLAY_HEIGHT - ROWS * CEL) / 2;
    static const int MAX_LEN   = COLS * ROWS;
    static const int MOVE_TIME_MS = 180; // ms per step (decrease for difficulty)

    SnakeScene(GameContext* ctx, Renderer* r, InputHandler* inp)
        : Scene(ctx,r,inp), _state(ST_IDLE) {}

    void enter() override { _reset(); }

    SceneResult update(float dt) override {
        if (_state != ST_PLAYING) return NO_CHANGE;

        _stepTimer += dt * 1000.0f;
        while (_stepTimer >= _moveMs) {
            _stepTimer -= _moveMs;
            _step();
            if (_state != ST_PLAYING) break;
        }
        return NO_CHANGE;
    }

    void draw() override {
        _renderer->clear();

        // Grid border
        _renderer->drawRect(GRID_X - 1, GRID_Y - 1,
                            COLS * CEL + 2, ROWS * CEL + 2, COLOR_UI_BORDER, false);

        // Food
        int fx = GRID_X + _foodX * CEL + 1;
        int fy = GRID_Y + _foodY * CEL + 1;
        _renderer->drawRect(fx, fy, CEL - 2, CEL - 2, COLOR_YELLOW, true);

        // Snake
        for (int i = 0; i < _len; i++) {
            int sx = GRID_X + _bodyX[i] * CEL + 1;
            int sy = GRID_Y + _bodyY[i] * CEL + 1;
            uint16_t col = (i == 0) ? COLOR_UI_SELECT : COLOR_GREEN;
            _renderer->drawRect(sx, sy, CEL - 2, CEL - 2, col, true);
        }

        // HUD
        char buf[32];
        snprintf(buf, sizeof(buf), "Score:%d Hi:%d", _score, _context->highScoreMaze);
        _renderer->drawText(buf, 4, PLAY_Y + 2, COLOR_UI_TEXT, COLOR_BLACK, 1);

        if (_state == ST_IDLE) {
            _renderer->drawText("A: Start", 84, GRID_Y + ROWS * CEL / 2 - 5,
                                COLOR_UI_SELECT, COLOR_BLACK, 1);
            _renderer->drawText("B:turn R  PWR:turn L", 30, GRID_Y + ROWS * CEL / 2 + 8,
                                COLOR_UI_DIM, COLOR_BLACK, 1);
        }
        if (_state == ST_DEAD) {
            _renderer->drawText("Game Over!", 68, GRID_Y + ROWS * CEL / 2 - 5,
                                COLOR_RED, COLOR_BLACK, 1);
            _renderer->drawText("A: Restart", 68, GRID_Y + ROWS * CEL / 2 + 8,
                                COLOR_UI_DIM, COLOR_BLACK, 1);
        }
    }

    SceneResult handleInput() override {
        if (_input->btnPWR_shortPress()) {
            _saveHigh();
            return changeTo("normal");
        }

        if (_state == ST_IDLE || _state == ST_DEAD) {
            if (_input->btnA_shortPress()) {  // BtnA = start/restart
                _reset();
                _state = ST_PLAYING;
            }
            return NO_CHANGE;
        }

        // BtnB short = turn right (clockwise)
        if (_input->btnB_shortPress()) _dir = (_dir + 1) % 4;
        // BtnA short = turn left (counter-clockwise)
        if (_input->btnA_shortPress())  _dir = (_dir + 3) % 4;

        return NO_CHANGE;
    }

private:
    enum State { ST_IDLE, ST_PLAYING, ST_DEAD };

    // dir: 0=up, 1=right, 2=down, 3=left
    static const int DX[4]; // defined below
    static const int DY[4];

    State _state;
    int   _bodyX[MAX_LEN], _bodyY[MAX_LEN];
    int   _len;
    int   _dir;
    int   _foodX, _foodY;
    int   _score;
    float _stepTimer;
    float _moveMs;

    void _reset() {
        _len  = 3;
        _dir  = 1; // right
        _bodyX[0] = COLS/2;     _bodyY[0] = ROWS/2;
        _bodyX[1] = COLS/2 - 1; _bodyY[1] = ROWS/2;
        _bodyX[2] = COLS/2 - 2; _bodyY[2] = ROWS/2;
        _score     = 0;
        _stepTimer = 0;
        _moveMs    = MOVE_TIME_MS;
        _placeFood();
    }

    void _step() {
        int nx = _bodyX[0] + DX[_dir];
        int ny = _bodyY[0] + DY[_dir];

        // Wall collision
        if (nx < 0 || nx >= COLS || ny < 0 || ny >= ROWS) { _die(); return; }

        // Self collision
        for (int i = 0; i < _len - 1; i++)
            if (_bodyX[i] == nx && _bodyY[i] == ny) { _die(); return; }

        // Check food
        bool ate = (nx == _foodX && ny == _foodY);

        // Shift body
        if (!ate) {
            for (int i = _len - 1; i > 0; i--) {
                _bodyX[i] = _bodyX[i-1];
                _bodyY[i] = _bodyY[i-1];
            }
        } else {
            if (_len < MAX_LEN) {
                for (int i = _len; i > 0; i--) {
                    _bodyX[i] = _bodyX[i-1];
                    _bodyY[i] = _bodyY[i-1];
                }
                _len++;
            }
            _score++;
            _saveHigh();
            _placeFood();
            // Speed up every 5 points
            if (_score % 5 == 0 && _moveMs > 80) _moveMs -= 10;
        }
        _bodyX[0] = nx;
        _bodyY[0] = ny;
    }

    void _die() {
        _state = ST_DEAD;
        _saveHigh();
    }

    void _saveHigh() {
        if (_score > _context->highScoreMaze)
            _context->highScoreMaze = _score;
    }

    void _placeFood() {
        // Find a free cell
        for (int attempt = 0; attempt < 200; attempt++) {
            int fx = random(COLS);
            int fy = random(ROWS);
            bool ok = true;
            for (int i = 0; i < _len; i++)
                if (_bodyX[i] == fx && _bodyY[i] == fy) { ok = false; break; }
            if (ok) { _foodX = fx; _foodY = fy; return; }
        }
    }
};

const int SnakeScene::DX[4] = { 0, 1, 0, -1 };
const int SnakeScene::DY[4] = { -1, 0, 1, 0 };
