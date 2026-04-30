#pragma once
// BreakoutScene.h - Brick breaker minigame
// BtnA short = launch ball.  BtnB held = paddle right.  BtnPWR held = paddle left.  BtnB long = exit.

#include "Scene.h"

class BreakoutScene : public Scene {
public:
    static const int BRICK_COLS  = 10;
    static const int BRICK_ROWS  = 4;
    static const int BRICK_W     = 22;
    static const int BRICK_H     = 7;
    static const int BRICK_GAP   = 1;
    static const int PADDLE_W    = 30;
    static const int PADDLE_H    = 4;
    static const int BALL_R      = 3;
    static const int PADDLE_SPEED= 280;
    static const int BALL_SPEED  = 110;

    BreakoutScene(GameContext* ctx, Renderer* r, InputHandler* inp)
        : Scene(ctx,r,inp), _state(ST_IDLE) {}

    void enter() override { _reset(); }

    void draw() override {
        _renderer->clear();

        // Bricks
        int brickStartX = (DISPLAY_WIDTH - (BRICK_COLS*(BRICK_W+BRICK_GAP)-BRICK_GAP)) / 2;
        int brickStartY = PLAY_Y + 18;
        for (int row=0; row<BRICK_ROWS; row++) {
            for (int col=0; col<BRICK_COLS; col++) {
                if (!_bricks[row][col]) continue;
                int bx = brickStartX + col*(BRICK_W+BRICK_GAP);
                int by = brickStartY + row*(BRICK_H+BRICK_GAP);
                uint16_t col_color = (row==0)?COLOR_RED : (row==1)?COLOR_ORANGE
                                   : (row==2)?COLOR_YELLOW : COLOR_GREEN;
                _renderer->drawRect(bx, by, BRICK_W, BRICK_H, col_color, true);
                _renderer->drawRect(bx, by, BRICK_W, BRICK_H, COLOR_BLACK, false);
            }
        }

        // Paddle
        _renderer->drawRect((int)_paddleX - PADDLE_W/2,
            PLAY_Y + PLAY_HEIGHT - PADDLE_H - 4,
            PADDLE_W, PADDLE_H, COLOR_WHITE, true);

        // Ball
        _renderer->drawCircle((int)_ballX, (int)_ballY, BALL_R, COLOR_UI_SELECT, true);

        // HUD
        char buf[28];
        snprintf(buf, sizeof(buf), "Lvl:%d Sc:%d Hi:%d",
                 _level, _score, _context->highScoreBreakout);
        _renderer->drawText(buf, 4, PLAY_Y+2, COLOR_UI_TEXT, COLOR_BLACK, 1);

        if (_state == ST_IDLE) {
            _renderer->drawText("A: Launch", 80, PLAY_Y+PLAY_HEIGHT/2,
                                COLOR_UI_SELECT, COLOR_BLACK, 1);
            _renderer->drawText("B/PWR: move paddle", 50, PLAY_Y+PLAY_HEIGHT/2+12,
                                COLOR_UI_DIM, COLOR_BLACK, 1);
        }
            _renderer->drawText("Clear!", 90, PLAY_Y+PLAY_HEIGHT/2,
                                COLOR_GREEN, COLOR_BLACK, 1);
        if (_state == ST_DEAD)
            _renderer->drawText("Ball lost!", 75, PLAY_Y+PLAY_HEIGHT/2,
                                COLOR_RED, COLOR_BLACK, 1);
    }

    SceneResult handleInput() override {
        if (_input->btnPWR_shortPress()) {
            if (_score > _context->highScoreBreakout)
                _context->highScoreBreakout = _score;
            return changeTo("normal");
        }
        if (_state == ST_IDLE) {
            if (_input->btnA_shortPress() || _input->btnB_shortPress()) {
                _state = ST_PLAYING;
                _ballVX = BALL_SPEED * 0.7f;
                _ballVY = -BALL_SPEED * 0.7f;
            }
            return NO_CHANGE;
        }
        if (_state != ST_PLAYING) return NO_CHANGE;

        // BtnB held = paddle right, BtnA held = paddle left
        return NO_CHANGE;
    }

    SceneResult update(float dt) override {
        // Move paddle using held buttons (needs dt, so done here)
        if (_state == ST_PLAYING) {
            if (_input->btnB_held()) _paddleX = min((float)(DISPLAY_WIDTH - PADDLE_W/2), _paddleX + PADDLE_SPEED * dt);
            if (_input->btnA_held()) _paddleX = max((float)(PADDLE_W/2),                 _paddleX - PADDLE_SPEED * dt);
        }
        switch (_state) {
        case ST_PLAYING: _update(dt); break;
        case ST_WIN:  case ST_DEAD:
            _stateTimer -= dt;
            if (_stateTimer<=0) {
                if (_state==ST_WIN && _level<3) { _level++; _resetBall(); _state=ST_PLAYING; }
                else { _state=ST_IDLE; }
            }
            break;
        default: break;
        }
        return NO_CHANGE;
    }

private:
    enum State { ST_IDLE, ST_PLAYING, ST_WIN, ST_DEAD };

    State _state;
    float _paddleX;
    float _ballX, _ballY;
    float _ballVX, _ballVY;
    float _stateTimer;
    int   _score, _level;
    bool  _bricks[BRICK_ROWS][BRICK_COLS];

    void _reset() {
        _state     = ST_IDLE;
        _score     = 0;
        _level     = 1;
        _stateTimer= 0;
        _paddleX   = DISPLAY_WIDTH / 2.0f;
        _resetBricks();
        _resetBall();
    }

    void _resetBall() {
        _ballX  = _paddleX;
        _ballY  = (float)(PLAY_Y + PLAY_HEIGHT - PADDLE_H - 4 - BALL_R - 2);
        _ballVX = 0;
        _ballVY = 0;
    }

    void _resetBricks() {
        for (int r=0; r<BRICK_ROWS; r++)
            for (int c=0; c<BRICK_COLS; c++)
                _bricks[r][c] = true;
    }

    void _update(float dt) {
        _ballX += _ballVX * dt;
        _ballY += _ballVY * dt;

        // Wall bounce (sides)
        if (_ballX - BALL_R < 0)              { _ballX = BALL_R;                _ballVX =  abs(_ballVX); }
        if (_ballX + BALL_R > DISPLAY_WIDTH)  { _ballX = DISPLAY_WIDTH - BALL_R;_ballVX = -abs(_ballVX); }
        // Top wall
        if (_ballY - BALL_R < PLAY_Y)         { _ballY = PLAY_Y + BALL_R;       _ballVY =  abs(_ballVY); }

        // Paddle bounce
        float paddleY = PLAY_Y + PLAY_HEIGHT - PADDLE_H - 4;
        if (_ballVY > 0
            && _ballY + BALL_R >= paddleY
            && _ballY + BALL_R <= paddleY + PADDLE_H + 4
            && _ballX >= _paddleX - PADDLE_W/2
            && _ballX <= _paddleX + PADDLE_W/2)
        {
            float relX = (_ballX - _paddleX) / (PADDLE_W/2.0f); // -1..1
            float angle = relX * 60.0f * (M_PI/180.0f);
            float spd   = sqrt(_ballVX*_ballVX + _ballVY*_ballVY);
            _ballVX = spd * sin(angle);
            _ballVY = -spd * cos(angle);
        }

        // Brick collision
        int brickStartX = (DISPLAY_WIDTH - (BRICK_COLS*(BRICK_W+BRICK_GAP)-BRICK_GAP)) / 2;
        int brickStartY = PLAY_Y + 18;
        bool anyLeft = false;
        for (int row=0; row<BRICK_ROWS; row++) {
            for (int col=0; col<BRICK_COLS; col++) {
                if (!_bricks[row][col]) continue;
                anyLeft = true;
                int bx = brickStartX + col*(BRICK_W+BRICK_GAP);
                int by = brickStartY + row*(BRICK_H+BRICK_GAP);
                if (_ballX + BALL_R >= bx && _ballX - BALL_R <= bx+BRICK_W
                    && _ballY + BALL_R >= by && _ballY - BALL_R <= by+BRICK_H)
                {
                    _bricks[row][col] = false;
                    _score += (BRICK_ROWS - row) * 10;
                    // Determine bounce axis
                    float overlapL = (_ballX + BALL_R) - bx;
                    float overlapR = (bx+BRICK_W) - (_ballX - BALL_R);
                    float overlapT = (_ballY + BALL_R) - by;
                    float overlapB = (by+BRICK_H) - (_ballY - BALL_R);
                    float minH = min(overlapL, overlapR);
                    float minV = min(overlapT, overlapB);
                    if (minH < minV) _ballVX = -_ballVX;
                    else             _ballVY = -_ballVY;
                    if (_score > _context->highScoreBreakout)
                        _context->highScoreBreakout = _score;
                    goto done_bricks;
                }
            }
        }
        done_bricks:

        // Ball fell off bottom
        if (_ballY - BALL_R > PLAY_Y + PLAY_HEIGHT) {
            _state      = ST_DEAD;
            _stateTimer = 1.8f;
        }

        // All bricks cleared
        if (!anyLeft) {
            _state      = ST_WIN;
            _stateTimer = 2.0f;
            _resetBricks();
        }
    }
};
