#pragma once

#ifdef DESKTOP

#include <iostream>
#include "utils/Time.h"
#include "window/Window.hpp"

#include <GLFW/glfw3.h>

namespace lau {

std::ostream& lout = std::cout;
std::ostream& lerr = std::cerr;

class Desktop : public Window {
public:
    void init(int winWidth, int winHeight) {
        if (!glfwInit()) {
            lerr << "[error] Could not initialize GLFW." << endl;
            throw 1; // TODO 
        }
        glfwSetErrorCallback(&windowErrorCallback);

        /* Create a windowed mode window and its OpenGL context */
        // TODO grab the window size and position from somewhere else
        this->window = glfwCreateWindow(640, 480, "Hello World", NULL, NULL);
        if (!this->window)
        {
            lerr << "[error] Could not create window." << endl;
            glfwTerminate();
            throw 1;
        }

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

        game.init(winWidth, winHeight);
    }

    void loop() {
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

    void terminate() {
        game.terminate();
        glfwSetWindowShouldClose(window, GL_TRUE);
    }

    virtual ~Desktop() {
        glfwTerminate();
    }

private:
    static void windowErrorCallback(int error, const char* description) {
        lerr << "[error] GLFW error: " << description << endl;
    }

    GLFWwindow* window;
};

} // namespace lau

#endif
