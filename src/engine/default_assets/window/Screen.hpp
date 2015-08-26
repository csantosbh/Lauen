#pragma once

namespace lau {

class Screen {
public:
    struct Resolution {
        int width;
        int height;
        int refreshRate;
    };

    // Screen resolution
    static Resolution screenResolution;
    /*
     * TODO figure out a better way for this module to communicate with the window
    static bool getFullScreen();
    static void setFullScreen(bool);
    */
    // Window resolution
    static int windowWidth;
    static int windowHeight;

};

} // namespace lau

