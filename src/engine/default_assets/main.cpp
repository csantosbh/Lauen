#include "window/Desktop.hpp"

using namespace std;

#if defined(DESKTOP)
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
namespace pp { // TODO tem que ser no namespace pp?
    Module* CreateModule() {
        return new lau::NaClCanvasModule();
    }
}
#endif
