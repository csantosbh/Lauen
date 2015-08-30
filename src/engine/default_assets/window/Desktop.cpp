#include "Desktop.hpp"

#ifdef DESKTOP

namespace lau {

std::ostream& lout = std::cout;
std::ostream& lerr = std::cerr;

Game* Desktop::static_game = nullptr; // GLFW static callbacks need a pointer to this

void Desktop::windowResizeCallback(GLFWwindow* window, int w, int h) {
    // TODO maybe use glfwGetFramebufferSize? window size is not the correct way to go; check out http://www.glfw.org/docs/latest/window.html
    static_game->resize(w, h);
}

void Desktop::init(int winWidth, int winHeight) {
    if (!glfwInit()) {
        lerr << "[error] Could not initialize GLFW." << endl;
        throw 1; // TODO 
    }
    glfwSetErrorCallback(&windowErrorCallback);

    /* Create a windowed mode window and its OpenGL context */
    this->window = glfwCreateWindow(winWidth, winHeight, "Hello World", NULL, NULL);
    if (!this->window)
    {
        lerr << "[error] Could not create window." << endl;
        glfwTerminate();
        throw 1;
    }

    glfwSetWindowSizeCallback(this->window, &windowResizeCallback);

    /* Make the window's context current */
    glfwMakeContextCurrent(this->window);
    {
        glewExperimental=GL_TRUE;
        GLenum err=glewInit();
        if(err!=GLEW_OK) {
            // Problem: glewInit failed, something is seriously wrong.
            lerr<<"[error] glewInit failed, aborting due to: "<<glewGetErrorString(err)<<endl;
            throw 1;
        }
    }

    static_game = &game;
    game.init(winWidth, winHeight);
}

void Desktop::loop() {
    double current = utils::time::now();
    double previous = current, lag = 0;

    /* Loop until the user closes the window */
    while (!glfwWindowShouldClose(window))
    {
        current = utils::time::now();
        lag = min(lag + current - previous, MAXIMUM_LAG);
        previous = current;

        /* Poll for and process events */
        glfwPollEvents();

        // Update
        while(lag >= MS_PER_FRAME) {
            game.update(MS_PER_FRAME);
            lag -= MS_PER_FRAME;
        }

        /* Render here */
        game.draw(lag/MS_PER_FRAME);
        /* Swap front and back buffers */
        glfwSwapBuffers(window);
    }
}

void Desktop::terminate() {
    game.terminate();
    glfwSetWindowShouldClose(window, GL_TRUE);
}

Desktop::~Desktop() {
    glfwTerminate();
}

void Desktop::windowErrorCallback(int error, const char* description) {
    lerr << "[error] GLFW error: " << description << endl;
}

} // namespace lau

#endif
