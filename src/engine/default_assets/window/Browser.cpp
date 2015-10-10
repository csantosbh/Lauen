#include "Browser.hpp"

#ifdef JAVASCRIPT
#include <emscripten.h>
#include <emscripten/bind.h>
#include <emscripten/val.h>
#include <SDL/SDL.h>
#include <GL/glew.h>

namespace lau {

std::ostream& lout = std::cout;
std::ostream& lerr = std::cerr;
static double current, previous, lag;

Game* Browser::static_game = nullptr; // GLFW static callbacks need a pointer to this

void Browser::init(int winWidth, int winHeight) {
    if ( !SDL_SetVideoMode( winWidth, winHeight, 24, SDL_OPENGL ) ) {
        lerr << "Could not initialize webgl!" << endl;
        return;
    }
    glewInit ();

    static_game = &game;
    game.init(winWidth, winHeight);

    current = utils::time::now();
    previous = current;
    lag = 0;

}

void Browser::loopImpl(void* game_) {
    Game* game = static_cast<Game*>(game_);
    current = utils::time::now();
    lag = min(lag + current - previous, Window::MAXIMUM_LAG);
    previous = current;

    // Update
    while(lag >= Window::MS_PER_FRAME) {
        game->update(Window::MS_PER_FRAME);
        lag -= Window::MS_PER_FRAME;
    }

    /* Render here */
    game->draw(lag/Window::MS_PER_FRAME);
}

void Browser::loop() {
    emscripten_set_main_loop_arg(&Browser::loopImpl, &game, 0, 1);
}

void Browser::terminate() {
}

Browser::~Browser() {
    game.terminate();
}

} // namespace lau

#endif
