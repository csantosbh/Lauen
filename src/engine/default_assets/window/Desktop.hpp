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
    static void windowSizeCallback(GLFWwindow*, int, int);
    static void updateScreenData(int w, int h);

    GLFWwindow* window;
};

} // namespace lau

#endif
