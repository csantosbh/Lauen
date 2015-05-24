#include <iostream>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <cstdlib>
#include <vector>
#include <memory>

#include <rapidjson/document.h>

#include "utils/IO.h"
#include "utils/Time.h"
#include "GameObject.hpp"
#include "Factories.hpp"

using namespace std;

namespace lau {
void windowErrorCallback(int error, const char* description) {
    cerr << "[ERROR] " << description << endl;
}

//////
GLuint bufferIds[2];
GLuint program;
const int vertexAttribId = 0;
void checkShaderCompilation(GLuint shaderId) {
    GLint compilationStatus;
    glGetShaderiv(shaderId, GL_COMPILE_STATUS, &compilationStatus);
    if(compilationStatus == GL_FALSE) {
        GLint logSize = 0;
        glGetShaderiv(shaderId, GL_INFO_LOG_LENGTH, &logSize);
        vector<char> infoLog(logSize);
        glGetShaderInfoLog(shaderId, logSize, &logSize, &infoLog[0]);
        // TODO proper error handling here
        cerr << &infoLog[0] << endl;
        exit(0);
    }
}

void createSimpleGeometry() {
    float vertices[] = {
        -1,-1,0,
        -1,1,0,
        1,1,0,
        1,-1,0,
    };
    uint8_t indices[] = {
        0, 1, 3, 2
    };

    // Create VBO with triangle and indices
    glGenBuffers(2, bufferIds);

    // Lets upload the vertex data first.
    // Bind the buffer object to the generated vertex buffer
    glBindBuffer(GL_ARRAY_BUFFER, bufferIds[0]);
    // Copy the data to the buffer
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    // Now lets upload the indices.
    // Bind the buffer object to the generated index buffer
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, bufferIds[1]);
    // Upload indices to the buffer
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    // Create vertex attributes
    glEnableVertexAttribArray(vertexAttribId);
    glVertexAttribPointer(vertexAttribId, 3, GL_FLOAT, GL_FALSE, 0, 0);

    // Create shaders
    string vs = utils::io::readFile("default_assets/shaders/basic.vs");
    string fs = utils::io::readFile("default_assets/shaders/basic.fs");

    GLuint vsId = glCreateShader(GL_VERTEX_SHADER);
    GLuint fsId = glCreateShader(GL_FRAGMENT_SHADER);

    const GLchar *shaders[] = {vs.c_str(), fs.c_str()};
    glShaderSource(vsId, 1, &shaders[0], NULL);
    glShaderSource(fsId, 1, &shaders[1], NULL);

    glCompileShader(vsId);
    glCompileShader(fsId);

    // Get shader compilation status
    checkShaderCompilation(vsId);
    checkShaderCompilation(fsId);

    program = glCreateProgram();
    glAttachShader(program, vsId);
    glAttachShader(program, fsId);
    glBindAttribLocation(program, vertexAttribId, "in_Position");
    glLinkProgram(program);
    glUseProgram(program);
}

void lauTerminate(GLFWwindow* window) {
    glDeleteBuffers(2, bufferIds);
    glfwSetWindowShouldClose(window, GL_TRUE);
}

void key_callback(GLFWwindow *window, int key, int scancode, int action, int mods) {
    // TODO actually this will be passed to the entities that are listening to keyboard events; I'm not going to exit arbitrarily here.
    switch(key) {
        case GLFW_KEY_ESCAPE:
            if(action == GLFW_PRESS) {
                lauTerminate(window);
            }
            break;
    }
}

void configureCallbacks(GLFWwindow* window) {
    // TODO study which is better for IO: callbacking or polling
    glfwSetKeyCallback(window, key_callback);
}

vector<shared_ptr<GameObject>> gameObjects;
void gameInit() {
    glClearColor(0,0,0,1);
    glViewport(0, 0, 640, 480);
    createSimpleGeometry();
	
	// Load game objects
	rapidjson::Document serializedGameObjects;
    string scene = utils::io::readFile("scene.json");
	serializedGameObjects.Parse(scene.c_str());
	gameObjects = gameObjectFactory(serializedGameObjects);
}

void draw(double temporalAlpha) {
    glClear(GL_COLOR_BUFFER_BIT);

    glBindBuffer(GL_ARRAY_BUFFER, bufferIds[0]);

    glVertexAttribPointer(vertexAttribId, 3, GL_FLOAT, GL_FALSE, 0, 0);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, bufferIds[1]);

    glDrawElements(GL_TRIANGLE_STRIP, 4, GL_UNSIGNED_BYTE, 0);
}

void update(double dt) {
	for(auto& gameObject: gameObjects) {
		gameObject->update(dt);
	}
}

GLFWwindow* window;
void gameLoop() {
    double current = utils::time::now();
    double previous = current, lag = 0;
    const double MS_PER_FRAME = 1.0/60.0;
    const double MAXIMUM_LAG = 1.0/60.0*5.0;

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
            update(MS_PER_FRAME);
            lag -= MS_PER_FRAME;
        }

        cout << "lag = " <<lag<< "/" << MAXIMUM_LAG<< endl;

        /* Render here */
        draw(lag/MS_PER_FRAME);
        /* Swap front and back buffers */
        glfwSwapBuffers(window);
    }
}

} // namespace

int main(void)
{
    /* Initialize the library */
    if (!glfwInit())
        return -1;
    glfwSetErrorCallback(lau::windowErrorCallback);

    /* Create a windowed mode window and its OpenGL context */
    lau::window = glfwCreateWindow(640, 480, "Hello World", NULL, NULL);
    if (!lau::window)
    {
        glfwTerminate();
        return -1;
    }

    /* Make the window's context current */
    glfwMakeContextCurrent(lau::window);
    {
        glewExperimental=GL_TRUE;
        GLenum err=glewInit();
        if(err!=GLEW_OK) {
            // Problem: glewInit failed, something is seriously wrong.
            cerr<<"glewInit failed, aborting due to:"<<glewGetErrorString(err)<<endl;
            exit(0);
        }
    }

    // Configure glfw callbacks
    lau::configureCallbacks(lau::window);

    // Init game
    lau::gameInit();

    // Run game
    lau::gameLoop();

    glfwTerminate();
    return 0;
}
