#pragma once

#include <vector>
#include <deque>
#include <memory>
#include <mutex>

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

    static void scheduleMainThreadTask(const std::function<void()>& task);

private:
    static std::deque<std::function<void()>> enqueuedSequentialTasks_;
    static std::mutex enqueuedSequentialTasksMtx_;
};

} // namespace lau
