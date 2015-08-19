#if defined(DESKTOP)
#include "window/Desktop.hpp"

using namespace std;

int main(void)
{
    try {
#if defined(DESKTOP)
        lau::Desktop window;
#endif
        // Initialize window
        window.init(640, 480);

        // Run game
        window.loop();
    } catch(...) {
        lau::lerr << "[error] Unhandled exception thrown." << endl;
    }

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
