#pragma once

#ifdef JAVASCRIPT

#include <iostream>
#include "utils/Time.h"
#include "Window.hpp"

#include <GLFW/glfw3.h>

namespace lau {

class Browser : public Window {
public:
    void init(int winWidth, int winHeight);

    void loop();

    void terminate();

    virtual ~Browser();

private:
    static void loopImpl(void* game_);
    static Game* static_game; // GLFW static callbacks need a pointer to this
};

} // namespace lau

#endif
