#pragma once

#include <vector>
#include <deque>
#include <memory>

#include "LauCommon.h"
#include "GameObject.hpp"

namespace lau {

class Game {
public:
    void init(int windowWidth, int windowHeight);

    void resize(int windowWidth, int windowHeight);

    void draw(double temporalAlpha);

    void update(double dt);

    void terminate();

private:
    std::vector<std::shared_ptr<GameObject>> gameObjects;
};

} // namespace lau
