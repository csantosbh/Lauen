#if defined(DESKTOP)
#include "window/Desktop.hpp"

int main(void)
{
    lau::Window* window = lau::Window::getInstance();

    // Initialize window
    window->init();

    // Run game
    window->loop();

    return 0;
}
#elif defined(NACL)
#include "window/NaCl.hpp"

namespace pp { // TODO tem que ser no namespace pp?
    Module* CreateModule() {
        return new lau::NaClCanvasModule();
    }
}
#endif
