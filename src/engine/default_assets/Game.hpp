#pragma once

#include "utils/IO.h"
#include "Factories.hpp"

namespace lau {

#ifndef NACL
class Game {
public:
    void init() {
        glClearColor(0,0,0,1);
        glViewport(0, 0, 640, 480);
        createSimpleGeometry();

        // Load game objects
        rapidjson::Document serializedGameObjects;
        string scene = utils::io::readFile("scenes/scene0.json");
        serializedGameObjects.Parse(scene.c_str());
        gameObjects = Factories::gameObjectFactory(serializedGameObjects);
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

    void terminate() {
        glDeleteBuffers(2, bufferIds);
    }

private:
    vector<shared_ptr<GameObject>> gameObjects;

    ////// TODO shader&VBO&EBO classes
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
};
#else
class Game {
public:
};
#endif

} // namespace lau
