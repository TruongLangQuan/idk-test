#pragma once

#include "../../engine/IAnimation.h"
#include <cmath>

namespace idk {

class Torus3D : public IAnimation {
public:
    void init(ASCIIRenderer& r) override {
        r.clear();
        _A = 0;
        _angleB = 0;
    }

    void update(ASCIIRenderer& r, float dtMs) override {
        r.clear(' ', TFT_WHITE, TFT_BLACK);
        float zBuffer[ASCIIRenderer::kRows][ASCIIRenderer::kCols];
        memset(zBuffer, 0, sizeof(zBuffer));

        _A += 0.04f * (dtMs / 33.3f);
        _angleB += 0.02f * (dtMs / 33.3f);

        float sinA = std::sin(_A), cosA = std::cos(_A);
        float sinB = std::sin(_angleB), cosB = std::cos(_angleB);

        // Torus algorithm from donut.c
        for (float theta = 0; theta < 6.28f; theta += 0.07f) {
            float cosTheta = std::cos(theta), sinTheta = std::sin(theta);
            for (float phi = 0; phi < 6.28f; phi += 0.02f) {
                float cosPhi = std::cos(phi), sinPhi = std::sin(phi);

                float circleX = R2 + R1 * cosTheta;
                float circleY = R1 * sinTheta;

                float x = circleX * (cosB * cosPhi + sinA * sinB * sinPhi) - circleY * cosA * sinB;
                float y = circleX * (sinB * cosPhi - sinA * cosB * sinPhi) + circleY * cosA * cosB;
                float z = K2 + cosA * circleX * sinPhi + circleY * sinA;
                float ooz = 1 / z; // one over z

                int xp = (int)(ASCIIRenderer::kCols / 2 + K1 * ooz * x * 0.5f); // Scale x for char aspect ratio
                int yp = (int)(ASCIIRenderer::kRows / 2 - K1 * ooz * y);

                // Luminance
                float L = cosPhi * cosTheta * sinB - cosA * cosTheta * sinPhi - sinA * sinTheta + cosB * (cosA * sinTheta - cosTheta * sinA * sinPhi);

                if (L > 0) {
                    if (xp >= 0 && xp < ASCIIRenderer::kCols && yp >= 0 && yp < ASCIIRenderer::kRows) {
                        if (ooz > zBuffer[yp][xp]) {
                            zBuffer[yp][xp] = ooz;
                            int luminanceIdx = (int)(L * 8);
                            if (luminanceIdx < 0) luminanceIdx = 0;
                            if (luminanceIdx >= 12) luminanceIdx = 11;
                            r.setCell(xp, yp, _chars[luminanceIdx], 
                                      M5.Display.color565(100 + luminanceIdx * 12, 100 + luminanceIdx * 12, 255), 
                                      TFT_BLACK);
                        }
                    }
                }
            }
        }
    }

    const char* name() const override { return "Torus (Donut)"; }

private:
    float _A = 0, _angleB = 0;
    const float R1 = 1;
    const float R2 = 2;
    const float K2 = 5;
    const float K1 = ASCIIRenderer::kCols * K2 * 3 / (8 * (R1 + R2));
    const char* _chars = ".,-~:;=!*#$@";
};

} // namespace idk
