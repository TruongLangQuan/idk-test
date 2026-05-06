#pragma once

#include "../../engine/IAnimation.h"
#include <cmath>
#include <vector>

namespace idk {

class BohrAtom : public IAnimation {
public:
    void init(ASCIIRenderer& r) override {
        r.clear();
        _shells = {
            {3.0f, 0.05f, 0.0f}, // K shell
            {6.0f, 0.03f, 2.0f}, // L shell
            {9.0f, 0.02f, 4.0f}  // M shell
        };
    }

    void update(ASCIIRenderer& r, float dtMs) override {
        r.clear(' ', TFT_WHITE, TFT_BLACK);
        
        int cx = ASCIIRenderer::kCols / 2;
        int cy = ASCIIRenderer::kRows / 2;

        // Draw Nucleus
        r.setCell(cx, cy, 'O', TFT_RED, TFT_BLACK);
        r.setCell(cx+1, cy, '+', TFT_RED, TFT_BLACK);
        r.setCell(cx-1, cy, '+', TFT_RED, TFT_BLACK);

        for (auto& s : _shells) {
            s.angle += s.speed * (dtMs / 33.3f);
            
            // Draw shell orbit (faint)
            for (float a = 0; a < 6.28f; a += 0.3f) {
                int ox = (int)(cx + s.radius * std::cos(a) * 1.5f);
                int oy = (int)(cy + s.radius * std::sin(a));
                r.setCell(ox, oy, '.', 0x4208, TFT_BLACK); // Dark grey
            }

            // Draw Electron
            int ex = (int)(cx + s.radius * std::cos(s.angle) * 1.5f);
            int ey = (int)(cy + s.radius * std::sin(s.angle));
            r.setCell(ex, ey, '*', TFT_CYAN, TFT_BLACK);
            
            // Trail
            for (int i = 1; i < 5; ++i) {
                float ta = s.angle - i * 0.1f;
                int tx = (int)(cx + s.radius * std::cos(ta) * 1.5f);
                int ty = (int)(cy + s.radius * std::sin(ta));
                r.setCell(tx, ty, '.', 0x03EF, TFT_BLACK); // Darker cyan
            }
        }
    }

    const char* name() const override { return "Bohr Atom"; }

private:
    struct Shell {
        float radius;
        float speed;
        float angle;
    };
    std::vector<Shell> _shells;
};

} // namespace idk
