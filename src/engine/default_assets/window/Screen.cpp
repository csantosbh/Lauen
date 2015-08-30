#include "Screen.hpp"

namespace lau {

Screen::Resolution Screen::screenResolution;
int Screen::windowWidth;
int Screen::windowHeight;

Callback<int,int> Screen::onWindowResize;

void Screen::windowResizeCallback(int w, int h) {
    windowWidth = w;
    windowHeight = h;

    onWindowResize.broadcast(w, h);
}

} // namespace lau
