#pragma once

#include <memory>
#include "Game.hpp"

namespace lau {

using namespace std;

class Window {
public:
    virtual void init() = 0;
    virtual void loop() = 0;
    virtual void terminate() = 0;
    virtual ~Window() {}
    static Window* getInstance();

protected:
    Game game;
    static shared_ptr<Window> wndInstance;
};

} // namespace lau
