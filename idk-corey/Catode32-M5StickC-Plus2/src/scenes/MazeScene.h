#pragma once
// MazeScene.h - Maze navigation minigame
// BtnA short = turn right / move fwd. BtnPWR short = turn left.
// BtnA medium = move forward. BtnB long = exit.

#include "Scene.h"

class MazeScene : public Scene {
public:
    static const int COLS = 9;
    static const int ROWS = 7;
    static const int CEL  = 14; // cell size px

    MazeScene(GameContext* ctx, Renderer* r, InputHandler* inp)
        : Scene(ctx,r,inp), _px(0), _py(0), _dir(0), _won(false),
          _wonTimer(0.0f), _moves(0), _score(0) {}

    void enter() override { _generate(); _won=false; _moves=0; _wonTimer=0; }

    SceneResult update(float dt) override {
        if (_won) {
            _wonTimer += dt;
            if (_wonTimer > 2.5f) {
                if (_score > _context->highScoreMaze)
                    _context->highScoreMaze = _score;
                return changeTo("normal");
            }
        }
        return NO_CHANGE;
    }

    void draw() override {
        _renderer->clear();

        // Maze walls
        int offX = (DISPLAY_WIDTH  - COLS * CEL) / 2;
        int offY = PLAY_Y + (PLAY_HEIGHT - ROWS * CEL) / 2;

        for (int r=0; r<ROWS; r++) {
            for (int c=0; c<COLS; c++) {
                int x = offX + c*CEL;
                int y = offY + r*CEL;
                uint8_t w = _walls[r][c];
                if (w & W_N) _renderer->drawLine(x,    y,    x+CEL, y,    COLOR_WHITE);
                if (w & W_S) _renderer->drawLine(x,    y+CEL,x+CEL, y+CEL,COLOR_WHITE);
                if (w & W_W) _renderer->drawLine(x,    y,    x,     y+CEL,COLOR_WHITE);
                if (w & W_E) _renderer->drawLine(x+CEL,y,    x+CEL, y+CEL,COLOR_WHITE);
            }
        }

        // Goal
        int gx = offX + (COLS-1)*CEL + 2;
        int gy = offY + (ROWS-1)*CEL + 2;
        _renderer->drawRect(gx, gy, CEL-4, CEL-4, COLOR_YELLOW, true);

        // Player
        int px = offX + _px*CEL + 2;
        int py = offY + _py*CEL + 2;
        _renderer->drawRect(px, py, CEL-4, CEL-4, COLOR_UI_SELECT, true);
        // Direction indicator
        static const int dx[] = {0,1,0,-1}, dy[] = {-1,0,1,0};
        _renderer->drawLine(px + (CEL-4)/2,     py + (CEL-4)/2,
                            px + (CEL-4)/2 + dx[_dir]*3,
                            py + (CEL-4)/2 + dy[_dir]*3,
                            COLOR_BLACK);

        // HUD
        char buf[28];
        snprintf(buf, sizeof(buf), "Moves:%d HiScore:%d", _moves, _context->highScoreMaze);
        _renderer->drawText(buf, 4, PLAY_Y+2, COLOR_UI_TEXT, COLOR_BLACK, 1);
        _renderer->drawText("A:R-turn B:fwd PWR:L-turn", 4, DISPLAY_HEIGHT-9,
                            COLOR_UI_DIM, COLOR_BLACK, 1);

        if (_won) {
            _renderer->drawText("You made it!", 50, PLAY_Y+PLAY_HEIGHT/2-5,
                                COLOR_GREEN, COLOR_BLACK, 1);
        }
    }

    SceneResult handleInput() override {
        if (_won) return NO_CHANGE;
        if (_input->back()) return changeTo("normal");

        if (_input->select()) {
            // BtnA short = turn right
            _dir = (_dir + 1) % 4;
        }
        if (_input->next()) {
            // BtnB short = move forward
            _tryMove();
        }
        if (_input->prev()) {
            // BtnPWR short = turn left
            _dir = (_dir + 3) % 4;
        }
        return NO_CHANGE;
    }

private:
    static const uint8_t W_N=1, W_S=2, W_W=4, W_E=8;

    int   _px, _py, _dir;
    bool  _won;
    float _wonTimer;
    int   _moves, _score;

    uint8_t _walls[ROWS][COLS];
    bool    _visited[ROWS][COLS];

    void _tryMove() {
        static const int dx[] = {0,1,0,-1}, dy[] = {-1,0,1,0};
        static const uint8_t opposite[] = {W_S, W_W, W_N, W_E};
        static const uint8_t wallBit[]  = {W_N, W_E, W_S, W_W};

        if (_walls[_py][_px] & wallBit[_dir]) return; // wall there

        _px += dx[_dir];
        _py += dy[_dir];
        _moves++;

        if (_px == COLS-1 && _py == ROWS-1) {
            _won = true;
            _wonTimer = 0;
            _score = max(0, 200 - _moves);
        }
    }

    void _generate() {
        memset(_walls,   0xFF, sizeof(_walls));
        memset(_visited, 0,    sizeof(_visited));
        _px=0; _py=0; _dir=1;
        _dfs(0,0);
    }

    void _dfs(int c, int r) {
        _visited[r][c] = true;
        int order[4] = {0,1,2,3};
        // Fisher-Yates shuffle
        for (int i=3; i>0; i--) {
            int j = random(0,i+1);
            int tmp=order[i]; order[i]=order[j]; order[j]=tmp;
        }
        static const int dc[] = {0,1,0,-1}, dr[] = {-1,0,1,0};
        static const uint8_t wallDir[] = {W_N, W_E, W_S, W_W};
        static const uint8_t wallOpp[] = {W_S, W_W, W_N, W_E};

        for (int i=0; i<4; i++) {
            int d = order[i];
            int nc = c + dc[d], nr = r + dr[d];
            if (nc<0||nc>=COLS||nr<0||nr>=ROWS||_visited[nr][nc]) continue;
            _walls[r][c]   &= ~wallDir[d];
            _walls[nr][nc] &= ~wallOpp[d];
            _dfs(nc, nr);
        }
    }
};
