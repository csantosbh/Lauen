#if defined(NACL)
#include "window/NaCl.hpp"

namespace pp { // TODO tem que ser no namespace pp?
    Module* CreateModule() {
        return new lau::NaClCanvasModule();
    }
}
#else

#if defined(DESKTOP)
#include "window/Desktop.hpp"
#elif defined(JAVASCRIPT)
#include "window/Browser.hpp"
#endif

using namespace std;

int main(void)
{
    try {
#if defined(DESKTOP)
        lau::Desktop window;
#elif defined(JAVASCRIPT)
        lau::Browser window;
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
#endif
