#pragma once

#include "Game.hpp"
#include "../utils/Callback.hpp"

namespace lau {

class Screen {
    friend class Game;

public:
    struct Resolution {
        int width;
        int height;
        int refreshRate;
    };

    // Screen resolution
    static Resolution screenResolution;
    /*
    static bool getFullScreen();
    static void setFullScreen(bool);
    */
    // Window resolution
    static int windowWidth;
    static int windowHeight;

    static Callback<int,int> onWindowResize;

private:
    static void windowResizeCallback(int w, int h);
};

} // namespace lau

