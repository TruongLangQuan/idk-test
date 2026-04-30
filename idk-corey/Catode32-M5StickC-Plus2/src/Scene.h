#pragma once
// Scene.h - Abstract base class for all game scenes

#include "Renderer.h"
#include "Input.h"
#include "GameContext.h"

// Scene lifecycle:
//   load()   - called once when scene is first created
//   unload() - called when scene is evicted from cache
//   enter()  - called every time scene becomes active
//   exit()   - called every time scene becomes inactive
//   update() - called every frame
//   draw()   - called every frame after update
//   handleInput() - called every frame before update

// Return type from update/handleInput: signal a scene change
struct SceneResult {
    bool        changeScene = false;
    const char* sceneName   = nullptr;
};

static const SceneResult NO_CHANGE    = { false, nullptr };
static inline SceneResult changeTo(const char* name) { return { true, name }; }

class Scene {
public:
    Scene(GameContext* ctx, Renderer* r, InputHandler* inp)
        : _context(ctx), _renderer(r), _input(inp) {}
    virtual ~Scene() {}

    virtual void load()   {}
    virtual void unload() {}
    virtual void enter()  {}
    virtual void exit()   {}

    virtual SceneResult update(float dt)     { return NO_CHANGE; }
    virtual void        draw()               {}
    virtual SceneResult handleInput()        { return NO_CHANGE; }

protected:
    GameContext*  _context;
    Renderer*     _renderer;
    InputHandler* _input;
};
