#include <iomanip>
#include <cmath>

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

#include "math/Math.hpp"
#include "math/Matrix.hpp"
#include "math/Vector.hpp"
#include "math/Quaternion.hpp"

using namespace std;
using namespace lau::math;

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
