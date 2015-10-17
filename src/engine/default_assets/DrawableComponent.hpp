#pragma once

#include "Component.hpp"

namespace lau {

class DrawableComponent : public Component {
public:
    virtual void start() {}
    virtual void draw(float alpha) = 0;
    virtual ~DrawableComponent() {}
};

}
