#include "MenuSystem.h"
#include <M5Unified.h>

namespace idk {

MenuSystem::MenuSystem() {
    // Define the full feature tree as requested in the prompt
    _categories = {
        {"ASCII 2D", {"Matrix Rain", "Sine Wave", "ASCII Fire", "Fireworks", "Game of Life", "Maze Gen", "Worm/Snake", "Wind Particles", "Ripples", "Typewriter", "Bouncing Ball", "Langton's Ant", "Binary Rain", "Radar Sweep", "Spinner"}},
        {"ASCII 3D", {"Torus (Donut)", "Cube", "Sphere", "Infinite Tunnel", "DNA Helix", "Pyramid", "Mobius Strip", "Wormhole", "Hypercube", "Fractal Tree"}},
        {"Screensaver", {"Meteor Shower", "Aurora", "Bubbles", "Snow Fall", "Ocean Waves", "Autumn Leaves", "Fireflies", "Sand Storm", "Starfield", "Clock"}},
        {"Atom/Physics", {"Bohr Atom", "Electron Orbitals", "Nuclear Fission", "EM Wave", "Interference", "Photoelectric", "Plasma", "Decay", "Tunneling"}},
        {"Fractals", {"Mandelbrot", "Julia", "Burning Ship", "Sierpinski", "Koch Snowflake", "Barnsley Fern", "Dragon Curve", "Hilbert Curve", "L-System", "MZoom"}},
        {"Chaos Theory", {"Lorenz", "Rossler", "Bifurcation", "Double Pendulum", "Logistic Map", "Henon Map", "Arnold Cat", "Lyapunov"}},
        {"N-Body", {"3-Body", "Boids", "Particle Gas", "Galaxy Merge", "Swarm", "Spring-Mass", "Vortex", "Charged"}},
        {"Fluid", {"Smoke", "Vortex Street", "KH Instability", "Convection", "Droplet", "Turbulence", "Soliton"}},
        {"Space", {"Black Hole", "Spiral Galaxy", "Pulsar", "Eclipse", "Comet", "Supernova", "G-Waves", "Big Bang", "Asteroids", "Nebula"}},
        {"Math/Geo", {"Lissajous", "Spirograph", "Fourier", "Vector Field", "Voronoi", "Contour", "Quaternion", "Non-Euclidean", "Sieve", "Penrose"}},
        {"Settings", {"Brightness", "FPS Target", "Color Theme", "Auto-Screensaver", "IMU Control", "About"}}
    };
}

void MenuSystem::init() {
    _state = MenuState::CategorySelect;
    _categoryIdx = 0;
    _animIdx = 0;
}

void MenuSystem::update(ASCIIRenderer& r) {
    handleInput();
    if (_state != MenuState::RunningAnimation) {
        draw(r);
    }
}

void MenuSystem::handleInput() {
    bool up = M5.BtnPWR.wasPressed() || M5.BtnC.wasPressed();
    bool down = M5.BtnB.wasPressed();
    bool select = M5.BtnA.wasPressed();
    
    // Hold PWR/BtnC to go back to category selection
    bool back = M5.BtnPWR.wasHold() || M5.BtnC.wasHold(); 

    if (back) {
        if (_state == MenuState::AnimationSelect) {
            _state = MenuState::CategorySelect;
        }
        return; // Skip other input processing this frame if back was triggered
    }

    if (down) { // Next item
        if (_state == MenuState::CategorySelect) {
            _categoryIdx = (_categoryIdx + 1) % _categories.size();
        } else if (_state == MenuState::AnimationSelect) {
            _animIdx = (_animIdx + 1) % _categories[_categoryIdx].animations.size();
        }
    }
    
    if (up && !back) { // Prev item
        if (_state == MenuState::CategorySelect) {
            _categoryIdx = (_categoryIdx + _categories.size() - 1) % _categories.size();
        } else if (_state == MenuState::AnimationSelect) {
            _animIdx = (_animIdx + _categories[_categoryIdx].animations.size() - 1) % _categories[_categoryIdx].animations.size();
        }
    }
    
    if (select) { // Select
        if (_state == MenuState::CategorySelect) {
            _state = MenuState::AnimationSelect;
            _animIdx = 0;
        } else if (_state == MenuState::AnimationSelect) {
            _state = MenuState::RunningAnimation;
        }
    }
}

void MenuSystem::draw(ASCIIRenderer& r) {
    r.clear(' ', TFT_WHITE, 0x1082); // Dark blue background for menu
    
    std::string title = (_state == MenuState::CategorySelect) ? "CATEGORIES" : _categories[_categoryIdx].name;
    r.print(2, 1, title.c_str(), TFT_YELLOW, 0x1082);
    r.print(2, 2, "====================================", TFT_DARKGREY, 0x1082);
    
    int currentIdx = (_state == MenuState::CategorySelect) ? _categoryIdx : _animIdx;
    int totalItems = (_state == MenuState::CategorySelect) ? _categories.size() : _categories[_categoryIdx].animations.size();
    
    // Show 3 items, current in middle (row 6, 8, 10)
    for (int i = -1; i <= 1; ++i) {
        int idx = (currentIdx + i + totalItems) % totalItems;
        const char* name = (_state == MenuState::CategorySelect) ? _categories[idx].name.c_str() : _categories[_categoryIdx].animations[idx].c_str();
        
        uint16_t fg = (i == 0) ? TFT_WHITE : TFT_LIGHTGREY;
        uint16_t bg = (i == 0) ? 0x4208 : 0x1082; // Highlight background
        
        char buffer[40];
        snprintf(buffer, sizeof(buffer), " %c %-30s ", (i == 0 ? '>' : ' '), name);
        r.print(2, 7 + i * 2, buffer, fg, bg);
    }
    
    drawBattery(r);
}

void MenuSystem::drawBattery(ASCIIRenderer& r) {
    float voltage = M5.Power.getBatteryVoltage();
    int pct = (voltage - 3.3f) / (4.2f - 3.3f) * 100.0f;
    if (pct < 0) pct = 0;
    if (pct > 100) pct = 100;
    
    char buffer[10];
    snprintf(buffer, sizeof(buffer), "[%3d%%]", pct);
    r.print(33, 1, buffer, pct > 20 ? TFT_GREEN : TFT_RED, 0x1082);
}

} // namespace idk
