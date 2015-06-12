#include "window/Window.hpp"
#include "window/Desktop.hpp"
#include "window/NaCl.hpp"

namespace lau {

shared_ptr<Window> Window::wndInstance;

Window* Window::getInstance() {
    if(wndInstance == NULL) {
#if defined(DESKTOP)
        wndInstance = shared_ptr<Window>(new Desktop());
#endif
    }
    return wndInstance.get();
}

} // namespace lau
