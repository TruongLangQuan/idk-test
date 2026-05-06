#pragma once

#include "../../engine/IAnimation.h"
#include <vector>

namespace idk {

class GameOfLife : public IAnimation {
public:
    void init(ASCIIRenderer& r) override {
        r.clear();
        for (int r = 0; r < ASCIIRenderer::kRows; ++r) {
            for (int c = 0; c < ASCIIRenderer::kCols; ++c) {
                _grid[r][c] = (esp_random() % 100 < 20); // 20% alive
            }
        }
        _genCount = 0;
    }

    void update(ASCIIRenderer& r, float dtMs) override {
        _timer += dtMs;
        if (_timer < 100.0f) return; // Limit generation speed
        _timer = 0;

        bool next[ASCIIRenderer::kRows][ASCIIRenderer::kCols];
        bool changed = false;

        for (int row = 0; row < ASCIIRenderer::kRows; ++row) {
            for (int col = 0; col < ASCIIRenderer::kCols; ++col) {
                int neighbors = countNeighbors(row, col);
                bool alive = _grid[row][col];

                if (alive) {
                    next[row][col] = (neighbors == 2 || neighbors == 3);
                } else {
                    next[row][col] = (neighbors == 3);
                }

                if (next[row][col] != _grid[row][col]) changed = true;
                
                // Render
                r.setCell(col, row, next[row][col] ? '#' : ' ', TFT_GREEN, TFT_BLACK);
            }
        }

        memcpy(_grid, next, sizeof(_grid));
        _genCount++;

        // If no change or too many generations, re-seed
        if (!changed || _genCount > 500) {
            init(r);
        }
    }

    const char* name() const override { return "Game of Life"; }

private:
    int countNeighbors(int r, int c) {
        int count = 0;
        for (int i = -1; i <= 1; ++i) {
            for (int j = -1; j <= 1; ++j) {
                if (i == 0 && j == 0) continue;
                int row = (r + i + ASCIIRenderer::kRows) % ASCIIRenderer::kRows;
                int col = (c + j + ASCIIRenderer::kCols) % ASCIIRenderer::kCols;
                if (_grid[row][col]) count++;
            }
        }
        return count;
    }

    bool _grid[ASCIIRenderer::kRows][ASCIIRenderer::kCols];
    float _timer = 0;
    int _genCount = 0;
};

} // namespace idk
