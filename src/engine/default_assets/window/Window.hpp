#pragma once

#include <memory>
#include "Game.hpp"

namespace lau {

using namespace std;

class Window {
public:
    virtual void init(int winWidth, int winHeight) = 0;
    virtual void loop() = 0;
    virtual void terminate() = 0;
    virtual ~Window() {}

protected:
    static const double MS_PER_FRAME;
    static const double MAXIMUM_LAG;

    Game game;
};

} // namespace lau
