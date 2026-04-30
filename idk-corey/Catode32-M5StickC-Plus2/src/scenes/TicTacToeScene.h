#pragma once
// TicTacToeScene.h - Tic-tac-toe vs simple AI
// BtnA short = advance cursor. BtnPWR short = prev cursor.
// BtnA medium = place mark. BtnB long = exit.

#include "Scene.h"

class TicTacToeScene : public Scene {
public:
    TicTacToeScene(GameContext* ctx, Renderer* r, InputHandler* inp)
        : Scene(ctx,r,inp), _cursor(0), _turn(1), _result(0),
          _aiThinking(false), _aiTimer(0.0f), _state(ST_PLAYING)
    {}

    void enter() override { _reset(); }

    SceneResult update(float dt) override {
        if (_state == ST_AI_THINKING) {
            _aiTimer -= dt;
            if (_aiTimer <= 0) {
                _doAiMove();
                _state = ST_PLAYING;
            }
        }
        if (_state == ST_GAMEOVER) {
            // Wait for any button via handleInput
        }
        return NO_CHANGE;
    }

    void draw() override {
        _renderer->clear();

        // Grid
        int gx = (DISPLAY_WIDTH - GRID_SIZE) / 2;
        int gy = PLAY_Y + (PLAY_HEIGHT - GRID_SIZE) / 2;

        // Grid lines
        _renderer->drawLine(gx+CELL, gy,            gx+CELL, gy+GRID_SIZE,    COLOR_UI_BORDER);
        _renderer->drawLine(gx+CELL*2, gy,          gx+CELL*2, gy+GRID_SIZE,  COLOR_UI_BORDER);
        _renderer->drawLine(gx, gy+CELL,            gx+GRID_SIZE, gy+CELL,    COLOR_UI_BORDER);
        _renderer->drawLine(gx, gy+CELL*2,          gx+GRID_SIZE, gy+CELL*2,  COLOR_UI_BORDER);

        // Marks
        for (int i=0; i<9; i++) {
            int cx = gx + (i%3)*CELL + CELL/2;
            int cy = gy + (i/3)*CELL + CELL/2;
            if (_board[i] == 1) {
                // X
                _renderer->drawLine(cx-10, cy-10, cx+10, cy+10, COLOR_UI_SELECT);
                _renderer->drawLine(cx+10, cy-10, cx-10, cy+10, COLOR_UI_SELECT);
            } else if (_board[i] == 2) {
                // O
                _renderer->drawCircle(cx, cy, 11, COLOR_RED, false);
            }
        }

        // Cursor (player turn only)
        if (_state == ST_PLAYING && _turn == 1) {
            int cx = gx + (_cursor%3)*CELL;
            int cy = gy + (_cursor/3)*CELL;
            _renderer->drawRect(cx, cy, CELL, CELL, COLOR_YELLOW, false);
        }

        // HUD
        const char* who = (_turn==1) ? "Your turn (X)" : "AI thinking…";
        _renderer->drawText(who, 4, PLAY_Y+2, COLOR_UI_TEXT, COLOR_BLACK, 1);
        _renderer->drawText("A:move PWR:prev A-med:place", 4, DISPLAY_HEIGHT-9,
                            COLOR_UI_DIM, COLOR_BLACK, 1);

        if (_state == ST_GAMEOVER) {
            const char* msg;
            if (_result==1)      msg = "You win!";
            else if (_result==2) msg = "AI wins!";
            else                 msg = "Draw!";
            int tx = (DISPLAY_WIDTH - (int)strlen(msg)*6) / 2;
            _renderer->drawText(msg, tx, PLAY_Y + PLAY_HEIGHT/2 - 5,
                                COLOR_GREEN, COLOR_BLACK, 1);
            _renderer->drawText("B:back to home",
                                (DISPLAY_WIDTH - 14*6)/2, PLAY_Y+PLAY_HEIGHT/2+8,
                                COLOR_UI_DIM, COLOR_BLACK, 1);
        }
    }

    SceneResult handleInput() override {
        if (_input->btnPWR_shortPress()) return changeTo("normal");

        if (_state == ST_GAMEOVER) {
            if (_input->select() || _input->next()) { _reset(); return NO_CHANGE; }
            return NO_CHANGE;
        }
        if (_state != ST_PLAYING || _turn != 1) return NO_CHANGE;

        if (_input->next()) {
            _cursor = (_cursor + 1) % 9;
        }
        if (_input->prev()) {
            _cursor = (_cursor + 8) % 9;
        }
        if (_input->select()) {
            if (_board[_cursor] == 0) {
                _place(1, _cursor);
                if (!_checkGameOver()) {
                    _turn = 2;
                    _state = ST_AI_THINKING;
                    _aiTimer = 0.6f;
                }
            }
        }
        return NO_CHANGE;
    }

private:
    enum State { ST_PLAYING, ST_AI_THINKING, ST_GAMEOVER };

    static const int CELL      = 36;
    static const int GRID_SIZE = CELL * 3;

    int   _board[9];
    int   _cursor, _turn, _result;
    bool  _aiThinking;
    float _aiTimer;
    State _state;

    void _reset() {
        memset(_board, 0, sizeof(_board));
        _cursor  = 0;
        _turn    = 1;
        _result  = 0;
        _state   = ST_PLAYING;
        _aiTimer = 0;
    }

    void _place(int who, int cell) {
        _board[cell] = who;
    }

    bool _checkGameOver() {
        static const int wins[8][3] = {
            {0,1,2},{3,4,5},{6,7,8},  // rows
            {0,3,6},{1,4,7},{2,5,8},  // cols
            {0,4,8},{2,4,6}           // diagonals
        };
        for (auto& w : wins) {
            if (_board[w[0]] && _board[w[0]]==_board[w[1]] && _board[w[1]]==_board[w[2]]) {
                _result = _board[w[0]];
                _state  = ST_GAMEOVER;
                return true;
            }
        }
        // Draw?
        bool full = true;
        for (int i=0;i<9;i++) if (_board[i]==0) { full=false; break; }
        if (full) { _result=0; _state=ST_GAMEOVER; return true; }
        return false;
    }

    void _doAiMove() {
        // Try to win, then block, then center, then random
        int move = _findBestMove();
        if (move >= 0) {
            _place(2, move);
            _turn = 1;
            _checkGameOver();
        }
    }

    int _findBestMove() {
        // Win
        int m = _findWinningMove(2); if (m>=0) return m;
        // Block player
        m = _findWinningMove(1); if (m>=0) return m;
        // Center
        if (_board[4]==0) return 4;
        // Corner
        int corners[]={0,2,6,8};
        for (int c:corners) if (_board[c]==0) return c;
        // Any empty
        for (int i=0;i<9;i++) if (_board[i]==0) return i;
        return -1;
    }

    int _findWinningMove(int who) {
        static const int wins[8][3] = {
            {0,1,2},{3,4,5},{6,7,8},
            {0,3,6},{1,4,7},{2,5,8},
            {0,4,8},{2,4,6}
        };
        for (auto& w : wins) {
            int cnt=0, empty=-1;
            for (int j=0;j<3;j++) {
                if (_board[w[j]]==who) cnt++;
                else if (_board[w[j]]==0) empty=w[j];
            }
            if (cnt==2 && empty>=0) return empty;
        }
        return -1;
    }
};
