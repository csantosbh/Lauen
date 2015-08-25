#pragma once

#include <vector>
#include <deque>
#include <memory>

#include "LauCommon.h"
#include "GameObject.hpp"

namespace lau {

class Game {
public:
    Game();

    void init(int windowWidth, int windowHeight);

    void resize(int windowWidth, int windowHeight);

    void draw(double temporalAlpha);

    void update(double dt);

    void terminate();

    const std::vector<std::shared_ptr<GameObject>>& allGameObjects() {
        return gameObjects_;
    }

private:
    std::vector<std::shared_ptr<GameObject>> gameObjects_;
};

namespace lau_internal {
extern Game *GameInstance;
} // namespace lau_internal

} // namespace lau
