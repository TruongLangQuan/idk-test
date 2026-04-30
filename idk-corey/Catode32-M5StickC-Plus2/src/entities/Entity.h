#pragma once
// Entity.h - Base entity class

class Entity {
public:
    float x, y;
    bool  visible;

    Entity(float x = 0, float y = 0) : x(x), y(y), visible(true) {}
    virtual ~Entity() {}

    virtual void update(float dt) {}
    virtual void draw(class Renderer& r) {}
};
