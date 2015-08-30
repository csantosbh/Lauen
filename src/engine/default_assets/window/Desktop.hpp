#pragma once

#ifdef DESKTOP

#include <iostream>
#include "utils/Time.h"
#include "window/Window.hpp"

#include <GLFW/glfw3.h>

namespace lau {

class Desktop : public Window {
public:
    void init(int winWidth, int winHeight);

    void loop();

    void terminate();

    virtual ~Desktop();

private:
    static void windowErrorCallback(int error, const char* description);
    static void windowResizeCallback(GLFWwindow*, int, int);
    static Game* static_game; // GLFW static callbacks need a pointer to this

    GLFWwindow* window;
};

} // namespace lau

#endif
