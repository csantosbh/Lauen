#pragma once

#include <memory>
#include "Game.hpp"

namespace lau {

using namespace std;

class Window {
public:
    Window() {
        lau_internal::GameInstance = &game;
    }
    virtual void init(int winWidth, int winHeight) = 0;
    virtual void loop() = 0;
    virtual void terminate() = 0;
    virtual ~Window() {}

protected:
    const double MS_PER_FRAME = 1.0/60.0;
    const double MAXIMUM_LAG = 1.0/60.0*5.0;

    Game game;
};

} // namespace lau
